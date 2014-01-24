/*  channel.h -- device communication channels
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


#ifndef channel_h_included
#define channel_h_included

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sane/sane.h>
#include <sys/types.h>
#include <stdint.h>

#include "defines.h"

typedef enum
{
  CHAN_NET = 0,
  CHAN_PIO,
  CHAN_SCSI,
  CHAN_USB,
  CHAN_INTERP
}
channel_type;

typedef struct channel
{
  struct channel * (*ctor) (struct channel *self,
                            const char *dev_name, SANE_Status *status);
  struct channel * (*dtor) (struct channel *self);

  void (*open)  (struct channel *self, SANE_Status *status);
  void (*close) (struct channel *self, SANE_Status *status);

  bool (*is_open) (const struct channel *self);

  ssize_t (*send) (struct channel *self, const void *buffer,
                   size_t buf_size, SANE_Status *status);
  ssize_t (*recv) (struct channel *self, void *buffer,
                   size_t buf_size, SANE_Status *status);
  size_t (*max_request_size) (const struct channel *self);
  void (*set_max_request_size) (struct channel *self, size_t size);

  char *name;
  channel_type type;
  int   fd;
  uint16_t id;  /* target scanner ID when used with the network plugin
                 * or USB product ID
                 */
  size_t max_size;

  struct interpreter_type *interpreter;

} channel;


channel * channel_create (const char *dev_name, SANE_Status *status);


/* Convenience API */
ssize_t channel_send (channel *ch, const void *buffer,
                      size_t size, SANE_Status *status);
ssize_t channel_recv (channel *ch, void *buffer,
                      size_t size, SANE_Status *status);
ssize_t channel_recv_all (channel *ch, void *buffer,
                          size_t size, SANE_Status *status);
ssize_t channel_recv_all_retry (channel *ch, void *buffer, size_t size,
                                size_t max_attempts, SANE_Status *status);


#ifdef USE_PROTECTED_CHANNEL_API

channel *channel_dtor (channel *self);
bool channel_is_open (const channel *self);
size_t channel_max_request_size (const channel *self);
void channel_set_max_request_size (struct channel *self, size_t size);

#endif


#endif  /* !defined (channel_h_included) */
