/* epkowa_ip.h -- for scanners that don't speak EPSON ESC/I natively
   Copyright (C) 2005  Olaf Meeuwissen
   Copyright (C) 2009  SEIKO EPSON CORPORATION

   This file is part of the EPKOWA SANE backend.
   It declares a wrapper around the backend's "Interpreter" interface.

   The EPKOWA SANE backend is free software.
   You can redistribute it and/or modify it under the terms of the GNU
   General Public License as published by the Free Software Foundation;
   either version 2 of the License or at your option any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY;  without even the implied warranty of FITNESS
   FOR A PARTICULAR PURPOSE or MERCHANTABILITY.
   See the GNU General Public License for more details.

   You should have received a verbatim copy of the GNU General Public
   License along with this program; if not, write to:

       Free Software Foundation, Inc.
       59 Temple Place, Suite 330
       Boston, MA  02111-1307  USA

   Linking the EPKOWA SANE backend statically or dynamically with
   other modules is making a combined work based on the EPKOWA SANE
   backend.  Thus, the terms and conditions of the GNU General Public
   License cover the whole combination.

   As a special exception, the copyright holders of the EPKOWA SANE
   backend give you permission to link the EPKOWA SANE backend with
   independent modules that communicate with the EPKOWA SANE backend
   solely through the "Interpreter" interface, regardless of the
   license terms of these independent modules, and to copy and
   distribute the resulting combined work under terms of your choice,
   provided that every copy of the combined work is accompanied by a
   complete copy of the source code of the EPKOWA SANE backend (the
   version of the EPKOWA SANE backend used to produce the combined
   work), being distributed under the terms of the GNU General Public
   License plus this exception.  An independent module is a module
   which is not derived from or based on the EPKOWA SANE backend.

   Note that people who make modified versions of the EPKOWA SANE
   backend are not obligated to grant this special exception for their
   modified versions; it is their choice whether to do so.  The GNU
   General Public License gives permission to release a modified
   version without this exception; this exception also makes it
   possible to release a modified version which carries forward this
   exception.
 */

#ifndef epkowa_ip_h_included
#define epkowa_ip_h_included

#include <ltdl.h>

#include "epkowa.h"
#include "epkowa_ip_api.h"

/* Insulate the interpreter_type declaration against changes.
 */
typedef channel  device_type;


/*! An object-like type for ESC/I interpreters.
 */
struct interpreter_type
{
  /* public members */

  SANE_Status (*dtor) (device_type *device);

  int (*open)  (device_type *device);
  int (*close) (device_type *device);

  ssize_t (*recv) (device_type *device, void *buffer, size_t size,
		   SANE_Status *status);
  ssize_t (*send) (device_type *device, const void *buffer, size_t size,
		   SANE_Status *status);

  /* FIXME: very questionable public API for an _interpreter_ as this
     seems to do some kind of image processing -> suggest to refactor
     this out into a separate object */

  SANE_Status (*free) (device_type *device);

  SANE_Status (*ftor0) (device_type *device, SANE_Parameters *params,
			SANE_Byte *ptr, SANE_Byte *end);
  SANE_Status (*ftor1) (device_type *device, SANE_Parameters *params,
			int depth, int left, int x_dpi, int optical_res);


  /* private members */

  device_type *_device;
  void	      *_module;

  int _tick_count;

  double    *_table;
  SANE_Byte *_buffer;

  /* hooks for all API declared in epkowa_ip_api.h */

  bool (*_init)  (int fd, io_callback *read, io_callback *write);
  bool (*_init_with_ctrl) (int fd, io_callback *read, io_callback *write,
                           ctrl_callback *ctrl);
  void (*_fini)  (void);
  int  (*_read)  (void *buf, size_t size);
  int  (*_write) (void *buf, size_t size);
  void (*_power) (void);
  int  (*_s_0)   (unsigned int offset,	   unsigned int width,
		  unsigned int resolution, unsigned int opt_resolution,
		  double *table);
  void (*_s_1)   (uint8_t *in_buf, uint8_t *out_buf,
		  unsigned int width, bool color, double *table);
};

/*! An abstract, factory-like ctor for interpreters.
 */
SANE_Status create_interpreter (device_type *device,
				unsigned int usb_product_id);

#endif /* !epkowa_ip_h_included */
