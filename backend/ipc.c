/*  ipc.c -- inter-process communication (IPC) support
 *  Copyright (C) 2008, 2009, 2011  SEIKO EPSON CORPORATION
 *
 *  License: AVASYS PUBLIC LICENSE
 *  Author : AVASYS CORPORATION
 *
 *  This file is part of Image Scan! for Linux.
 *  It is distributed under the terms of the AVASYS PUBLIC LICENSE.
 *
 *  You should have received a verbatim copy of the AVASYS PUBLIC
 *  LICENSE along with the software.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ipc.h"

#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#include "defines.h"
#include "message.h"


/*! Attempts to read all the data up to \a size bytes.
 *  MERR is returned if an error, such as a timeout, occurs.
 *  MEOF is returned if EOF is reached.
 *  Otherwise, returns the number of bytes read, which must be greater than 0.
 */
static ssize_t
recv_all (int fd, void *buf, size_t size)
{
  ssize_t n = 0;
  ssize_t t = 1;

  if (0 == size) return MERR;

  while (n < size && t > 0)
    {
      errno = 0;
      t = read (fd, buf + n, size - n);
      if (0 > t)
        {
          err_major ("read failed: %s", strerror (errno));
          return MERR;
        }
      else
        {
          n += t;
          log_call ("transferred %zd bytes, total %zd/%zd", t, n, size);
        }
      if (0 == t) return MEOF;
    }

  return n;
}

/*! Attempts to write all the data up to \a size bytes.
 *  MERR is returned if an error, such as a timeout, occurs.
 *  Otherwise, returns the number of bytes written.
 *  A return value of zero indicates that nothing was written.
 */
static ssize_t
send_all (int fd, const void *buf, size_t size)
{
  ssize_t n = 0;
  ssize_t t = 1;

  if (0 == size) return MERR;

  while (n < size && t > 0)
    {
      errno = 0;
      t = write (fd, buf + n, size - n);
      if (0 > t)
        {
          err_major ("write failed: %s", strerror (errno));
          return MERR;
        }
      else
        {
          n += t;
          log_call ("transferred %zd bytes, total %zd/%zd", t, n, size);
        }
    }

  return n;
}

/*! MERR is returned if an error, such as a timeout, occurs.
 *  MERR is also returned if writing the ipc header failed.
 *  Otherwise, the number of bytes of the payload that were successfully
 *  written is returned.
 *  A return value of zero indicates that nothing was written, this should
 *  only occur when the payload is of zero size.
 */
ssize_t
ipc_send (int sock, uint16_t id, uint8_t type_status,
          size_t size, const void* payload)
{
  ssize_t n = 0;

  n = send_all (sock, &id, sizeof (id));
  if (0 >= n) return MERR;

  n = send_all (sock, &type_status, sizeof (type_status));
  if (0 >= n) return MERR;

  n = send_all (sock, &size, sizeof (size));
  if (0 >= n) return MERR;

  if (0 == size) return 0;
  if (!payload) return MERR;

  n = send_all (sock, payload, size);

  log_info ("send packet {key: %d, msg: 0x%02x, size: %zd}",
            id, type_status, size);

  if (ENABLE_DEBUG && 0 < n)
    {
      if (MSG_DBG_IMG_THRESHOLD < n)
        dbg_img (payload, n);
      else
        dbg_hex (payload, n);
    }

  return n;
}

/*! Caller must have allocated space for \a id, and \a type_status.
 *  \a payload is automatically allocated based on the size field of the
 *  ipcling header, and it is the responsibility of the caller to
 *  deallocate it.
 *
 *  MERR is returned if an error, such as a timeout, occurs.
 *  MEOF is returned if EOF is reached.
 *  Otherwise, returns the number of bytes read.
 *  A return value of zero is valid, as ipc packets do not necessarily
 *  have to contain a payload.
 */
ssize_t
ipc_recv (int sock, uint16_t *id, uint8_t *type_status,
          void** payload)
{
  size_t size = 0;
  char* buf = NULL;
  ssize_t n = 0;

  n = recv_all (sock, id, sizeof (*id));
  if (0 > n) return n;

  n = recv_all (sock, type_status, sizeof (*type_status));
  if (0 > n) return n;

  n = recv_all (sock, &size, sizeof (size));
  if (0 > n) return n;

  if (0 == size) return 0;
  if (!payload) return MERR;

  buf = t_malloc (size, char);
  if (!buf) return MERR;

  n = recv_all (sock, buf, size);

  *payload = buf;

  log_info ("recv packet {key: %d, msg: 0x%02x, size: %zd}",
            *id, *type_status, size);

  if (ENABLE_DEBUG && 0 < n)
    {
      if (MSG_DBG_IMG_THRESHOLD < n)
        dbg_img (*payload, n);
      else
        dbg_hex (*payload, n);
    }

  return n;
}

/*! \brief  Does the real work of starting the \a child process
 */
static
SANE_Status
ipc_fork (process *child)
{
  SANE_Status s = SANE_STATUS_GOOD;

  int pipe_fd[2];

  require (child);

  if (-1 == pipe (pipe_fd))
    {
      err_fatal ("pipe: %s", strerror (errno));
      return SANE_STATUS_ACCESS_DENIED;
    }

  child->pid = fork ();
  if (0 == child->pid)
    {
      /*  replace child process with a plugin program
       */
      close (pipe_fd[0]);           /* unused read end */
      if (0 <= dup2 (pipe_fd[1], STDOUT_FILENO))
        {
          log_info ("%s[%d]: starting", child->name, getpid ());
          if (-1 == execl (child->name, child->name, NULL))
            {
              err_fatal ("%s[%d]: %s", child->name, getpid (),
                         strerror (errno));
            }
        }
      else
        {
          err_major ("%s[%d]: %s", child->name, getpid (),
                     strerror (errno));
        }

      /* notify the parent process that we're done here */
      write (pipe_fd[1], "-1\n", strlen ("-1\n"));
      fsync (pipe_fd[1]);

      close (pipe_fd[1]);
      exit (EXIT_FAILURE);
    }

  if (0 > child->pid)
    {
      err_fatal ("fork: %s", strerror (errno));
      s = SANE_STATUS_CANCELLED;
    }
  else
    {
      /*  Check whether child process has (unexpectedly) exited.  We
          don't want to have zombies in our closet ;-)
       */
      pid_t w = waitpid (child->pid, NULL, WNOHANG);
      if (-1 == w)
        {
          err_minor ("waitpid: %s", strerror (errno));
        }
      if (0 != w)
        {
          log_info ("%s[%d]: exited prematurely", child->name, child->pid);
          s = SANE_STATUS_CANCELLED;
        }
      else
        {
          FILE *fp = fdopen (pipe_fd[0], "rb");
          if (fp)
            {
              if (1 != fscanf (fp, "%d", &(child->port)))
                {
                  err_major ("fscanf: %s", strerror (errno));
                }
              fclose (fp);
            }
          else
            {
              err_fatal ("%s", strerror (errno));
            }
        }
    }
  close (pipe_fd[0]);
  close (pipe_fd[1]);

  if (0 > child->port)
    s = SANE_STATUS_CANCELLED;

  return s;
}

/*! \brief  Requests a connection to a \a child
 */
static
SANE_Status
ipc_connect (process *child)
{
  struct sockaddr_in addr;
  struct timeval t;
  int rv;

  require (child);

  log_call ("(%s, %d)", child->name, child->port);

  errno = 0;
  child->socket = socket (AF_INET, SOCK_STREAM, 0);
  if (0 > child->socket)
    {
      err_major ("socket: %s", strerror (errno));
      return SANE_STATUS_IO_ERROR;
    }

  t.tv_sec = 30;
  t.tv_usec = 0;
  errno = 0;
  rv = setsockopt (child->socket, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof (t));
  if (0 > rv)
    {
      err_minor ("socket option: %s", strerror (errno));
    }

  errno = 0;
  rv = setsockopt (child->socket, SOL_SOCKET, SO_SNDTIMEO, &t, sizeof (t));
  if (0 > rv)
    {
      err_minor ("socket option: %s", strerror (errno));
    }

  memset (&addr, 0, sizeof (addr));
  addr.sin_family      = AF_INET;
  addr.sin_port        = htons (child->port);
  addr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);

  if (0 != connect (child->socket, (struct sockaddr *) &addr, sizeof (addr)))
    {
      err_major ("connect: %s", strerror (errno));
      return SANE_STATUS_IO_ERROR;
    }

  return SANE_STATUS_GOOD;
}

process *
ipc_exec (const char *program, const char *pkglibdir, SANE_Status *status)
{
  SANE_Status s  = SANE_STATUS_GOOD;
  process *child = NULL;

  log_call ("(%s, %s, %p)", program, pkglibdir, status);

  child = t_malloc (1, process);
  if (!child)
    {
      if (status) *status = SANE_STATUS_NO_MEM;
      return NULL;
    }

  child->pid    = -1;
  child->port   = -1;
  child->socket = -1;
  child->name   = NULL;

  if (!pkglibdir)
    {
      child->name = strdup (program);
    }
  else
    {
      int n = (strlen (pkglibdir) + strlen (FILE_SEP_STR)
               + strlen (program) + 1);
      char *name = t_malloc (n, char);

      if (name)
        {
          sprintf (name, "%s%s%s", pkglibdir, FILE_SEP_STR, program);
          child->name = name;
        }
    }

  if (!child->name)
    {
      s = SANE_STATUS_NO_MEM;
    }
  else if (access (child->name, X_OK))
    {
      s = SANE_STATUS_ACCESS_DENIED;
    }

  if (SANE_STATUS_GOOD != s)
    {
      if (status) *status = s;
      delete (child);
      return NULL;
    }

  s = ipc_fork (child);
  if (SANE_STATUS_GOOD == s)
    {
      int tries = 5;

      do
        {
          if (SANE_STATUS_GOOD != s)
            sleep (1);
          s = ipc_connect (child);
        }
      while (0 < --tries && SANE_STATUS_GOOD != s);
    }

  if (SANE_STATUS_GOOD != s)
    {
      child = ipc_kill (child);
      promise (!child);
    }
  else
    {
      promise (child);
      promise (0 < child->pid);
      promise (0 < child->port);
      promise (0 < child->socket);
      promise (child->name);
    }

  if (status) *status = s;

  return child;
}

process *
ipc_kill (process *child)
{
  log_call ("(%p)", child);

  if (child)
    {
      int status = 0;

      log_info ("terminating %s (port %d)", child->name, child->port);

      if (0 <= child->socket)
        {
          if (0 != close (child->socket))
            {
              err_minor ("%s", strerror (errno));
            }
        }
      if (1 < child->pid)
        {
          if (0 != kill (child->pid, SIGHUP))
            {
              err_minor ("%s", strerror (errno));
            }
          if (child->pid != waitpid (child->pid, &status, 0))
            {
              err_major ("%s", strerror (errno));
            }

          if (!WIFSIGNALED (status))
            {
              err_major ("%s[%d]: went off the deep end!",
                         child->name, child->pid);
            }
          else
            {
              if (SIGHUP != WTERMSIG (status))
                {
                  err_major ("%s[%d]: %s", child->name, child->pid,
                             strsignal (WTERMSIG (status)));
                }
            }
        }

      const_delete (child->name, char *);
      delete (child);
    }

  return child;
}

void
ipc_dip_proc (process *child, int flag, const ipc_dip_parms *p,
              SANE_Parameters *ctx, void **buffer)
{
  int socket;
  uint8_t  status = STATUS_NG;
  uint16_t id     = 0;
  ssize_t  n;

  require (child);
  socket = child->socket;

  require (TYPE_DIP_SKEW_FLAG == flag || TYPE_DIP_CROP_FLAG == flag);
  require (0 < socket && p && ctx && buffer && *buffer);

  /* inter-process procedure call, status will be STATUS_NG in case
   * anything goes wrong during IPC call sequence
   */
  {
    n = ipc_send (socket, id, flag | TYPE_DIP_CTOR,
                  strlen (p->fw_name), p->fw_name);
    if (strlen (p->fw_name) == n)
      {
        n = ipc_recv (socket, &id, &status, NULL);
        if (STATUS_OK == status)
          {
            if (sizeof (*p) != ipc_send (socket, id, flag | TYPE_DIP_PARM,
                                         sizeof (*p), p))
              {
                status = STATUS_NG;
              }
            else
              {
                ipc_recv (socket, &id, &status, NULL);
                if (STATUS_OK == status)
                  {
                    ssize_t size = ctx->bytes_per_line * ctx->lines;

                    if (size != ipc_send (socket, id, flag | TYPE_DIP_DATA,
                                          size, *buffer))
                      {
                        err_minor ("image truncated");
                        status = STATUS_NG;
                      }
                  }
              }
          }
      }
  }

  if (STATUS_NG == status)      /* abort further processing */
    {
      ipc_send (socket, id, flag | TYPE_DIP_DTOR, 0, NULL);
      ipc_recv (socket, &id, &status, NULL);
      return;
    }

  /* acquire DIP results */
  {
    uint8_t req = flag | TYPE_DIP_PARM;
    void   *buf = NULL;
    ipc_dip_parms par;

    if (sizeof (par) == ipc_recv (socket, &id, &req, &buf))
      {
        ssize_t size;

        memcpy (&par, buf, sizeof (par));
        size = par.parms.bytes_per_line * par.parms.lines;

        req = flag | TYPE_DIP_DATA;
        delete (buf);

        if (size == ipc_recv (socket, &id, &req, &buf))
          {
            memcpy (ctx, &par.parms, sizeof (*ctx));
            delete (*buffer);
            *buffer = buf;
          }
        else
          {
            err_minor ("image truncated");
            delete (buf);
          }
      }
  }
  ipc_send (socket, id, flag | TYPE_DIP_DTOR, 0, NULL);
  ipc_recv (socket, &id, &status, NULL);
}
