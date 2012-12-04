/*  utils.c -- assorted utility functions and macros
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "utils.h"

#include <errno.h>
#include <string.h>
#include <time.h>               /* nanosleep */


/*! Creates a string of 2-character hexadecimal values from a NUL
    terminated character string.

    The implementation acquires the memory needed for the result and
    the caller is responsible for releasing it.

    \return  a pointer to the encoded string or \c NULL if the memory
             required could not be acquired
 */
char *
fw_name_to_hex (const char *fw_name)    /* '\0' terminated */
{
  char *rv, *p;

  log_call();

  if (!fw_name) return NULL;            /* guard clause */

  p = rv = t_malloc (2 * strlen (fw_name) + 1, char);
  if (!rv) return NULL;

  while ('\0' != *fw_name)
  {
    sprintf (p, "%02x", *fw_name);
    p += 2;
    ++fw_name;
  }
  *p = '\0';
  return rv;
}

/*! Sets up a properly initialised resolution info object.
 */
void
init_resolution_info (resolution_info *self, u_char *data)
{
  SANE_Bool (*cond) (const u_char *) = NULL;
  size_t step = 0;

  if (!self) return;

  self->last =  0;		/* set defaults */
  self->size = -1;
  self->list = NULL;
  self->deep = SANE_TRUE;

  if (!data) return;		/* act like a default constructor */

  self->size = 0;
  self->list = t_realloc (NULL, self->size + 1, SANE_Word);

  if (!self->list)
  {
    err_major ("%s", strerror (ENOMEM));
    self->size = -1;
    return;
  }

  if ('R' == data[0])		/* ESC I data block */
  {
    cond = resolution_info_ESC_I_cond;
    step = 3;
  }
  else				/* ESC i data block, hopefully */
  {
    cond = resolution_info_ESC_i_cond;
    step = 2;
  }

  while (cond (data))
  {
    void *p = self->list;

    self->size++;
    self->list = t_realloc (p, self->size + 1, SANE_Word);
    if (!self->list)
    {
      delete (p);

      err_major ("%s", strerror (ENOMEM));
      self->size = -1;
      return;
    }
    self->list[self->size] = data[step - 1] << 8 | data[step - 2];
    data += step;
    log_info ("resolution: %d dpi", self->list[self->size]);
  }
  self->list[0] = self->size;
}

/*! Releases resources held by \a self and resets it to default state.
 */
void
free_resolution_info (resolution_info *self)
{
  if (!self) return;

  if (self->deep)
    delete (self->list);

  init_resolution_info (self, NULL);
}

/*! Makes an optionally deep copy of \a src to \a dest.

    Any resources held by \a dest will be returned to the system.
 */
SANE_Status
copy_resolution_info (resolution_info *dest, const resolution_info *src,
		      SANE_Bool deep)
{
  if (!dest || !src) return SANE_STATUS_INVAL;

  require (!src->list || src->size == src->list[0]);

  if (deep && src->list)	/* copy resolution list */
  {
    size_t     size = (src->size + 1) * sizeof (SANE_Word);
    SANE_Word *list = t_malloc (size, SANE_Word);

    if (!list) return SANE_STATUS_NO_MEM;

    memcpy (list, src->list, size);

    if (dest->deep)
      delete (dest->list);
    dest->list = list;
  }
  else				/* just refer to it */
  {
    if (dest->deep)
      delete (dest->list);
    dest->list = src->list;
  }

  dest->last = src->last;
  dest->size = src->size;
  dest->deep = deep;

  promise (!dest->list || dest->size == dest->list[0]);

  return SANE_STATUS_GOOD;
}


void
_update_ranges (const device *hw, extension *src)
{
  require (hw);
  require (src);

  src->x_range.min = 0;
  src->x_range.max = SANE_FIX (src->max_x * MM_PER_INCH / hw->base_res);
  src->x_range.quant = 0;

  src->y_range.min = 0;
  src->y_range.max = SANE_FIX (src->max_y * MM_PER_INCH / hw->base_res);
  src->y_range.quant = 0;

  if (!hw->cmd->request_identity2) return;

  /* correct for color shuffle offsets */
  src->y_range.max = SANE_FIX ((src->max_y - 2 * hw->max_line_distance)
			       * MM_PER_INCH / hw->base_res);
}


/*! Convenience type to hold document size information.
 */
struct _doc_size_info
{
  const double width;
  const double height;
  const char  *label;
};

/*! Document size information for known sizes. 
 */
static const struct _doc_size_info
doc_size[] = {
  /* second byte bit flag values */
  { 182.00, 257.00, "B5V"},
  { 257.00, 182.00, "B5H"},
  { 148.00, 210.00, "A5V"},
  { 210.00, 148.00, "A5H"},
  { 184.15, 266.70, "EXV"},
  { 266.70, 184.15, "EXH"},
  {   0   ,   0   , "RSV"},	/* reserved */
  {   0   ,   0   , "UNK"},	/* unknown */
  /* first byte bit flag values */
  { 297.00, 420.00, "A3V"},
  { 279.40, 431.80, "WLT"},
  { 257.00, 364.00, "B4V"},
  { 215.90, 355.60, "LGV"},
  { 210.00, 297.00, "A4V"},
  { 297.00, 210.00, "A4H"},
  { 215.90, 279.40, "LTV"},
  { 279.40, 215.90, "LTH"},
};


void
_update_doc_size (extension *src, uint16_t value)
{
  const uint16_t DOC_MASK = ~0x0200;

  size_t i = 0;

  require (src);

  if ((DOC_MASK & value) != value)
  {
    err_minor ("clearing reserved bit flags to match spec");
    value &= DOC_MASK;
  }

  if (0 == value)		/* size detection not supported */
  {
    src->doc_x = 0;
    src->doc_y = 0;
    return;
  }

  while (!(0x8000 & value) && (num_of (doc_size) > i))
  {
    value = value << 1;
    ++i;
  }

  if (0 != strcmp_c ("UNK", doc_size[i].label))
  {
    src->doc_x = doc_size[i].width;
    src->doc_y = doc_size[i].height;
  }
  else
  {
    src->doc_x = SANE_UNFIX (src->x_range.max);
    src->doc_y = SANE_UNFIX (src->y_range.max);
  }

  value = value << 1;
  if (0 != value)
  {
    err_minor ("device detected multiple document sizes!\n");
  }

  log_info ("detected document size: %s (%.2fmm x %.2fmm)",
            doc_size[i].label, src->doc_x, src->doc_y);
}

int
microsleep (size_t usec)
{
  struct timespec ts;
  ts.tv_sec  =  usec / 1000000;
  ts.tv_nsec = (usec % 1000000) * 1000;

  return nanosleep (&ts, NULL);
}
