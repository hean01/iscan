/* 
   SANE EPSON backend
   Copyright (C) 2001, 2005 SEIKO EPSON CORPORATION

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

#ifndef ___PISA_CHANGE_UNIT_H
#define ___PISA_CHANGE_UNIT_H

double inches_reflect_zoom ( double inch, long zoom );
double inch2centi ( double inch, long zoom = 100 );

// Convenience API to convert dimensions to pixels the same way as the
// epkowa backend does.

long inch2width  (double inch, long resolution, long zoom = 100,
		  bool monochrome = false);
long inch2height (double inch, long resolution, long zoom = 100);
long inch2pixel  (double inch, long resolution, long zoom = 100);
long inch2pixel  (bool is_width,
		  double inch, long resolution, long zoom = 100);

long centi2width  (double centi, long resolution, long zoom = 100);
long centi2height (double centi, long resolution, long zoom = 100);
long centi2pixel  (double centi, long resolution, long zoom = 100);
long centi2pixel  (bool is_width,
		   double centi, long resolution, long zoom = 100);

#endif // ___PISA_CHANGE_UNIT_H
