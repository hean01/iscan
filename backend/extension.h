/*  extension.h -- types to handle the various scanner document sources
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


#ifndef included_source_h
#define included_source_h

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


/*! Base "class" for hardware extensions.

  This "class" provides the common part of the other extension types.

  \sa struct _fbf_extension, fbf_extension
  \sa struct _adf_extension, adf_extension
  \sa struct _tpu_extension, tpu_extension
 */
struct _extension
{
  SANE_Byte status;

  SANE_Range x_range;		/* in mm */
  SANE_Range y_range;

  SANE_Int max_x;		/* in pixels */
  SANE_Int max_y;

  double doc_x;			/* in mm */
  double doc_y;
  SANE_Bool has_size_check;
};
typedef struct _extension extension;


/*! Flatbed hardware "extension".

  \sa struct _extension, extension
 */
struct _fbf_extension
{
  SANE_Byte status;

  SANE_Range x_range;		/* in mm */
  SANE_Range y_range;

  SANE_Int max_x;		/* in pixels */
  SANE_Int max_y;

  double doc_x;			/* in mm */
  double doc_y;
  SANE_Bool has_size_check;
};
typedef struct _fbf_extension fbf_extension;


/*! Auto Document Feeder (ADF) hardware extension.

  \sa struct _extension, extension
 */
struct _adf_extension
{
  SANE_Byte status;

  SANE_Range x_range;		/* in mm */
  SANE_Range y_range;

  SANE_Int max_x;		/* in pixels */
  SANE_Int max_y;

  double doc_x;			/* in mm */
  double doc_y;
  SANE_Bool has_size_check;

  SANE_Byte ext_status;

  unsigned int sheet_count;
  SANE_Bool using_duplex;

  SANE_Bool auto_eject;
};
typedef struct _adf_extension adf_extension;


/*! TransParency Unit (TPU) hardware extension.

  \sa struct _extension, extension
 */
struct _tpu_extension
{
  SANE_Byte status;

  SANE_Range x_range;		/* in mm */
  SANE_Range y_range;

  SANE_Int max_x;		/* in pixels */
  SANE_Int max_y;

  double doc_x;			/* in mm */
  double doc_y;
  SANE_Bool has_size_check;

  SANE_Bool has_focus;
  SANE_Bool use_focus;
};
typedef struct _tpu_extension tpu_extension;


/*! Convenience macro to test whether \a ext is currently being used.
 */
#define using(hw,ext) \
  ((hw) && ((hw)->src == (const extension *) (hw)->ext))


#endif  /* !defined (included_source_h) */
