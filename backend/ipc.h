/*  ipc.h -- inter-process communication (IPC) support
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


#ifndef ipc_h
#define ipc_h

#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#include <sane/sane.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*! \brief Exceptional return values from ipc_recv() and ipc_send()
 *  In particular, a return value of zero from read() conflicts with
 *  our ipc_recv() API where a return value of zero does not indicate
 *  EOF, but a successful read of zero bytes.
 */
#define MERR -1               /*!< read/write error */
#define MEOF -2               /*!< eof; only returned by ipc_recv() */

enum {
  TYPE_ESC = 1,
  TYPE_INIT,
  TYPE_DIE,
  TYPE_OPEN,
  TYPE_CLOSE,
  TYPE_LIST,
  TYPE_STATUS,

  TYPE_DIP_CTOR = 0x01,
  TYPE_DIP_DTOR = 0x02,
  TYPE_DIP_PARM = 0x03,
  TYPE_DIP_DATA = 0x04,
  TYPE_DIP_MASK = 0x0f,
  TYPE_DIP_FLAG = 0xf0,

  TYPE_DIP_SKEW_FLAG = 0x10,
  TYPE_DIP_SKEW_CTOR = TYPE_DIP_SKEW_FLAG | TYPE_DIP_CTOR,
  TYPE_DIP_SKEW_DTOR = TYPE_DIP_SKEW_FLAG | TYPE_DIP_DTOR,
  TYPE_DIP_SKEW_PARM = TYPE_DIP_SKEW_FLAG | TYPE_DIP_PARM,
  TYPE_DIP_SKEW_DATA = TYPE_DIP_SKEW_FLAG | TYPE_DIP_DATA,
  TYPE_DIP_SKEW_MASK = TYPE_DIP_SKEW_FLAG | TYPE_DIP_MASK,

  TYPE_DIP_CROP_FLAG = 0x20,
  TYPE_DIP_CROP_CTOR = TYPE_DIP_CROP_FLAG | TYPE_DIP_CTOR,
  TYPE_DIP_CROP_DTOR = TYPE_DIP_CROP_FLAG | TYPE_DIP_DTOR,
  TYPE_DIP_CROP_PARM = TYPE_DIP_CROP_FLAG | TYPE_DIP_PARM,
  TYPE_DIP_CROP_DATA = TYPE_DIP_CROP_FLAG | TYPE_DIP_DATA,
  TYPE_DIP_CROP_MASK = TYPE_DIP_CROP_FLAG | TYPE_DIP_MASK,
};

enum {
  STATUS_OK = 0,
  STATUS_NG
};

ssize_t ipc_send (int sock, uint16_t id, uint8_t type_status,
                  size_t size, const void* payload);
ssize_t ipc_recv (int sock, uint16_t *id, uint8_t *type_status,
                  void** payload);

  typedef struct
  {
    pid_t pid;
    int   port;
    int   socket;

    const char *name;

  } process;

  /*! \brief  Attempts to start \a program as a child process
   */
  process * ipc_exec (const char *program, const char *pkglibdir,
                      SANE_Status *status);

  /*! \brief  Terminates a child process
   */
  process * ipc_kill (process *child);

  typedef struct
  {
    SANE_Parameters parms;
    SANE_Int        res_x;
    SANE_Int        res_y;
    SANE_Int        gamma;
    SANE_Bool       bside;
    char            fw_name[16 + 1];

  } ipc_dip_parms;

  /*! \brief Performs an image processing action
   *
   *  If any of the IPC messaging signals an error, the original image
   *  data will not be modified at all.  That is, \a ctx and \a buffer
   *  remain unchanged in such a case.
   */
  void ipc_dip_proc (process *child, int flag, const ipc_dip_parms *p,
                     SANE_Parameters *ctx, void **buffer);

#ifdef __cplusplus
}       /* extern "C" */
#endif

#endif  /* !defined (ipc_h) */
