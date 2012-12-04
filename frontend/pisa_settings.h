/* 
   SANE EPSON backend
   Copyright (C) 2001, 2008 SEIKO EPSON CORPORATION

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

#ifndef ___PISA_SETTINGS_H
#define ___PISA_SETTINGS_H

#include "pisa_structs.h"
#include "pisa_marquee.h"

class settings
{
 public:

  // attribute
  char			destination;

  imagetype		imgtype;
  long			resolution;
  bool			enable_start_button;
  bool			enable_draft_mode;
  long			usm;

  double		max_area [ 2 ];
  double		coef [ 9 ];

  long			unit;

  // operation
  void	init ( marquee ** marq );

  long	get_marquee_size (void) const;

  const marquee& get_marquee (long index = -1) const;
	marquee& get_marquee (long index = -1);

  void	add_marquee ( marquee ** marq );
  void	del_marquee (long index = -1);
  void	delete_all ( void );

 private:
  
  // attribute
  marquee_node	* top;

};

#endif // ___PISA_SETTINGS_H

