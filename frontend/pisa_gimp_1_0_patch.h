/* 
   SANE EPSON backend
   Copyright (C) 2001 SEIKO EPSON CORPORATION

   Date         Author      Reason
   06/01/2001   N.Sasaki    New

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

#ifndef ___PISA_GIMP_1_0_PATCH_H
#define ___PISA_GIMP_1_0_PATCH_H

#define GimpPlugInInfo GPlugInInfo
#define GimpParam GParam
#define GimpParamDef GParamDef
#define GimpDrawable GDrawable
#define GimpPixelRgn GPixelRgn
#define GimpRunModeType GRunModeType
#define GimpImageType GImageType

#define GIMP_PDB_INT32 PARAM_INT32
#define GIMP_PDB_STATUS PARAM_STATUS
#define GIMP_PDB_CALLING_ERROR STATUS_CALLING_ERROR
#define GIMP_PDB_SUCCESS STATUS_SUCCESS
#define GIMP_RUN_INTERACTIVE RUN_INTERACTIVE
#define GIMP_RUN_NONINTERACTIVE RUN_NONINTERACTIVE
#define GIMP_RUN_WITH_LAST_VALS RUN_WITH_LAST_VALS
#define GIMP_EXTENSION PROC_EXTENSION
#define GIMP_RGB RGB
#define GIMP_RGB_IMAGE RGB_IMAGE
#define GIMP_GRAY GRAY
#define GIMP_GRAY_IMAGE GRAY_IMAGE
#define GIMP_RGBA_IMAGE RGBA_IMAGE
#define GIMP_NORMAL_MODE NORMAL_MODE

#endif // ___PISA_GIMP_1_0_PATCH_H

