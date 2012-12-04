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

#ifndef ___PISA_GAMMA_CORRECTION_H
#define ___PISA_GAMMA_CORRECTION_H

#include <gtk/gtk.h>
#include "pisa_enums.h"

#define GAMMA_WIDTH	192
#define GAMMA_HEIGHT	192

class gamma_correction
{
 public:

  // operation
  int	init ( void );
  GtkWidget *	create_controls ( GtkWidget * parent );
  int	close_window ( int destroy );
  void	sensitive ( int is_prev_img );

  void	change_page ( int page );
  void	reset ( int all );
  gint	event ( GtkWidget * widget, GdkEvent * event );

 private:

  typedef double matrix [ 4 ] [ 4 ];

  // operation
  GtkWidget	* create_gamma_notebook ( GtkWidget * parent );
  GtkWidget	* create_gamma_curve ( int i );
  GtkWidget	* create_reset_button ( void );

  void	reset_points ( int i );
  void	update_gamma ( void );
  void	calculate_curve ( void );
  void	plot_curve ( int p1, int p2, int p3, int p4 );
  void	compose_curve ( matrix a, matrix b, matrix ab );

  // attribute
  int		m_page;
  GtkWidget	* m_widget [ WG_GAMMA_NUM ];
  GtkWidget	* m_drawarea [ 4 ];
  GdkPixmap	* m_pixmap [ 4 ];

  int		m_leftmost, m_rightmost;
  int		m_points [ 4 ] [ 17 ] [ 2 ];
  unsigned char	m_curve[ 4 ] [ GAMMA_WIDTH ];
  int		m_grabpoint;
  matrix	m_basis;
  int		m_active [ 4 ];

  GdkCursor	* m_cursor [ 3 ];
};

#endif // ___PISA_GAMMA_CORRECTION_H
