/*  channel_usb.c -- USB device communication channel
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
    \brief  Implements a USB communication channel.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define USE_PROTECTED_CHANNEL_API
#include "channel.h"

#include <string.h>

#include "utils.h"

/* Deprecated includes */
#include "include/sane/sanei_usb.h"
#include "epkowa_ip.h"


static void channel_usb_open  (channel *, SANE_Status *);
static void channel_usb_close (channel *, SANE_Status *);

static ssize_t channel_usb_send (channel *, const void *,
                                 size_t, SANE_Status *);
static ssize_t channel_usb_recv (channel *, void *,
                                 size_t, SANE_Status *);


channel *
channel_usb_ctor (channel *self, const char *dev_name, SANE_Status *status)
{
  size_t name_len = 0;

  require (self && dev_name);
  require (0 == strncmp_c (dev_name, "usb:", strlen ("usb:")));

  dev_name += strlen ("usb:");
  name_len  = strlen ("libusb:") + strlen (dev_name) + 1;

  self->name = t_malloc (name_len, char);
  if (!self->name)
    {
      if (status) *status = SANE_STATUS_NO_MEM;
      return self->dtor (self);
    }
  self->name[0] = '\0';
  strcat (self->name, "libusb:");
  strcat (self->name, dev_name);

  self->open  = channel_usb_open;
  self->close = channel_usb_close;

  self->send = channel_usb_send;
  self->recv = channel_usb_recv;

  self->max_size = 128 * 1024;

  return self;
}

static ssize_t
channel_usb_send (channel *self, const void *buffer,
                  size_t size, SANE_Status *status)
{
  ssize_t n = size;

  if (self->interpreter)
    {
      n = self->interpreter->send (self, buffer, size, status);
    }
  else
    {
      SANE_Status s;
      s = sanei_usb_write_bulk (self->fd, buffer, (size_t *)&n);
      if (status) *status = s;
    }

  return n;
}

static ssize_t
channel_usb_recv (channel *self, void *buffer,
                  size_t size, SANE_Status *status)
{
  ssize_t n = size;

  if (self->interpreter)
    {
      n = self->interpreter->recv (self, buffer, size, status);
    }
  else
    {
      SANE_Status s = SANE_STATUS_GOOD;
      s = sanei_usb_read_bulk (self->fd, (SANE_Byte *) buffer, (size_t *)&n);
      if (status) *status = s;
    }

  return n;
}

static void
channel_usb_open (channel *self, SANE_Status *status)
{
  SANE_Status s;

  s = sanei_usb_open (self->name, &self->fd);

  if (SANE_STATUS_GOOD == s)
    {
      SANE_Word product_id = -1;

      sanei_usb_get_vendor_product (self->fd, NULL, &product_id);

      if (-1 != product_id)
        {
          self->id = product_id;
        }
    }

  if (self->interpreter && SANE_STATUS_GOOD == s)
    {
      if (0 > self->interpreter->open (self))
        {
          s = SANE_STATUS_IO_ERROR;
        }
    }

  if (status) *status = s;
}

static void
channel_usb_close (channel *self, SANE_Status *status)
{
  SANE_Status s = SANE_STATUS_GOOD;

  if (self->interpreter)
    {
      self->interpreter->close (self);
    }

  sanei_usb_close (self->fd);
  self->fd = -1;
  if (status) *status = s;
}


static channel * channel_interpreter_dtor (channel *self);

channel *
channel_interpreter_ctor (channel *self, const char *dev_name,
                          SANE_Status *status)
{
  char *name = NULL;
  size_t name_len = 0;

  require (self && dev_name);
  require (0 == strncmp_c (dev_name, "interpreter:", strlen ("interpreter:")));

  dev_name += strlen ("interpreter:");
  name_len  = strlen ("usb:") + strlen (dev_name) + 1;

  name = t_malloc (name_len, char);
  if (!name)
    {
      if (status) *status = SANE_STATUS_NO_MEM;
      return self->dtor (self);
    }
  strcpy (name, "usb:");
  strcat (name, dev_name);

  self = channel_usb_ctor (self, name, status);
  delete (name);

  if (self)
    {
      SANE_Status s = SANE_STATUS_GOOD;
      SANE_Word vendor;
      SANE_Word product;

      self->open (self, &s);
      if (SANE_STATUS_GOOD == s)
        {
          s = sanei_usb_get_vendor_product (self->fd,
                                            &vendor, &product);
        }
      self->close (self, NULL);
      if (SANE_STATUS_GOOD == s)
        {
          s = create_interpreter (self, product);
        }

      if (!self->interpreter)
        {
          if (status) *status = s;
          return self->dtor (self);
        }
      else
        {
          self->dtor = channel_interpreter_dtor;
        }
    }

  self->max_size = 32 * 1024;

  return self;
}

static channel *
channel_interpreter_dtor (channel *self)
{
  require (self);

  if (self->interpreter)
    {
      self->interpreter->dtor (self);
    }
  self->dtor = channel_dtor;
  return self->dtor (self);
}
