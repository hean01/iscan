/*  utils.h -- assorted utility functions and macros
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


#ifndef utils_h_included
#define utils_h_included

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include "device.h"


/*! \brief  Encodes a fw_name as a string of hexadecimals.
 */
char * fw_name_to_hex (const char *fw_name);

/*! \brief  Converts a buffer to an unsigned 32-bit integer.
 */
inline static uint32_t
buf_to_uint32 (const byte *buf)
{
  return buf[3] << 24 | buf[2] << 16 | buf[1] << 8 | buf[0];
}

/*! \brief  Converts a buffer to an unsigned 16-bit integer.
 */
inline static uint16_t
buf_to_uint16 (const byte *buf)
{
  return buf[1] << 8 | buf[0];
}

/*! \brief  Converts an unsigned 32-bit integer to a buffer.
 */
inline static void
uint32_to_buf (uint32_t val, byte *buf)
{
  if (!buf) return;

  buf[0] = val;
  buf[1] = val >> 8;
  buf[2] = val >> 16;
  buf[3] = val >> 24;
}

/*! \brief  Converts an unsigned 16-bit integer to a buffer.
 */
inline static void
uint16_to_buf (uint16_t val, byte *buf)
{
  if (!buf) return;

  buf[0] = val;
  buf[1] = val >> 8;
}


/* Scan area related queries and computations.
 */

/*! \brief  Tells whether an extension supports document size detection.
 */
inline static SANE_Bool
has_size_check_support (const extension *src)
{
  return src->has_size_check;
}

/*! \brief  Recomputes an extension's scan area dimensions (in mm).
    \hideinitializer
 */
#define update_ranges(hw,src) _update_ranges (hw, (extension *) src)

void _update_ranges (const device *hw, extension *src);

/*! \brief  Re-establishes the detected document size.
    \hideinitializer
 */
#define update_doc_size(src,value) _update_doc_size ((extension *) src, value)

void _update_doc_size (extension *src, uint16_t value);


/* Resolution information handlers.
 */

void init_resolution_info (resolution_info *self, byte *data);
void free_resolution_info (resolution_info *self);
SANE_Status copy_resolution_info (resolution_info *dest,
                                  const resolution_info *src, SANE_Bool deep);

/*! \brief  Tells whether we are looking at a resolution in an ESC I reply.
 */
inline static SANE_Bool
resolution_info_ESC_I_cond (const u_char *data)
{
  return ('R' == data[0]);
}

/*! \brief  Tells whether we are looking at a resolution in an ESC i reply.
 */
inline static SANE_Bool
resolution_info_ESC_i_cond (const u_char *data)
{
  return (0 != data[0] || 0 != data[1]);
}

/*! \brief  A wrapper around strcmp that checks for NULL strings
 */
inline static int
strcmp_c (const char *s1, const char *s2)
{
  if (!s1 && !s2) return 0;
  if (s1 && !s2) return 1;
  if (!s1 && s2) return -1;

  return strcmp (s1, s2);
}

/*! \brief  A wrapper around strncmp that checks for NULL strings
 */
inline static int
strncmp_c (const char *s1, const char *s2, size_t n)
{
  if (!s1 && !s2) return 0;
  if (s1 && !s2) return 1;
  if (!s1 && s2) return -1;

  return strncmp (s1, s2, n);
}

int microsleep (size_t usec);


#endif  /* !defined (utils_h_included) */
