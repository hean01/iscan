/*  dip-obj.h -- digital image processing functionality singleton
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


#ifndef dip_obj_h_included
#define dip_obj_h_included

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sane/sane.h>

#include "device.h"
#include "epkowa.h"

#ifdef __cplusplus
extern "C"
{
#endif

  void * dip_init (const char *pkglibdir, SANE_Status *status);
  void * dip_exit (void *self);

  bool dip_needs_whole_image (const void *self, const Option_Value *val,
                              const SANE_Option_Descriptor *opt);

  void dip_apply_LUT (const void *self, const buffer *buf, const LUT *m);
  void dip_apply_LUT_RGB (const void *self, const buffer *buf,
                          const LUT *r, const LUT *g, const LUT *b);
  LUT * dip_destroy_LUT (const void *self, LUT *m);
  LUT * dip_gamma_LUT (const void *self, int depth,
                       double gamma);
  LUT * dip_iscan_BCHS_LUT (const void *self, int depth,
                            double brightness, double contrast,
                            double highlight, double shadow);
  LUT * dip_iscan_BC_LUT (const void *self, int depth,
                          double brightness, double contrast);
  LUT * dip_gimp_BC_LUT (const void *self, int depth,
                         double brightness, double contrast);

  void dip_flip_bits (const void *self, const buffer *buf);
  void dip_change_GRB_to_RGB (const void *self, const buffer *buf);
  void dip_apply_color_profile (const void *self, const buffer *buf,
                                const double profile[9]);

  bool dip_has_deskew (const void *self, const device *hw);
  bool dip_has_autocrop (const void *self, const device *hw);

  void dip_deskew (const void *self, const device *hw, unsigned int count,
                   buffer *buf, const Option_Value *val);
  void dip_autocrop (const void *self, const device *hw, unsigned int count,
                     buffer *buf, const Option_Value *val);

#ifdef __cplusplus
}       /* extern "C" */
#endif

#endif  /* !defined (dip_obj_h_included) */
