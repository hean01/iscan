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

#ifndef ___PISA_MARQUEE_H
#define ___PISA_MARQUEE_H

#include "pisa_structs.h"

/*--------------------------------------------------------------*/
class marquee
{
 public:

  long			active;

  _pointD		offset;
  _pointD		area;

  long			scale;

  long			gamma;
  long			highlight;
  long			shadow;
  long			threshold;
  long			brightness;
  long			contrast;

  unsigned char		gamma_table [ 4 ] [ 256 ]; // mono,red,green,blue

  long			graybalance;
  long			saturation;

  long			focus;

  double		film_gamma [ 3 ];
  double		film_yp [ 3 ];
  double		grayl [ 3 ];

  gamma_struct		lut;

  marquee (const double w = 8.5, const double h = 11.7,
           const long f = 0, const long b = 0, const long c = 0);

  marquee & operator= ( const marquee & x );
};


/*--------------------------------------------------------------*/
typedef struct _marquee_node
{
  _marquee_node		* next;
  marquee		* data;
} marquee_node;

#endif // ___PISA_MARQUEE_H

