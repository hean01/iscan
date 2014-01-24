/*  channel_pio.c -- parallel device communication channel
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
    \brief  Implements a parallel device communication channel.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "channel.h"

#include <string.h>

#include "utils.h"

/* Deprecated includes */
#include "sane/sanei_pio.h"


static void channel_pio_open  (channel *, SANE_Status *);
static void channel_pio_close (channel *, SANE_Status *);

static ssize_t channel_pio_send (channel *, const void *,
                                 size_t, SANE_Status *);
static ssize_t channel_pio_recv (channel *, void *,
                                 size_t, SANE_Status *);

static void channel_pio_set_max_request_size (channel *, size_t);

channel *
channel_pio_ctor (channel *self, const char *dev_name, SANE_Status *status)
{
  require (self && dev_name);
  require (0 == strncmp_c (dev_name, "pio:", strlen ("pio:")));

  self->open  = channel_pio_open;
  self->close = channel_pio_close;

  self->send = channel_pio_send;
  self->recv = channel_pio_recv;

  self->set_max_request_size = channel_pio_set_max_request_size;

  if (status) *status = SANE_STATUS_UNSUPPORTED;
  return self->dtor (self);
}

static ssize_t
channel_pio_send (channel *self, const void *buffer,
                  size_t size, SANE_Status *status)
{
  SANE_Status s = SANE_STATUS_GOOD;
  ssize_t n = sanei_pio_write (self->fd, buffer, size);

  if (size != n)
    s = SANE_STATUS_INVAL;
  if (status) *status = s;

  return n;
}

static ssize_t
channel_pio_recv (channel *self, void *buffer,
                  size_t size, SANE_Status *status)
{
  SANE_Status s = SANE_STATUS_GOOD;
  ssize_t n = sanei_pio_read (self->fd, buffer, (size_t) size);

  if (size != n)
    s = SANE_STATUS_INVAL;
  if (status) *status = s;

  return n;
}

static void
channel_pio_open (channel *self, SANE_Status *status)
{
  SANE_Status s = sanei_pio_open (self->name, &self->fd);

  if (SANE_STATUS_GOOD != s)
    {
      err_fatal ("can not open %s (%s)", self->name, sane_strstatus (s));
    }
  if (status) *status = s;
}

static void
channel_pio_close (channel *self, SANE_Status *status)
{
  sanei_pio_close (self->fd);
  self->fd = -1;
}

static void
channel_pio_set_max_request_size (channel *self, size_t size)
{
  require (self);

  self->max_size = (size < (32 * 1024)) ? size : (32 * 1024);
}
