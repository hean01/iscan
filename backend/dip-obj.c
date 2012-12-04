/*  dip-obj.c -- digital image processing functionality singleton
 *  Copyright (C) 2011  SEIKO EPSON CORPORATION
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

#include <math.h>
#include <string.h>

#include "dip-obj.h"
#include "defines.h"
#include "hw-data.h"
#include "include/sane/sanei_magic.h"
#include "ipc.h"

#ifndef ENABLE_SANEI_MAGIC
#define ENABLE_SANEI_MAGIC 0
#endif

/*! \brief  Constrain a value \a v in the interval \c [lo,hi]
 */
#define clamp(v,lo,hi) \
  ((v) < (lo)          \
   ? (lo)              \
   : ((v) > (hi)       \
      ? (hi)           \
      : (v)))          \
  /**/

typedef struct
{
  process *plugin;

  void (*autocrop) ();
  void (*deskew) (buffer *, int, int);

} dip_type;

static dip_type *dip = NULL;

static void esdip_crop (buffer *buf, const device *hw, unsigned int count,
                        const Option_Value *val);
static void esdip_turn (buffer *buf, int res_x, int res_y);

static void magic_crop (buffer *buf, int res_x, int res_y);
static void magic_turn (buffer *buf, int res_x, int res_y);

void *
dip_init (const char *pkglibdir, SANE_Status *status)
{
  SANE_Status s = SANE_STATUS_GOOD;

  log_call ("(%s, %p)", pkglibdir, status);

  if (dip)
    {
      err_minor ("been here, done that");
      if (status) *status = s;
      return dip;
    }

  dip = t_calloc (1, dip_type);
  if (!dip)
    {
      if (status) *status = SANE_STATUS_NO_MEM;
      return dip;
    }

  dip->plugin = ipc_exec ("esdip", pkglibdir, status);

  if (dip->plugin)
    {
      dip->autocrop = esdip_crop;
      dip->deskew   = esdip_turn;
    }
  else if (ENABLE_SANEI_MAGIC)  /* use free alternative API */
    {
      sanei_magic_init ();
      dip->autocrop = magic_crop;
      dip->deskew   = magic_turn;
    }

  if (status) *status = s;
  return dip;
}

void *
dip_exit (void *self)
{
  log_call ("(%p)", self);
  require (dip == self);

  if (dip)
    {
      if (dip->plugin)
        {
          dip->plugin = ipc_kill (dip->plugin);
        }
      else
        {
          /* sanei_magic_exit, if there was one */
        }
      delete (dip);
    }

  return dip;
}

bool
dip_needs_whole_image (const void *self, const Option_Value *val,
                       const SANE_Option_Descriptor *opt)
{
  require (dip == self && val);

  /* esdip_deskew and esdip_autocrop do not support this
   */
  if (val[OPT_X_RESOLUTION].w != val[OPT_Y_RESOLUTION].w)
    return false;

  return ((SANE_OPTION_IS_ACTIVE (opt[OPT_DESKEW].cap)
           && val[OPT_DESKEW].b)
          || (SANE_OPTION_IS_ACTIVE (opt[OPT_AUTOCROP].cap)
              && val[OPT_AUTOCROP].b));
}

void
dip_apply_LUT (const void *self, const buffer *buf,
               const LUT *m)
{
  require (dip == self && buf && m);
  require (m->depth == buf->ctx.depth);

  /**/ if (16 == buf->ctx.depth)
    {
      uint16_t *p = (uint16_t *) buf->ptr;
      uint16_t *e = (uint16_t *) buf->end;

      while (p < e)
        {
          *p = * (uint16_t *) (m->lut + 2 * *p);
          ++p;
        }
    }
  else if ( 8 == buf->ctx.depth)
    {
      SANE_Byte *p = buf->ptr;
      SANE_Byte *e = buf->end;

      while (p < e)
        {
          *p = m->lut[*p];
          ++p;
        }
    }
  else
    err_major ("noop: unsupported bit depth %d", buf->ctx.depth);
}

void
dip_apply_LUT_RGB (const void *self, const buffer *buf,
                   const LUT *r, const LUT *g, const LUT *b)
{
  require (dip == self && buf && r && g && b);
  require (r->depth == buf->ctx.depth);
  require (g->depth == buf->ctx.depth);
  require (b->depth == buf->ctx.depth);

  if (SANE_FRAME_RGB != buf->ctx.format)
    {
      err_minor ("noop: image data not in RGB format");
      return;
    }

  /**/ if (16 == buf->ctx.depth)
    {
      uint16_t *p = (uint16_t *) buf->ptr;
      uint16_t *e = (uint16_t *) buf->end;

      while (p < e)
        {
          p[0] = * (uint16_t *) (r->lut + 2 * p[0]);
          p[1] = * (uint16_t *) (g->lut + 2 * p[1]);
          p[2] = * (uint16_t *) (b->lut + 2 * p[2]);
          p += 3;
        }
    }
  else if ( 8 == buf->ctx.depth)
    {
      SANE_Byte *p = buf->ptr;
      SANE_Byte *e = buf->end;

      while (p < e)
        {
          p[0] = r->lut[p[0]];
          p[1] = g->lut[p[1]];
          p[2] = b->lut[p[2]];
          p += 3;
        }
    }
  else
    err_major ("noop: unsupported bit depth %d", buf->ctx.depth);
}

/*! \brief  Destroys a LUT object
 */
LUT *
dip_destroy_LUT (const void *self, LUT *m)
{
  require (dip == self);

  if (m) delete (m->lut);
  delete (m);

  return m;
}

/*! \brief  Creates a LUT with an encoding \a gamma
 *
 *  \sa http://en.wikipedia.org/wiki/Gamma_correction
 */
LUT *
dip_gamma_LUT (const void *self, int depth,
               double gamma)
{
  SANE_Byte *lut;
  LUT       *m;

  size_t i;
  double max;

  require (dip == self);
  require (8 == depth || 16 == depth);

  lut = t_malloc ((1 << depth) * (depth / 8), SANE_Byte);
  m   = t_malloc (1, LUT);

  if (!lut || !m)
    {
      delete (lut);
      delete (m);
      return m;
    }

  m->lut   = lut;
  m->depth = depth;

  max = (1 << depth) - 1;

  for (i = 0; i < (1 << depth); ++i)
    {
      double value = max * pow (i / max, 1 / gamma);

      if (16 == depth)
        {
          uint16_t *p  = (uint16_t *) lut;

          p[i] = clamp (value, 0, max);
        }
      else
        {
          lut[i] = clamp (value, 0, max);
        }
    }

  return m;
}

LUT *
dip_iscan_BCHS_LUT (const void *self, int depth,
                    double brightness, double contrast,
                    double highlight, double shadow)
{
  SANE_Byte *lut;
  LUT       *m;

  size_t i;
  size_t max;

  int32_t b, c, h, s, f;

  require (dip == self);
  require (-1 <= brightness && brightness <= 1);
  require (-1 <= contrast   && contrast   <= 1);
  require ( 0 <= highlight  && highlight  <= 1);
  require ( 0 <= shadow     && shadow     <= 1);
  require (8 == depth || 16 == depth);

  lut = t_malloc ((1 << depth) * (depth / 8), SANE_Byte);
  m   = t_malloc (1, LUT);
  if (!lut || !m)
    {
      delete (lut);
      delete (m);
      return m;
    }

  m->lut   = lut;
  m->depth = depth;

  /* Compute algorithm parameters by scaling the double values into
   * integral values that correspond to the bit depth.  Computation
   * of loop variable independent parts is done by absorption into
   * existing variables.
   */
  f = (1 << (depth - 1)) - 1;
  s =  f * shadow;
  h = -f * highlight;  h += 2 * f + 1;
  b =  f * brightness;
  c = ((0 > contrast)
       ? f * contrast
       : (h - s) / 2 * contrast);

  log_data ("b = %d", b);
  log_data ("c = %d", c);
  log_data ("h = %d", h);
  log_data ("s = %d", s);

  if (2 * c == (h - s)) --c;    /* avoid zero division */

  s += c;                       /* absorb scaled contrast */
  h -= c;                       /* absorb scaled contrast */
  h -= s;                       /* absorb shifted shadow */

  log_data ("h' = %d", h);
  log_data ("s' = %d", s);

  max = (1 << depth) - 1;
  log_data ("max = %zd", max);

  for (i = 0; i < (1 << depth); ++i)
    {
      int32_t value = i;

      value -= s;
      value *= max;
      value /= h;
      value += b;

      if (16 == depth)
        {
          uint16_t *p  = (uint16_t *) lut;

          p[i] = clamp (value, 0, max);
        }
      else
        {
          lut[i] = clamp (value, 0, max);
        }
    }
  return m;
}

LUT *
dip_iscan_BC_LUT (const void *self, int depth,
                  double brightness, double contrast)
{
  return dip_iscan_BCHS_LUT (self, depth, brightness, contrast, 0, 0);
}

/*! \brief  Creates a LUT for brightness/contrast manipulations
 *
 *  Algorithm indirectly taken from the GIMP.
 *
 *  \sa http://en.wikipedia.org/wiki/Image_editing#Contrast_change_and_brightening
 */
LUT *
dip_gimp_BC_LUT (const void *self, int depth,
                 double brightness, double contrast)
{
  SANE_Byte *lut;
  LUT       *m;

  size_t i;
  double max;

  require (dip == self);
  require (-1 <= brightness && brightness <= 1);
  require (-1 <= contrast   && contrast   <= 1);
  require (8 == depth || 16 == depth);

  lut = t_malloc ((1 << depth) * (depth / 8), SANE_Byte);
  m   = t_malloc (1, LUT);

  if (!lut || !m)
    {
      delete (lut);
      delete (m);
      return m;
    }

  m->lut   = lut;
  m->depth = depth;

  max = (1 << depth) - 1;

  for (i = 0; i < (1 << depth); ++i)
    {
      double value = i / max;

      if (brightness < 0.0)
        value *= 1.0 + brightness;
      else
        value += (1 - value) * brightness;

      value = (value - 0.5) * tan ((contrast + 1) * M_PI / 4) + 0.5;
      value *= max;

      if (16 == depth)
        {
          uint16_t *p  = (uint16_t *) lut;

          p[i] = clamp (value, 0, max);
        }
      else
        {
          lut[i] = clamp (value, 0, max);
        }
    }

  return m;
}

void
dip_flip_bits (const void *self, const buffer *buf)
{
  SANE_Byte *p;

  require (dip == self && buf);

  p = buf->ptr;
  while (p != buf->end)
    {
      *p = ~*p;
      ++p;
    }
}

static
void
dip_change_GRB_to_RGB_16 (const void *self, const buffer *buf)
{
  SANE_Byte *p, tmp;

  require (dip == self && buf && 16 == buf->ctx.depth);

  p = buf->ptr;
  while (p < buf->end)
    {
      tmp  = p[0];              /* most significant byte */
      p[0] = p[2];
      p[2] = p[0];
      tmp  = p[1];              /* least significant byte */
      p[1] = p[3];
      p[3] = tmp;

      p += 6;
    }
}

static
void
dip_change_GRB_to_RGB_8 (const void *self, const buffer *buf)
{
  SANE_Byte *p, tmp;

  require (dip == self && buf && 8 == buf->ctx.depth);

  p = buf->ptr;
  while (p < buf->end)
    {
      tmp  = p[0];
      p[0] = p[1];
      p[1] = tmp;

      p += 3;
    }
}

void
dip_change_GRB_to_RGB (const void *self, const buffer *buf)
{
  require (dip == self && buf);

  if (SANE_FRAME_RGB != buf->ctx.format)
    return;

  /**/ if (16 == buf->ctx.depth)
    return dip_change_GRB_to_RGB_16 (self, buf);
  else if ( 8 == buf->ctx.depth)
    return dip_change_GRB_to_RGB_8 (self, buf);

  err_major ("unsupported bit depth");
  return;
}

/*! \todo Add support for 16 bit color values (#816).
 */
void
dip_apply_color_profile (const void *self, const buffer *buf,
                         const double profile[9])
{
  SANE_Int i;
  SANE_Byte *r_buf, *g_buf, *b_buf;
  double red, grn, blu;

  SANE_Byte *data;
  SANE_Int size;

  require (dip == self && buf && profile);
  require (8 == buf->ctx.depth);

  if (SANE_FRAME_RGB != buf->ctx.format)
    return;

  data = buf->ptr;
  size = buf->end - buf->ptr;

  for (i = 0; i < size / 3; i++)
  {
    r_buf = data;
    g_buf = data + 1;
    b_buf = data + 2;

    red =
      profile[0] * (*r_buf) + profile[1] * (*g_buf) + profile[2] * (*b_buf);
    grn =
      profile[3] * (*r_buf) + profile[4] * (*g_buf) + profile[5] * (*b_buf);
    blu =
      profile[6] * (*r_buf) + profile[7] * (*g_buf) + profile[8] * (*b_buf);

    *data++ = clamp (red, 0, 255);
    *data++ = clamp (grn, 0, 255);
    *data++ = clamp (blu, 0, 255);
  }
}

bool
dip_has_deskew (const void *self, const device *hw)
{
  require (dip == self);

  return (magic_turn == dip->deskew
          || (esdip_turn == dip->deskew
              && enable_dip_deskew (hw)));
}

static
void
esdip_turn (buffer *buf, int res_x, int res_y)
{
  ipc_dip_parms p;

  require (dip->plugin);

  memset (&p, 0, sizeof (p));
  memcpy (&p.parms, &buf->ctx, sizeof (p.parms));
  p.res_x = res_x;
  p.res_y = res_y;

  ipc_dip_proc (dip->plugin, TYPE_DIP_SKEW_FLAG, &p,
                &buf->ctx, (void **) &buf->buf);

  buf->cap  = buf->ctx.bytes_per_line * buf->ctx.lines;
  buf->ptr  = buf->buf;
  buf->end  = buf->ptr;
  buf->end += buf->ctx.bytes_per_line * buf->ctx.lines;
}

static
void
magic_turn (buffer *buf, int res_x, int res_y)
{
  SANE_Status status;
  int center_x, center_y;
  double angle;
  const int bg_sample = 0xff;   /* white */

  require (buf);

  status = sanei_magic_findSkew (&buf->ctx, buf->buf, res_x, res_y,
                                 &center_x, &center_y, &angle);
  if (SANE_STATUS_GOOD == status)
    {
      status = sanei_magic_rotate (&buf->ctx, buf->buf,
                                   center_x, center_y, -angle, bg_sample);
    }

  buf->ptr  = buf->buf;
  buf->end  = buf->ptr;
  buf->end += buf->ctx.bytes_per_line * buf->ctx.lines;
}

void
dip_deskew (const void *self, const device *hw, unsigned int count,
            buffer *buf, const Option_Value *val)
{
  require (dip == self && buf && val);

  dip->deskew (buf, val[OPT_X_RESOLUTION].w, val[OPT_Y_RESOLUTION].w);
}

/*! \bug autocrop_max_y() is the \e wrong criterion to limit autocrop
 *       support to selected devices.
 */
bool
dip_has_autocrop (const void *self, const device *hw)
{
  require (dip == self);

  return (magic_crop == dip->autocrop
          || (esdip_crop == dip->autocrop
              && 0 != autocrop_max_y (hw)));
}

static
void
esdip_crop (buffer *buf, const device *hw, unsigned int count,
            const Option_Value *val)
{
  ipc_dip_parms p;

  require (dip->plugin && hw && hw->fw_name && val);

  memset (&p, 0, sizeof (p));
  memcpy (&p.parms, &buf->ctx, sizeof (p.parms));
  p.res_x = val[OPT_X_RESOLUTION].w;
  p.res_y = val[OPT_Y_RESOLUTION].w;
  p.gamma = hw->gamma_type[ val[OPT_GAMMA_CORRECTION].w ];
  p.bside = SANE_FALSE;
  if (using (hw, adf) && val[OPT_ADF_MODE].w)
    {
      p.bside = (0 == count % 2);
    }
  strncpy (p.fw_name, hw->fw_name, num_of (p.fw_name));

  ipc_dip_proc (dip->plugin, TYPE_DIP_CROP_FLAG, &p,
                &buf->ctx, (void **) &buf->buf);

  buf->cap  = buf->ctx.bytes_per_line * buf->ctx.lines;
  buf->ptr  = buf->buf;
  buf->end  = buf->ptr;
  buf->end += buf->ctx.bytes_per_line * buf->ctx.lines;
}

static
void
magic_crop (buffer *buf, int res_x, int res_y)
{
  SANE_Status status;
  int top, left, bottom, right;

  require (buf);

  status = sanei_magic_findEdges (&buf->ctx, buf->buf, res_x, res_y,
                                  &top, &bottom, &left, &right);
  if (SANE_STATUS_GOOD == status)
    {
      status = sanei_magic_crop (&buf->ctx, buf->buf,
                                 top, bottom, left, right);
    }

  buf->ptr  = buf->buf;
  buf->end  = buf->ptr;
  buf->end += buf->ctx.bytes_per_line * buf->ctx.lines;
}

void
dip_autocrop (const void *self, const device *hw, unsigned int count,
              buffer *buf, const Option_Value *val)
{
  require (dip == self && buf && val);

  /**/ if (esdip_crop == dip->autocrop)
    {
      esdip_crop (buf, hw, count, val);
    }
  else if (magic_crop == dip->autocrop)
    {
      int res_x = val[OPT_X_RESOLUTION].w;
      int res_y = val[OPT_Y_RESOLUTION].w;

      magic_crop (buf, res_x, res_y);
    }
}
