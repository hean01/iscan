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

/*--------------------------------------------------------------*/
#include "pisa_tool.h"

#include "gettext.h"
#define  _(msg_id)      gettext (msg_id)

template <>
double similarity <double> ( const double & A,
			     const double & a,
			     const double & b )
{
  double B;

  if ( a == 0 )
    return 0;

  B = ( b * A ) / a;

  return B;
}

/*--------------------------------------------------------------*/
GtkWidget * xpm2widget ( GtkWidget * widget, char ** xpm_data )
{
  GdkBitmap	* mask;
  GdkPixmap	* pixmap;
  GtkWidget	* pixmap_widget;
  GtkStyle	* style;

  style = ::gtk_widget_get_style ( widget );

  pixmap = ::gdk_pixmap_create_from_xpm_d ( widget->window,
					    & mask,
					    & style->bg [ GTK_STATE_NORMAL ],
					    xpm_data );
  
  pixmap_widget = ::gtk_pixmap_new ( pixmap, mask );

  ::gtk_widget_show ( pixmap_widget );

  return pixmap_widget;
}


/*------------------------------------------------------------*/
GtkWidget * xpmlabelbox ( GtkWidget * parent, char ** xpm_data,
                          const char * text )
{
  GtkWidget	* hbox;
  GtkWidget	* label;
  GtkWidget	* img;

  hbox = ::gtk_hbox_new ( FALSE, 0 );
  ::gtk_container_border_width ( GTK_CONTAINER ( hbox ), 3 );
  
  img = ::xpm2widget ( parent, xpm_data );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), img, FALSE, FALSE, 3 );
  ::gtk_widget_show ( img );
  
  label = ::gtk_label_new ( text );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 3 );
  ::gtk_widget_show ( label );

  return hbox;
}


/*------------------------------------------------------------*/
GtkWidget *  pisa_create_toolbar ( const GtkWidget * parent,
				   toolbar_items * list )
{
  GtkWidget		* toolbar;

#ifdef HAVE_GTK_2
  toolbar = ::gtk_toolbar_new ();
#else
  toolbar = ::gtk_toolbar_new ( GTK_ORIENTATION_HORIZONTAL,
				GTK_TOOLBAR_ICONS );
#endif

  while ( list->func )
    {
      if ( list->xpm )
	{
	  list->widget = ::xpm2widget ( GTK_WIDGET ( parent ), list->xpm );
	  ::gtk_toolbar_append_item ( GTK_TOOLBAR ( toolbar ),
				      0, 0, 0,
				      list->widget,
				      GTK_SIGNAL_FUNC ( list->func ),
				      0 );
	}
      else
	::gtk_toolbar_append_space ( GTK_TOOLBAR ( toolbar ) );
      
      list++;
    }

  return toolbar;
}


/*------------------------------------------------------------*/
GtkWidget * pisa_create_option_menu ( menu_items * list )
{
  GtkWidget	* option_menu;
  GtkWidget	* menu, * item;

  option_menu = ::gtk_option_menu_new ( );
  ::gtk_widget_show ( option_menu );

  menu = ::gtk_menu_new ( );

  while ( list->func )
    {
      list->widget = item = ::gtk_menu_item_new_with_label (_(list->name));
      ::gtk_signal_connect ( GTK_OBJECT ( item ), "activate",
			     GTK_SIGNAL_FUNC ( list->func ),
			     ( void * ) & list->id );
      ::gtk_menu_append ( GTK_MENU ( menu ), item );
      ::gtk_widget_show ( item );

      list++;
    }

  ::gtk_option_menu_set_menu ( GTK_OPTION_MENU ( option_menu ), menu );

  return option_menu;
}


/*--------------------------------------------------------------*/
GtkWidget * pisa_create_scale ( scale_items * list )
{
  GtkWidget	* scale;
  GtkObject	* adjust;

  adjust = ::gtk_adjustment_new ( list->val,
				  list->min,
				  list->max,
				  list->step,
				  list->page,
				  0 );
  list->adjust = GTK_ADJUSTMENT ( adjust );
  
  ::gtk_signal_connect ( adjust, "value_changed",
			 GTK_SIGNAL_FUNC ( list->func ),
			 ( void * ) & list->id );
  
  scale = ::gtk_hscale_new ( GTK_ADJUSTMENT ( adjust ) );

  list->widget = scale;
  
  return scale;
}
