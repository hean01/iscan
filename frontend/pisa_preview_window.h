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

#ifndef ___PISA_PREVIEW_WINDOW_H
#define ___PISA_PREVIEW_WINDOW_H

#include <gtk/gtk.h>
#include "pisa_enums.h"
#include "pisa_structs.h"

class preview_window
{
 public:

  preview_window ( ) { m_gc = NULL; }
  
  // operation
  int	init ( void );
  GtkWidget *	create_window ( GtkWidget * parent );
  int	close_window ( int destroy );

  int	is_prev_img ( void ) { return m_is_prev; }

  int	resize_window ( void );

  int	auto_exposure ( void );

  int	update_img ( bool left = false );

  void	start_preview (void);
  void start_zoom (void);

  gint	expose_event ( GtkWidget * widget, GdkEventExpose * event );
  gint	event ( GtkWidget * widget, GdkEvent * event );

  void	size_allocate ( GtkWidget * widget );

 private:

  // operation
  GtkWidget	* create_darea ( GtkWidget * parent );

  long	get_preview_resolution ( const _rectD * img_rect = 0 );
  void	resize_preview_window ( long width, long height );
  void  change_max_scan_area ( long width, long height );
  void	change_max_disp_area ( long width, long height );
  void	clear_image ( void );

  void modify_max_val ( void );

  void draw_marquee ( void );

  // preview, zoom
  void start_preview (bool zooming);
  void reset_settings (bool zooming);
  int set_preview_param (bool zooming);
  void revert_resolution ( void );

  void tool_usm (const pisa_image_info& info);
  void zoom_boundary (_pointD& pt_offset, _pointD& pt_area,
		      const _pointD& pt_max, float rate) const;

  // cursor
  void create_cursor ( void );
  int set_mouse_cursor ( int x, int y );
  int search_cursor_state ( const _pointL & pt_lt,
			    const _pointL & pt_rb,
			    const _pointL & pt );
  void change_cursor ( void );

  // marquee
  gint mouse_down ( GdkEvent * event );
  gint mouse_move ( GdkEvent * event );
  gint mouse_up   ( GdkEvent * event );  

  int create_marquee ( const _pointL & pt_lt, const _pointL & pt_rb );
  int delete_marquee ( void );

  void move_marquee ( const _pointL & pt_lt, const _pointL & pt_rb );
  void resize_marquee ( const _pointL & pt_lt, const _pointL & pt_rb );

  void begin_mouse_move ( _pointL * pt_lt, _pointL * pt_rb );
  void move_rect ( _pointL * pt_lt, _pointL * pt_rb, const _pointL & pt_move );

  _pointD clientpix2inches ( _pointL & pt );
  _pointL inches2clientpix ( _pointD & pt );
  int get_marquee_point ( long i, _pointL * pt_lt, _pointL * pt_rb );

  int check_min_size ( const _pointL & pt_lt, const _pointL & pt_rb );
  void check_max_size ( _pointD * pt_offset, _pointD * pt_area,
			int offset );
  void check_ltrb ( _pointL * pt_lt, _pointL * pt_rb );

  void draw_rect ( const _pointL & pt_lt, const _pointL & pt_rb );

  // attribute
  GtkWidget		* m_win;
  GtkWidget		* m_prev;

  long			m_is_prev;
  long			m_img_width, m_img_height;
  unsigned char		* m_img;
  unsigned char		* m_img_org;

  GdkGC			* m_gc;

  int			m_cursor_state;
  GdkCursor		* m_cursor [ 11 ];

  bool			m_on_preview;
  long			m_allocate_width;
  long			m_allocate_height;

  // for marquee
  int			m_drag;
  _rectL		m_client_rect;
  _rectD		m_max_img_rect;
  _rectD		m_img_rect;
  
  _pointL		m_pt_max_lt;
  _pointL		m_pt_max_rb;

  _pointL		m_pt_begin;
  _pointL		m_pt_old;
  _pointL		m_pt_old_lt;
  _pointL		m_pt_old_rb;
  _pointD		m_pt_save_offset;
  _pointD		m_pt_save_area;

};

inline void
preview_window::start_preview (void)
{
  start_preview (false);
}

inline void
preview_window::start_zoom (void)
{
  start_preview (true);
}

#endif // ___PISA_PREVIEW_WINDOW_H
