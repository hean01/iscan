/* 
   SANE EPSON backend
   Copyright (C) 2001, 2008, 2009  SEIKO EPSON CORPORATION

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

#ifndef ___PISA_MAIN_WINDOW_H
#define ___PISA_MAIN_WINDOW_H

#include <gtk/gtk.h>
#include "pisa_enums.h"
#include "pisa_scan_manager.h"

class main_window
{
 public:

  int	init ( void );
  GtkWidget *	create_window ( GtkWidget * parent = 0 );
  int	close_window ( int destroy );
  void  start_scan (const GtkWidget *destination) const;
  void  start_preview ( bool zooming );
  void  set_double_feed_detection (const GtkWidget *widget);
  void	sensitive ( int is_prev_img );
  void	sensitive_option ( );
  void  enable_start_button (bool yes);
  void  update_zoom_button ();
  void  do_sensitive_size_check (bool yes);

  GtkWidget *	get_widget (int wg_id = WG_MAIN_TOP) { return m_widget[wg_id]; }

private:

  void create_controls ();
  GtkWidget *tool_button (const char *text, GCallback callback, char **icon);
  GtkWidget *check_button (const char *text,
                           void (*callback) (GtkWidget*, gpointer));
  GtkWidget *close_button ();

  GtkWidget *action_button_box ();
  GtkWidget *toolbar ();
  GtkWidget *close_button_box ();

  GtkWidget *	create_left_area ( void );
  GtkWidget *	create_scan_button ( void );
  GtkWidget *	create_scan_label ( void );
  GtkWidget *	create_right_area ( void );
  GtkWidget *	create_notebook ( void );
  GtkWidget *	create_document_tab ( void );
  GtkWidget *	create_target (const scan_manager *sm);
  GtkWidget *	create_focus ( void );
  GtkWidget *	create_options ( void );

  void add_maintenance_ui (GtkWidget *vbox);

  void	sensitive_image_type ( void );
  void	sensitive_target ( void );
  void	sensitive_resolution ( void );
  void	sensitive_scale ( void );
  void	sensitive_focus ( void );
  void	sensitive_usm ( void );
  void	sensitive_size_check (void);
  void	sensitive_deskew (void);

  GtkWidget *prev_btn_;
  GtkWidget *zoom_btn_;
  GtkWidget *expo_btn_;

  GtkWidget *scan_btn_;
  GtkWidget *print_btn_;
  GtkWidget *dest_menu_;

  GtkWidget *scan_navi_;
  GtkWidget *draft_mode_;
  GtkWidget *size_check_;
  GtkWidget *deskew_;

  GtkWidget *config_;

  GtkWidget *unsharp_mask_;

  GtkWidget	* m_widget [ WG_MAIN_NUM ];
  bool          _done_preview;
  bool          _do_sensitive_size_check;
  bool          _compact_mode;
};

#endif // ___PISA_MAIN_WINDOW_H
