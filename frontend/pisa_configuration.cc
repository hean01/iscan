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

#include <config.h>

#include "gettext.h"
#define  _(msg_id)	gettext (msg_id)

/*------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>

/*------------------------------------------------------------*/
#include "pisa_configuration.h"
#include "pisa_view_manager.h"
#include "pisa_error.h"
#include "pisa_default_val.h"

/*------------------------------------------------------------*/
static gint delete_event ( GtkWidget * widget, gpointer data )
{
  widget = widget;
  data = data;

  ::g_view_manager->close_window ( ID_WINDOW_CONFIG, 1 );

  return TRUE;
}

/*------------------------------------------------------------*/
static void click_ok ( GtkWidget * widget, gpointer data )
{
  config_window * cf_cls;

  widget = widget;
 
  cf_cls = ( config_window * ) data;

  cf_cls->m_ok = 1;
  
  cf_cls->update_filename ( );

  ::g_view_manager->close_window ( ID_WINDOW_CONFIG, 1 );
}

/*------------------------------------------------------------*/
static void click_cancel ( GtkWidget * widget, gpointer data )
{
  config_window * cf_cls;

  widget = widget;
 
  cf_cls = ( config_window * ) data;

  cf_cls->m_ok = 0;

  ::g_view_manager->close_window ( ID_WINDOW_CONFIG, 1 );
}


/*------------------------------------------------------------*/
int config_window::init ( void )
{
  int i;

  for ( i = 0; i < WG_CONFIG_NUM; i++ )
    m_widget [ i ] = 0;

  m_ok = 0;
  ::strcpy ( m_cmd, "" );

  return PISA_ERR_SUCCESS;
}

/*------------------------------------------------------------*/
GtkWidget * config_window::create_window ( GtkWidget * parent )
{
  GtkWidget	* top;
  GtkWidget	* widget;

  top = ::gtk_dialog_new ( );
  ::gtk_window_set_policy ( GTK_WINDOW ( top ),
			    FALSE, FALSE, TRUE );
  ::gtk_container_border_width ( GTK_CONTAINER ( GTK_DIALOG ( top )->vbox ),
				 5 );
  ::gtk_window_set_title ( GTK_WINDOW ( top ), _( "Configuration" ) );
  ::gtk_widget_set_uposition ( top, POS_CONFIG_X, POS_CONFIG_Y );
  ::gtk_widget_realize ( top );

  ::gtk_signal_connect ( GTK_OBJECT ( top ), "delete_event",
			 GTK_SIGNAL_FUNC ( ::delete_event ), 0 );

  m_widget [ WG_CONFIG_TOP ] = top;

  widget = create_pips_path ( );
  ::gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( top )->vbox ),
			 widget, FALSE, FALSE, 5 );
  ::gtk_widget_show ( widget );

  create_action_area ( );

  ::gtk_window_set_modal ( GTK_WINDOW ( top ), TRUE );
  ::gtk_window_set_transient_for ( GTK_WINDOW ( top ), GTK_WINDOW ( parent ) );
  ::gtk_widget_show ( top );

  return top;
}

/*------------------------------------------------------------*/
int config_window::close_window ( int destroy )
{
  if ( destroy && m_widget [ WG_CONFIG_TOP ] )
    ::gtk_widget_destroy ( m_widget [ WG_CONFIG_TOP ] );

  m_widget [ WG_CONFIG_TOP ] = 0;

  return PISA_ERR_SUCCESS;
}

/*------------------------------------------------------------*/
void config_window::update_filename ( void )
{
  ::strcpy ( m_cmd,
	     ::gtk_entry_get_text ( GTK_ENTRY ( m_widget [ WG_CONFIG_PATH ] ) ) );
}

/*------------------------------------------------------------*/
GtkWidget * config_window::create_pips_path ( void )
{
  GtkWidget	* frame;
  GtkWidget	* vbox;
  GtkWidget	* hbox;
  GtkWidget	* comment;
  GtkWidget	* edit;

  frame = ::gtk_frame_new ( _( "Print Command" ) );
  ::gtk_container_border_width ( GTK_CONTAINER ( frame ), 5 );
  
  vbox = ::gtk_vbox_new ( FALSE, 5 );
  ::gtk_container_border_width ( GTK_CONTAINER ( vbox ), 5 );
  ::gtk_container_add ( GTK_CONTAINER ( frame ), vbox );
  ::gtk_widget_show ( vbox );

  hbox = ::gtk_hbox_new ( FALSE, 5 );
  ::gtk_container_border_width ( GTK_CONTAINER ( hbox ), 5 );
  ::gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 5 );
  ::gtk_widget_show ( hbox );

  edit = ::gtk_entry_new ( );
  ::gtk_widget_set_usize ( edit, 250, 0 );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), edit, FALSE, FALSE, 5 );
  ::gtk_widget_show ( edit );

  ::gtk_entry_set_max_length ( GTK_ENTRY ( edit ), 255 );
  ::gtk_entry_set_text ( GTK_ENTRY ( edit ), m_cmd );

  m_widget [ WG_CONFIG_PATH ] = edit;

  comment = ::gtk_label_new (_("In order to print, your print system "
			       "must be able to handle the PNG file "
			       "format directly.  CUPS or Photo Image "
			       "Print System (versions 1.3.1 or later) "
			       "do this by default."));
  ::gtk_label_set_line_wrap ( GTK_LABEL ( comment ), TRUE );
  ::gtk_box_pack_start ( GTK_BOX ( vbox ), comment, FALSE, FALSE, 0 );
  ::gtk_widget_show ( comment );

  return frame;
}

/*------------------------------------------------------------*/
GtkWidget * config_window::create_action_area ( void )
{
  GtkWidget	* button;

  // OK
  button = ::gtk_button_new_with_label ( _( "  OK  " ) );
  GTK_WIDGET_SET_FLAGS ( button, GTK_CAN_DEFAULT );
  ::gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( m_widget [ WG_CONFIG_TOP ] )->action_area ),
			 button, FALSE, FALSE, 0 );
  ::gtk_signal_connect ( GTK_OBJECT ( button ), "clicked",
			 GTK_SIGNAL_FUNC ( click_ok ), this );
  ::gtk_widget_grab_default ( button );
  ::gtk_widget_show ( button );

  // cancel
  button = ::gtk_button_new_with_label ( _( "  Cancel  " ) );
  ::gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( m_widget [ WG_CONFIG_TOP ] )->action_area ),
			 button, FALSE, FALSE, 0 );
  ::gtk_signal_connect ( GTK_OBJECT ( button ), "clicked",
			 GTK_SIGNAL_FUNC ( click_cancel ), this );
  ::gtk_widget_show ( button );
  
  return 0;
}

