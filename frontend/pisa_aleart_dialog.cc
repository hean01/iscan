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

#include <stdio.h>

/*------------------------------------------------------------*/
#include "pisa_aleart_dialog.h"

/*------------------------------------------------------------*/
static gint delete_event ( GtkWidget * widget, gpointer data )
{
  widget = widget;
  data = data;

  ::gtk_main_quit ( );

  return TRUE;
}

/*------------------------------------------------------------*/
void set_value ( GtkWidget * widget, gpointer data )
{
  aleart_dialog	* dlg= ( aleart_dialog * ) data;

  dlg->m_id = GPOINTER_TO_INT ( ::gtk_object_get_data ( GTK_OBJECT ( widget ),
							 "value" ) );

  ::gtk_main_quit ( );
}

/*------------------------------------------------------------*/
int aleart_dialog::message_box ( GtkWidget * parent,
				 const char * msg,
				 const char * btn1 ,
				 const char * btn2 )
{
  GtkWidget	* top;
  GtkWidget	* vbox;
  GtkWidget	* label;
  GtkWidget	* button;
  char		* default_btn = _("  OK  ");

  top = ::gtk_dialog_new ( );
  ::gtk_window_set_policy ( GTK_WINDOW ( top ),
			    FALSE, FALSE, TRUE );
  ::gtk_window_set_title ( GTK_WINDOW ( top ), PACKAGE );
  ::gtk_window_set_position ( GTK_WINDOW ( top ), GTK_WIN_POS_CENTER );
  ::gtk_container_border_width ( GTK_CONTAINER ( GTK_DIALOG ( top )->vbox ),
				 10 );
  ::gtk_widget_realize ( top );

  ::gtk_signal_connect ( GTK_OBJECT ( top ), "delete_event",
			 GTK_SIGNAL_FUNC ( ::delete_event ), 0 );

  if ( parent != 0 )
    ::gtk_window_set_transient_for ( GTK_WINDOW ( top ),
				     GTK_WINDOW ( parent ) );

  vbox = GTK_DIALOG ( top )->vbox;

  label = ::gtk_label_new ( _( msg ) );
  ::gtk_label_set_line_wrap (GTK_LABEL (label), true);
  ::gtk_box_pack_start ( GTK_BOX ( vbox ), label, TRUE, TRUE, 0 );
  ::gtk_widget_show ( label );

  if ( btn1 == 0 && btn2 == 0 )
    btn2 = default_btn;

  if ( btn1 != 0 )
    {
      button = ::gtk_button_new_with_label ( btn1 );
      if ( btn2 == 0 )
        {
          GTK_WIDGET_SET_FLAGS ( button, GTK_CAN_DEFAULT );
        }
      ::gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( top )->action_area ),
			     button, FALSE, FALSE, 5 );
      ::gtk_object_set_data ( GTK_OBJECT ( button ), "value", ( gpointer ) 1 );
      ::gtk_signal_connect ( GTK_OBJECT ( button ), "clicked",
			     GTK_SIGNAL_FUNC ( set_value ), this );
      if ( btn2 == 0 )
	::gtk_widget_grab_default ( button );
      ::gtk_widget_show ( button );

      m_id = 1;
    }

  if ( btn2 != 0 )
    {
      button = ::gtk_button_new_with_label ( btn2 );
      GTK_WIDGET_SET_FLAGS ( button, GTK_CAN_DEFAULT );
      ::gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( top )->action_area ),
			     button, FALSE, FALSE, 5 );
      ::gtk_object_set_data ( GTK_OBJECT ( button ), "value", ( gpointer ) 2 );
      ::gtk_signal_connect ( GTK_OBJECT ( button ), "clicked",
			     GTK_SIGNAL_FUNC ( set_value ), this );
      ::gtk_widget_grab_default ( button );
      ::gtk_widget_show ( button );

      m_id = 2;
    }
  
  ::gtk_widget_show ( top );

  ::gtk_grab_add ( top );
  ::gtk_main ( );
  ::gtk_grab_remove ( top );
  ::gtk_widget_destroy ( top );

  return m_id;
}

