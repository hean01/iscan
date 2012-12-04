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

#ifndef ___PISA_DEFAULT_VAL_H
#define ___PISA_DEFAULT_VAL_H

#include "esmod-wrapper.hh"

#define PREFERENCE	".iscan_preference"

// gamma
#define DEFGAMMA	ISCAN_DEFAULT_GAMMA
#define MINGAMMA	0.5
#define MAXGAMMA	5.0
#define LINEGAMMA	0.17
#define PAGEGAMMA	0.34

// highlight
#define DEFHIGHLIGHT	ISCAN_DEFAULT_HILITE
#define MINHIGHLIGHT	61
#define MAXHIGHLIGHT	490
#define LINEHIGHLIGHT	15
#define PAGEHIGHLIGHT	30

// shadow
#define DEFSHADOW	ISCAN_DEFAULT_SHADOW
#define MINSHADOW	0
#define MAXSHADOW	60
#define LINESHADOW	1
#define PAGESHADOW	2

// threshold
#define DEFTHRESHOLD	ISCAN_DEFAULT_THRESHOLD
#define MINTHRESHOLD	0
#define MAXTHRESHOLD	255
#define LINETHRESHOLD	1
#define PAGETHRESHOLD	5

// gray balance
#define DEFGRAYBALANCE	0
#define MINGRAYBALANCE	0
#define MAXGRAYBALANCE	100
#define LINEGRAYBALANCE	5
#define PAGEGRAYBALANCE	25

// saturation
#define DEFSATURATION	0
#define MINSATURATION	-100
#define MAXSATURATION	100
#define LINESATURATION	5
#define PAGESATURATION	25

// window position
#define POS_MAIN_X	20
#define POS_MAIN_Y	20
#define POS_CONFIG_X	100
#define POS_CONFIG_Y	100
#define POS_PRINT_X	100
#define POS_PRINT_Y	100

#endif // ___PISA_DEFAULT_VAL_H

