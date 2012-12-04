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

#include "pisa_change_unit.h"

const double CM_PER_INCH = 2.54;	// exact, per definition


double
inches_reflect_zoom (double inch, long zoom)
{
  return inch * zoom / 100;
}

double
inch2centi (double inch, long zoom)
{
  return (inch * CM_PER_INCH * zoom) / 100;
}


long
inch2width (double inch, long resolution, long zoom, bool monochrome)
{
  int boundary = (monochrome ? 32 : 8);

  return (inch2pixel (inch, resolution, zoom) / boundary) * boundary;
}

long
inch2height (double inch, long resolution, long zoom)
{
  return inch2pixel (inch, resolution, zoom);
}

long
inch2pixel (double inch, long resolution, long zoom)
{
  return (long) ((inch * resolution * zoom) / 100);
}

long
inch2pixel (bool is_width,
	    double inch, long resolution, long zoom)
{
  return (is_width
	  ? inch2width  (inch, resolution, zoom)
	  : inch2height (inch, resolution, zoom));
}


long
centi2width (double centi, long resolution, long zoom)
{
  return (centi2pixel (centi, resolution, zoom) / 8) * 8;
}

long
centi2height (double centi, long resolution, long zoom)
{
  return centi2pixel (centi, resolution, zoom);
}

long
centi2pixel (double centi, long resolution, long zoom)
{
  return (long) ((centi * resolution * zoom) / (100 * CM_PER_INCH));
}

long
centi2pixel (bool is_width,
	     double centi, long resolution, long zoom)
{
  return (is_width
	  ? centi2width  (centi, resolution, zoom)
	  : centi2height (centi, resolution, zoom));
}
