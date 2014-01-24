/*  channel.c -- device communication channel
 *  Copyright (C) 2008, 2009, 2013  SEIKO EPSON CORPORATION
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
    \brief  Implements a hardware communication channel.

    Hardware channels supported are usb, scsi and parallel.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define USE_PROTECTED_CHANNEL_API
#include "channel.h"

#include "utils.h"

extern channel *
channel_net_ctor (channel *self, const char *dev_name, SANE_Status *status);

extern channel *
channel_pio_ctor (channel *self, const char *dev_name, SANE_Status *status);

extern channel *
channel_scsi_ctor (channel *self, const char *dev_name, SANE_Status *status);

extern channel *
channel_usb_ctor (channel *self, const char *dev_name, SANE_Status *status);

extern channel *
channel_interpreter_ctor (channel *self,
                          const char *dev_name, SANE_Status *status);


#include <string.h>

#include "epkowa_ip.h"

#include "hw-data.h"


/*! A ::channel factory method.
 *
 *  Creates and initializes a channel object to match a \a dev_name.
 *
 *  \todo Validate \a dev_name?
 */
channel *
channel_create (const char *dev_name, SANE_Status *status)
{
  channel *ch = NULL;

  require (dev_name);

  if (status) *status = SANE_STATUS_GOOD;

  ch = t_calloc (1, channel);
  if (!ch)
    {
      if (status) *status = SANE_STATUS_NO_MEM;
      return NULL;
    }

  ch->dtor    = channel_dtor;
  ch->is_open = channel_is_open;
  ch->max_request_size     = channel_max_request_size;
  ch->set_max_request_size = channel_set_max_request_size;

  ch->fd = -1;
  ch->id =  0;
  ch->max_size = 32 * 1024;

  if (0 == strncmp_c (dev_name, "net:", strlen ("net:")))
    {
      ch->ctor = channel_net_ctor;
      ch->type = CHAN_NET;
    }
  if (0 == strncmp_c (dev_name, "pio:", strlen ("pio:")))
    {
      ch->ctor = channel_pio_ctor;
      ch->type = CHAN_PIO;
    }
  if (0 == strncmp_c (dev_name, "scsi:", strlen ("scsi:")))
    {
      ch->ctor = channel_scsi_ctor;
      ch->type = CHAN_SCSI;
    }
  if (0 == strncmp_c (dev_name, "usb:", strlen ("usb:")))
    {
      ch->ctor = channel_usb_ctor;
      ch->type = CHAN_USB;
    }
  if (0 == strncmp_c (dev_name, "interpreter:", strlen ("interpreter:")))
    {
      ch->ctor = channel_interpreter_ctor;
      ch->type = CHAN_INTERP;
    }

  if (!ch->ctor)
    {
      err_major ("unsupported channel for '%s'", dev_name);
      if (status) *status = SANE_STATUS_UNSUPPORTED;
      delete (ch);
      return NULL;
    }

  return ch->ctor (ch, dev_name, status);
}

/*! Logging wrapper around a channel's send() method.
 */
ssize_t
channel_send (channel* ch, const void *buffer, size_t size,
              SANE_Status *status)
{
  ssize_t n = 0;

  log_call ("(%zd)", size);
  dbg_hex (buffer, size);

  n = ch->send (ch, buffer, size, status);

  log_call ("transferred %zd bytes", n);
  return n;
}

/*! Logging wrapper around a channel's recv() method.
 */
ssize_t
channel_recv (channel *ch, void *buffer, size_t size, SANE_Status *status)
{
  ssize_t n = 0;

  log_call ("(%zd)", size);

  if (size < 256)
    memset (buffer, 0x00, size);

  n = ch->recv (ch, buffer, size, status);

  if (0 < n)
    {
      if (size < 256)
        { dbg_hex (buffer, n); }
      else
        { dbg_img (buffer, n); }
    }

  log_call ("transferred %zd bytes", n);
  return n;
}

/*! Throttle the number of bytes read in a single go
 */
static ssize_t
channel_recv_throttle (channel *ch, void *buffer, size_t size,
                       SANE_Status *status)
{
  size_t max = ch->max_request_size (ch);
  return ch->recv (ch, buffer, size < max ? size : max, status);
}

ssize_t channel_recv_all (channel *ch, void *buffer,
                          size_t size, SANE_Status *status)
{
  return channel_recv_all_retry (ch, buffer, size, 1, status);
}

ssize_t
channel_recv_all_retry (channel *ch, void *buffer, size_t size,
                        size_t max_attempts, SANE_Status *status)
{
  SANE_Status s = SANE_STATUS_GOOD;

  ssize_t n = 0;
  ssize_t t = 0;
  size_t attempts = 0;

  log_call ("(%zd)", size);

  while (n < size && attempts < max_attempts)
   {
     t = channel_recv_throttle (ch, buffer + n, size - n, &s);
     if (SANE_STATUS_GOOD != s || 0 >= t)
       {
         ++attempts;
         log_info ("attempts: %zd/%zd", attempts, max_attempts);
       }
     if (0 < t) n += t;
     log_call ("transferred %zd bytes, total %zd/%zd", t, n, size);
   }

  if (0 < n)
    {
      if (size < 256)
        { dbg_hex (buffer, n); }
      else
        { dbg_img (buffer, n); }
    }

  if (status) *status = s;

  return n;
}

/*! Tells whether a channel is ready to send() and recv() data.
 */
bool
channel_is_open (const struct channel *self)
{
  return (self && 0 <= self->fd);
}

/*! Indicates the maximum number of bytes the channel should read
 *  in a singe request.
 */
size_t
channel_max_request_size (const struct channel *self)
{
  require (self);

  return self->max_size;
}

/*! Change the maximum number of bytes a channel should read in a
 *  single request.
 */
void
channel_set_max_request_size (struct channel *self, size_t size)
{
  require (self);

  self->max_size = size;
}

/*! "Base class" destructor.
 */
channel *
channel_dtor (struct channel *self)
{
  SANE_Status status = SANE_STATUS_GOOD;

  log_call ("(fd = %d)", self->fd);

  if (!self) return NULL;

  if (self->interpreter) self->interpreter->dtor (self);

  if (self->is_open (self))
  {
    self->close (self, &status);
  }

  delete (self->name);
  delete (self);

  return NULL;
}
