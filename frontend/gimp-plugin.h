/*
   SANE EPSON backend
   Copyright (C) 2005 SEIKO EPSON CORPORATION

   This file is part of the `iscan' program.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   As a special exception, the copyright holders give permission
   to link the code of this program with the esmod library and
   distribute linked combinations including the two.  You must obey
   the GNU General Public License in all respects for all of the
   code used other then esmod.
*/
#ifndef GIMP_PLUGIN_H
#define GIMP_PLUGIN_H

#ifdef HAVE_GIMP_2
/* Define these structures for using gimp-devel-2 */

struct _GimpParamColor_1
{
  guint8 red;
  guint8 green;
  guint8 blue;
};
typedef struct _GimpParamColor_1 GimpParamColor_1 ;

union _GimpParamData_1
{
  gint32            d_int32;
  gint16            d_int16;
  gint8             d_int8;
  gdouble           d_float;
  gchar            *d_string;
  gint32           *d_int32array;
  gint16           *d_int16array;
  gint8            *d_int8array;
  gdouble          *d_floatarray;
  gchar           **d_stringarray;  
  GimpParamColor_1  d_color;
  GimpParamRegion   d_region;
  gint32            d_display;
  gint32            d_image;
  gint32            d_layer;
  gint32            d_layer_mask;
  gint32            d_channel;
  gint32            d_drawable;
  gint32            d_selection;
  gint32            d_boundary;
  gint32            d_path;
  gint32            d_unit;
  GimpParasite      d_parasite;
  gint32            d_tattoo;
  GimpPDBStatusType d_status;
};
typedef union _GimpParamData_1 GimpParamData_1 ;

struct _GimpParam_1
{
  GimpPDBArgType type;
  GimpParamData_1  data;
};
typedef struct _GimpParam_1 GimpParam_1;

typedef void (* GimpInitProc_1)  (void);
typedef void (* GimpQuitProc_1)  (void);
typedef void (* GimpQueryProc_1) (void);
typedef void (* GimpRunProc_1)   (gchar            *name,
				  gint              n_params,
				  GimpParam_1      *param,
				  gint             *n_return_vals,
				  GimpParam_1     **return_vals);

struct _GimpPlugInInfo_1
{
  /* called when the gimp application initially starts up */
  GimpInitProc_1  init_proc;

  /* called when the gimp application exits */
  GimpQuitProc_1  quit_proc;

  /* called by the gimp so that the plug-in can inform the
   * gimp of what it does. (ie. installing a procedure database
   * procedure).
   */
  GimpQueryProc_1 query_proc;

  /* called to run a procedure the plug-in installed in the
   * procedure database.
   */
  GimpRunProc_1   run_proc;
};
typedef struct _GimpPlugInInfo_1 GimpPlugInInfo_1;

#else
/* Define these structures for using gimp-devel-1 */

struct _GimpRGB_2
{
  gdouble r, g, b, a;
};
typedef struct _GimpRGB_2 GimpRGB_2;

union _GimpParamData_2
{
  gint32            d_int32;
  gint16            d_int16;
  gint8             d_int8;
  gdouble           d_float;
  gchar            *d_string;
  gint32           *d_int32array;
  gint16           *d_int16array;
  gint8            *d_int8array;
  gdouble          *d_floatarray;
  gchar           **d_stringarray;
  GimpRGB_2         d_color;
  GimpParamRegion   d_region;
  gint32            d_display;
  gint32            d_image;
  gint32            d_layer;
  gint32            d_layer_mask;
  gint32            d_channel;
  gint32            d_drawable;
  gint32            d_selection;
  gint32            d_boundary;
  gint32            d_path;
  gint32            d_unit;
  GimpParasite      d_parasite;
  gint32            d_tattoo;
  GimpPDBStatusType d_status;
};
typedef union _GimpParamData_2 GimpParamData_2 ;

struct _GimpParam_2
{
  GimpPDBArgType  type;
  GimpParamData_2 data;
};
typedef struct _GimpParam_2 GimpParam_2;

typedef void (* GimpInitProc_2)  (void);
typedef void (* GimpQuitProc_2)  (void);
typedef void (* GimpQueryProc_2) (void);
typedef void (* GimpRunProc_2)   (const gchar       *name,
				  gint               n_params,
				  const GimpParam_2 *param,
				  gint              *n_return_vals,
				  GimpParam_2      **return_vals);

struct _GimpPlugInInfo_2
{
  /* called when the gimp application initially starts up */
  GimpInitProc_2  init_proc;

  /* called when the gimp application exits */
  GimpQuitProc_2  quit_proc;

  /* called by the gimp so that the plug-in can inform the
   * gimp of what it does. (ie. installing a procedure database
   * procedure).
   */
  GimpQueryProc_2 query_proc;

  /* called to run a procedure the plug-in installed in the
   * procedure database.
   */
  GimpRunProc_2   run_proc;
};
typedef struct _GimpPlugInInfo_2 GimpPlugInInfo_2;

#endif // HAVE_GIMP_2

#endif  /* GIMP_PLUGIN_H */
