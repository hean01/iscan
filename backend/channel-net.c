/*  channel_net.c -- network device communication channel
 *  Copyright (C) 2008, 2009  SEIKO EPSON CORPORATION
 *
 *  License: GPLv2+|iscan
 *  Authors: AVASYS CORPORATION
 *
 *  This file is part of the SANE backend distributed with Image Scan!
 *
 *  Image Scan!'s SANE backend is free software.
 *  You can redistribute it and/or modify it under the terms of the GNU
 *  General Public License as published by the Free Software Foundation;
 *  either version 2 of the License or at your option any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *  You ought to have received a copy of the GNU General Public License
 *  along with this package.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 *  Linking Image Scan!'s SANE backend statically or dynamically with
 *  other modules is making a combined work based on this SANE backend.
 *  Thus, the terms and conditions of the GNU General Public License
 *  cover the whole combination.
 *
 *  As a special exception, the copyright holders of Image Scan!'s SANE
 *  backend give you permission to link Image Scan!'s SANE backend with
 *  SANE frontends that communicate with Image Scan!'s SANE backend
 *  solely through the SANE Application Programming Interface,
 *  regardless of the license terms of these SANE frontends, and to
 *  copy and distribute the resulting combined work under terms of your
 *  choice, provided that every copy of the combined work is
 *  accompanied by a complete copy of the source code of Image Scan!'s
 *  SANE backend (the version of Image Scan!'s SANE backend used to
 *  produce the combined work), being distributed under the terms of
 *  the GNU General Public License plus this exception.  An independent
 *  module is a module which is not derived from or based on Image
 *  Scan!'s SANE backend.
 *
 *  As a special exception, the copyright holders of Image Scan!'s SANE
 *  backend give you permission to link Image Scan!'s SANE backend with
 *  independent modules that communicate with Image Scan!'s SANE
 *  backend solely through the "Interpreter" interface, regardless of
 *  the license terms of these independent modules, and to copy and
 *  distribute the resulting combined work under terms of your choice,
 *  provided that every copy of the combined work is accompanied by a
 *  complete copy of the source code of Image Scan!'s SANE backend (the
 *  version of Image Scan!'s SANE backend used to produce the combined
 *  work), being distributed under the terms of the GNU General Public
 *  License plus this exception.  An independent module is a module
 *  which is not derived from or based on Image Scan!'s SANE backend.
 *
 *  Note that people who make modified versions of Image Scan!'s SANE
 *  backend are not obligated to grant special exceptions for their
 *  modified versions; it is their choice whether to do so.  The GNU
 *  General Public License gives permission to release a modified
 *  version without this exception; this exception also makes it
 *  possible to release a modified version which carries forward this
 *  exception.
 */


/*! \file
 *  \brief  Implements a network device communication channel.
 *  \todo   Deal correctly with \c SIGPIPE.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "channel.h"

#include <string.h>
#include <unistd.h>

#include "ipc.h"
#include "net-obj.h"
#include "utils.h"
#include "hw-data.h"


static void channel_net_open  (channel *, SANE_Status *);
static void channel_net_close (channel *, SANE_Status *);

static ssize_t channel_net_send (channel *, const void *,
                                 size_t, SANE_Status *);
static ssize_t channel_net_recv (channel *, void *,
                                 size_t, SANE_Status *);


channel *
channel_net_ctor (channel *self, const char *dev_name, SANE_Status *status)
{
  log_call ("(%p, '%s', %p)", self, dev_name, status);

  if (status) *status = SANE_STATUS_GOOD;

  require (self && dev_name);
  require (0 == strncmp_c (dev_name, "net:", strlen ("net:")));

  self->name = strdup (dev_name);
  if (!self->name)
    {
      if (status) *status = SANE_STATUS_NO_MEM;
      return self->dtor (self);
    }

  self->open  = channel_net_open;
  self->close = channel_net_close;

  self->send = channel_net_send;
  self->recv = channel_net_recv;
  
  return self;
}

/*! Each call must consist of a complete handshake step.
 *  Cannot send in parts.
 */
static ssize_t
channel_net_send (channel *self, const void *buffer,
                  size_t size, SANE_Status *status)
{
  ssize_t n = 0;
  if (0 > self->fd)
    {
      if (status) *status = SANE_STATUS_IO_ERROR;
      return -1;
    }
  if (status) *status = SANE_STATUS_GOOD;

  require (self && buffer);
  require (0 < self->id);

  n = ipc_send (self->fd, self->id, TYPE_ESC, size, buffer);
  if (n != size)
    {
      if (status) *status = SANE_STATUS_IO_ERROR;
    }
  return n;
}

/*! Each call must consist of a complete handshake step.
 *  Cannot receive in parts.
 */
static ssize_t
channel_net_recv (channel *self, void *buffer,
                  size_t size, SANE_Status *status)
{
  char* rbuf = NULL;
  uint16_t id = 0;
  uint8_t ipc_status = STATUS_OK;
  ssize_t n = 0;

  if (0 > self->fd)
    {
      if (status) *status = SANE_STATUS_IO_ERROR;
      return -1;
    }
  if (status) *status = SANE_STATUS_GOOD;

  require (self && buffer);
  require (0 < self->id);

  n = ipc_recv (self->fd, &id, &ipc_status, (void**)&rbuf);
  if (n != size) err_major ("expected %zd bytes, received %zd bytes", size, n);
  if (!rbuf || id != self->id || STATUS_OK != ipc_status || n != size)
    {
      if (status) *status = SANE_STATUS_IO_ERROR;
      delete (rbuf);
      return -1;
    }
  
  memcpy (buffer, rbuf, n);
  delete (rbuf);
  return n;
}

static void
channel_net_open (channel *self, SANE_Status *status)
{
  void* net = NULL;
  ssize_t n = 0;

  uint8_t ipc_status = STATUS_OK;

  char* scanner = self->name + strlen ("net:");

  if (status) *status = SANE_STATUS_GOOD;

  net = net_init ("", NULL);
  if (net) self->fd = net_get_sock (net);
  if (!net || 0 > self->fd)
    {
      if (status) *status = SANE_STATUS_IO_ERROR;
      return;
    }

  n = ipc_send (self->fd, 0, TYPE_OPEN, strlen (scanner), scanner);
  if (n != strlen (scanner))
    {
      self->fd = -1;
      if (status) *status = SANE_STATUS_IO_ERROR;
      return;
    }

  n = ipc_recv (self->fd, &self->id, &ipc_status, NULL);

  if (STATUS_OK != ipc_status || 0 != n)
    {
      self->id = 0;
      self->fd = -1;
      if (status) *status = SANE_STATUS_IO_ERROR;
      return;
    }

  log_info ("Opened network scanner at: %s", scanner);
}

static void
channel_net_close (channel *self, SANE_Status *status)
{
  ssize_t n = 0;

  if (status) *status = SANE_STATUS_GOOD;

  n = ipc_send (self->fd, self->id, TYPE_CLOSE, 0, NULL);
  self->id = 0;
  self->fd = -1;
  if (0 != n)
    {
      if (status) *status = SANE_STATUS_IO_ERROR;
      log_info ("failed to close network scanner: %s",
                self->name + strlen ("net:"));
      return;
    }
  log_info ("closed network scanner: %s", self->name + strlen ("net:"));
}
