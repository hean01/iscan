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

#ifndef ___PISA_CONFIGURATION_H
#define ___PISA_CONFIGURATION_H

#include <gtk/gtk.h>
#include <pisa_enums.h>

class config_window
{
 public:

  // operation
  int	init ( void );
  GtkWidget *	create_window ( GtkWidget * parent );
  int	close_window ( int destroy );

  void	update_filename ( void );

  int		m_ok;
  char		m_cmd [ 256 ];

 private:

  // operation
  GtkWidget *	create_pips_path ( void );
  GtkWidget *	create_action_area ( void );

  // attribute
  GtkWidget *	m_widget [ WG_CONFIG_NUM ];
 
};

#endif // ___PISA_CONFIGURATION_H

