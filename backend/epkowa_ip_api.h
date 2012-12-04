/* epkowa_ip_api.h -- for scanners that don't speak EPSON ESC/I natively
   Copyright (C) 2005  SEIKO EPSON CORPORATION

   This file is part of the EPKOWA SANE backend.
   It declares the backend's "Interpreter" interface.

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

#ifndef epkowa_ip_api_h_included
#define epkowa_ip_api_h_included

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#include "defines.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef ssize_t io_callback (void *buffer, size_t length);
typedef ssize_t ctrl_callback (size_t request_type, size_t request,
                               size_t value, size_t index,
                               size_t size, void *buffer);

bool int_init (int fd, io_callback *read, io_callback *write);
bool int_init_with_ctrl (int fd, io_callback *read, io_callback *write,
                         ctrl_callback *ctrl);
void int_fini (void);

int int_read  (void *buffer, size_t length);
int int_write (void *buffer, size_t length);

void int_power_saving_mode (void);

int  function_s_0 (unsigned int offset,
		   unsigned int width,
		   unsigned int resolution,
		   unsigned int opt_resolution,
		   double * table);

void function_s_1 (uint8_t *in_buf, uint8_t *out_buf,
		   unsigned int width, bool color, double *table);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* !epkowa_ip_api_h_included */
