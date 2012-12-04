/* 
   SANE EPSON backend
   Copyright (C) 2001, 2005, 2008  SEIKO EPSON CORPORATION

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

#ifndef ___EPS_LIVE_PREVIEW_H
#define ___EPS_LIVE_PREVIEW_H

#include "pisa_structs.h"
#include "pisa_enums.h"

/*------------------------------------------------------------*/
/* FIXME: calc_*() only used by scan_manager --> make member function
 */
int calc_rowbytes ( int width, pisa_pixel_type type );
int calc_bitperpix ( pisa_pixel_type pixel, pisa_bitdepth_type bitdepth );

int tool_lut ( pisa_image_info * info,
	       const gamma_struct * lut );
int tool_matrix ( pisa_image_info * info,
		  int saturation,
		  const double * color_profile );

int generate_color_coef ( double * dst,
			  const double * src,
			  int saturation );

int build_gray ( pisa_image_info * info,
		 unsigned char do_clr );

int build_bw ( pisa_image_info * info,
	       unsigned char do_clr,
	       unsigned char halftone,
	       long threshold );

#endif // ___EPS_LIVE_PREVIEW_H


