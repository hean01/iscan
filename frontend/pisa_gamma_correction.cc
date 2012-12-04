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

#include <config.h>

#include "gettext.h"
#define  _(msg_id)	gettext (msg_id)

/*------------------------------------------------------------*/
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*------------------------------------------------------------*/
#include "pisa_view_manager.h"
#include "pisa_gamma_correction.h"
#include "xpm_data.h"
#include "pisa_tool.h"
#include "pisa_error.h"

#define RADIUS		3
#define MIN_DISTANCE	8
#define NUM_POINTS	13

/*------------------------------------------------------------*/
char ** g_gamma_xpm [ ] =
{
  gamma_m_xpm,	// mono
  gamma_r_xpm,	// red
  gamma_g_xpm,	// green
  gamma_b_xpm,	// blue
};

/*------------------------------------------------------------*/
static void switch_page ( GtkNotebook * notebook,
			  GtkNotebookPage * page,
			  gint page_num,
			  gpointer * data )
{
  gamma_correction	* gamma_cls;

  notebook = notebook;
  page = page;

  gamma_cls = ( gamma_correction * ) data;

  gamma_cls->change_page ( page_num );
}

/*------------------------------------------------------------*/
static void click_reset ( GtkWidget * widget,
			  gpointer * data )
{
  gamma_correction	* gamma_cls;

  widget = widget;

  gamma_cls = ( gamma_correction * ) data;

  gamma_cls->reset ( 0 );

  ::g_view_manager->update_lut ( );

  preview_window *prev_cls =
    (preview_window *) ::g_view_manager->get_window_cls (ID_WINDOW_PREV);

  prev_cls->update_img ( );
}

/*------------------------------------------------------------*/
static gint gamma_curve_event ( GtkWidget * widget,
				GdkEvent * event,
				gpointer * data )
{
  gamma_correction	* gamma_cls;

  gamma_cls = ( gamma_correction * ) data;

  return gamma_cls->event ( widget, event );
}

/*------------------------------------------------------------*/
int gamma_correction::init ( void )
{
  int i;

  m_page = 0;

  for ( i = 0; i < WG_GAMMA_NUM; i++ )
    m_widget [ i ] = 0;

  for ( i = 0; i < 4; i++ )
    {
      m_pixmap [ i ] = 0;
      m_active [ i ] = 0;

      reset_points ( i );
    }

  m_basis [ 0 ] [ 0 ] = -0.5; m_basis [ 0 ] [ 1 ] =  1.5; m_basis [ 0 ] [ 2 ] = -1.5; m_basis [ 0 ] [ 3 ] =  0.5;
  m_basis [ 1 ] [ 0 ] =  1.0; m_basis [ 1 ] [ 1 ] = -2.5; m_basis [ 1 ] [ 2 ] =  2.0; m_basis [ 1 ] [ 3 ] = -0.5;
  m_basis [ 2 ] [ 0 ] = -0.5; m_basis [ 2 ] [ 1 ] =  0.0; m_basis [ 2 ] [ 2 ] =  0.5; m_basis [ 2 ] [ 3 ] =  0.0;
  m_basis [ 3 ] [ 0 ] =  0.0; m_basis [ 3 ] [ 1 ] =  1.0; m_basis [ 3 ] [ 2 ] =  0.0; m_basis [ 3 ] [ 3 ] =  0.0;

  for ( i = 0; i < 2; i++ )
    m_cursor [ i ] = 0;

  return PISA_ERR_SUCCESS;
}

/*------------------------------------------------------------*/
GtkWidget * gamma_correction::create_controls ( GtkWidget * parent )
{
  GtkWidget	* vbox;
  GtkWidget	* widget;

  vbox = ::gtk_vbox_new ( FALSE, 2 );
  widget = create_gamma_notebook ( parent );
  ::gtk_box_pack_start ( GTK_BOX ( vbox ), widget, FALSE, FALSE, 0 );
  ::gtk_widget_show ( widget );

  widget = create_reset_button ( );
  ::gtk_box_pack_start ( GTK_BOX ( vbox ), widget, FALSE, FALSE, 0 );
  ::gtk_widget_show ( widget );
  
  m_cursor [ PISA_CS_GM_X      ] = ::gdk_cursor_new ( GDK_X_CURSOR );
  m_cursor [ PISA_CS_GM_FLEUR  ] = ::gdk_cursor_new ( GDK_FLEUR );
  m_cursor [ PISA_CS_GM_TCROSS ] = ::gdk_cursor_new ( GDK_TCROSS );

  return vbox;
}

/*------------------------------------------------------------*/
int gamma_correction::close_window ( int destroy )
{
  int i;

  destroy = destroy;

  for ( i = 0; i < 2; i++ )
    {
      if ( m_cursor [ i ] )
	::gdk_cursor_destroy ( m_cursor [ i ] );
      m_cursor [ i ] = 0;
    }

  return PISA_ERR_SUCCESS;
}

/*------------------------------------------------------------*/
void gamma_correction::sensitive ( int is_prev_img )
{
  gboolean	enable_rgb, enable_r_g_b;

  settings set = g_view_manager->get_settings ();

  // grayout
  switch ( set.imgtype.pixeltype )
    {
    case PISA_PT_RGB:
      enable_rgb	= TRUE;
      enable_r_g_b	= TRUE;
      break;
    case PISA_PT_GRAY:
      enable_rgb	= TRUE;
      enable_r_g_b	= FALSE;
      break;
    case PISA_PT_BW:
      enable_rgb	= FALSE;
      enable_r_g_b	= FALSE;
      break;
    default:
      return;
    }

  if ( is_prev_img == 0 )
    enable_rgb = enable_r_g_b = FALSE;

  m_active [ 0 ] = enable_rgb;
  m_active [ 1 ] = enable_r_g_b;
  m_active [ 2 ] = enable_r_g_b;
  m_active [ 3 ] = enable_r_g_b;

  ::gtk_widget_set_sensitive ( m_widget [ WG_GAMMA_TAB_RGB ], enable_rgb );
  ::gtk_widget_set_sensitive ( m_widget [ WG_GAMMA_TAB_RED ], enable_r_g_b );
  ::gtk_widget_set_sensitive ( m_widget [ WG_GAMMA_TAB_GRN ], enable_r_g_b );
  ::gtk_widget_set_sensitive ( m_widget [ WG_GAMMA_TAB_BLU ], enable_r_g_b );

  ::gtk_widget_set_sensitive ( m_widget [ WG_GAMMA_BOX_RGB ], enable_rgb );
  ::gtk_widget_set_sensitive ( m_widget [ WG_GAMMA_BOX_RED ], enable_r_g_b );
  ::gtk_widget_set_sensitive ( m_widget [ WG_GAMMA_BOX_GRN ], enable_r_g_b );
  ::gtk_widget_set_sensitive ( m_widget [ WG_GAMMA_BOX_BLU ], enable_r_g_b );

  if ( enable_rgb == TRUE && enable_r_g_b == FALSE )
    ::gtk_notebook_set_page ( GTK_NOTEBOOK ( m_widget [ WG_GAMMA_NOTE ] ),
			      GAMMA_RGB );    

  ::gtk_widget_set_sensitive ( m_widget [ WG_GAMMA_RESET ], m_active [ m_page ] );

  update_gamma ( );
}

/*------------------------------------------------------------*/
void gamma_correction::change_page ( int page )
{
  m_page = page;

  ::gtk_widget_set_sensitive ( m_widget [ WG_GAMMA_RESET ], m_active [ m_page ] );

}


/*------------------------------------------------------------*/
void gamma_correction::reset ( int all )
{
  if ( all == 1 )
    {
      int i;
      
      for ( i = 0; i < 4; i++ )
	reset_points ( i );
    }
  else
    reset_points ( m_page );

  calculate_curve ( );
  update_gamma ( );
}

/*------------------------------------------------------------*/
gint gamma_correction::event ( GtkWidget * widget, GdkEvent * event )
{
  static int cursor_type = PISA_CS_GM_FLEUR;
  int new_type;
  GdkEventMotion	* mevent = NULL;
  gint	tx, ty, x, y;
  gint	i, distance, closest_point;

  ::gdk_window_get_pointer ( m_drawarea [ m_page ]->window,
			     & tx, & ty, 0 );
  x = CLAMP ( ( tx - RADIUS ), 0, GAMMA_WIDTH  - 1 );
  y = CLAMP ( ( ty - RADIUS ), 0, GAMMA_HEIGHT - 1 );

  distance = G_MAXINT;
  closest_point = 0;

  for ( i = 0; i < NUM_POINTS; i++ )
    {
      if ( m_points [ m_page ] [ i ] [ 0 ] != -1 )
	if ( ::abs ( x - m_points [ m_page ] [ i ] [ 0 ] ) < distance )
	  {
	    distance = ::abs ( x - m_points [ m_page ] [ i ] [ 0 ] );
	    closest_point = i;
	  }
    }

  if ( distance > MIN_DISTANCE )
    closest_point = ( x + 8 ) / 16;

  switch ( event->type )
    {
    case GDK_EXPOSE:
      if ( m_pixmap [ m_page ] == 0 )
	m_pixmap [ m_page ] = ::gdk_pixmap_new ( m_drawarea [ m_page ]->window,
						 GAMMA_WIDTH + RADIUS * 2,
						 GAMMA_HEIGHT + RADIUS * 2,
						 -1 );
      update_gamma ( );
      break;

    case GDK_BUTTON_PRESS:
      new_type = PISA_CS_GM_TCROSS;

      m_leftmost = -1;
      for ( i = closest_point - 1; i >= 0; i-- )
	if ( m_points [ m_page ] [ i ] [ 0 ] != -1 )
	  {
	    m_leftmost = m_points [ m_page ] [ i ] [ 0 ];
	    break;
	  }
      m_rightmost = GAMMA_WIDTH;
      for ( i = closest_point + 1; i < NUM_POINTS; i++ )
	if ( m_points [ m_page ] [ i ] [ 0 ] != -1 )
	  {
	    m_rightmost = m_points [ m_page ] [ i ][ 0 ];
	    break;
	  }

      m_grabpoint = closest_point;
      m_points [ m_page ] [ m_grabpoint ] [ 0 ] = x;
      m_points [ m_page ] [ m_grabpoint ] [ 1 ] = GAMMA_HEIGHT - 1 - y;

      calculate_curve ( );
      update_gamma ( );
      gtk_grab_add ( widget );
      break;

    case GDK_BUTTON_RELEASE:
      new_type = PISA_CS_GM_FLEUR;
      m_grabpoint = -1;
      gtk_grab_remove ( widget );
      ::g_view_manager->update_lut ( );
      {
      preview_window *prev_cls =
	(preview_window *) ::g_view_manager->get_window_cls (ID_WINDOW_PREV);

      prev_cls->update_img ( );
      }
      break;

    case GDK_MOTION_NOTIFY:
      mevent = ( GdkEventMotion * ) event;

      if ( mevent->is_hint )
	{
	  mevent->x = tx;
	  mevent->y = ty;
	}

      if ( m_grabpoint == -1 )
	{
	  if ( m_points [ m_page ] [ closest_point ] [ 0 ] != -1 )
	    new_type = PISA_CS_GM_FLEUR;
	  else
	    new_type = PISA_CS_GM_TCROSS;
	}
      else
	{
	  new_type = PISA_CS_GM_TCROSS;

	  m_points [ m_page ] [ m_grabpoint ] [ 0 ] = -1;

	  if ( x > m_leftmost && x < m_rightmost )
	    {
	      closest_point = ( x + 8 ) / 16;
	      if ( m_points [ m_page ] [ closest_point ] [ 0 ] == -1 )
		m_grabpoint = closest_point;

	      m_points [ m_page ] [ m_grabpoint ] [ 0 ] = x;
	      m_points [ m_page ] [ m_grabpoint ] [ 1 ] = GAMMA_HEIGHT - 1 - y;
	    }
	  calculate_curve ( );
	  update_gamma ( );
	}

      if ( new_type != cursor_type )
	{
	  cursor_type = new_type;
	  ::gdk_window_set_cursor ( m_drawarea [ m_page ]->window,
				    m_cursor [ cursor_type ] );
	}

      break;
      
    default:
      break;
    }

  return FALSE;
}

/*------------------------------------------------------------*/
GtkWidget * gamma_correction::create_gamma_notebook ( GtkWidget * parent )
{
  GtkWidget	* hbox;
  GtkWidget	* notebook;
  GtkWidget	* vbox;
  GtkWidget	* img;
  GtkWidget	* gamma_curve;
  int		i;

  hbox = ::gtk_hbox_new ( FALSE, 2 );

  notebook = ::gtk_notebook_new ( );
  ::gtk_container_border_width ( GTK_CONTAINER ( notebook ), 2 );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), notebook, TRUE, FALSE, 0 );

  for ( i = 0; i < 4; i++ )
    {
      vbox = gtk_vbox_new ( FALSE, 5 );
      
      img = ::xpm2widget ( parent, g_gamma_xpm [ i ] );

      m_widget [ i ] = img;

      ::gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ), vbox, img );
      ::gtk_widget_show ( vbox );

      gamma_curve = create_gamma_curve ( i );

      m_widget [ i + 4 ] = gamma_curve;

      ::gtk_box_pack_start ( GTK_BOX ( vbox ), gamma_curve, FALSE, FALSE, 0 );
      ::gtk_widget_show ( gamma_curve );
    }

  ::gtk_signal_connect ( GTK_OBJECT ( notebook ),
			 "switch_page",
			 GTK_SIGNAL_FUNC ( ::switch_page ),
			 this );
  
  ::gtk_widget_show ( notebook );

  m_widget [ WG_GAMMA_NOTE ] = notebook;

  return hbox;
}

/*------------------------------------------------------------*/
GtkWidget * gamma_correction::create_gamma_curve ( int i )
{
  GtkWidget	* frame;
  GtkWidget	* gamma_curve;

  frame = ::gtk_frame_new ( 0 );
  ::gtk_frame_set_shadow_type ( GTK_FRAME ( frame ), GTK_SHADOW_ETCHED_IN );

  gamma_curve = ::gtk_drawing_area_new ( );
  ::gtk_drawing_area_size ( GTK_DRAWING_AREA ( gamma_curve ),
			    GAMMA_WIDTH + RADIUS * 2,
			    GAMMA_HEIGHT + RADIUS * 2 );

  ::gtk_widget_set_events ( gamma_curve,
			    GDK_EXPOSURE_MASK |
			    GDK_POINTER_MOTION_MASK |
			    GDK_POINTER_MOTION_HINT_MASK |
			    GDK_ENTER_NOTIFY_MASK |
			    GDK_BUTTON_PRESS_MASK |
			    GDK_BUTTON_RELEASE_MASK |
			    GDK_BUTTON1_MOTION_MASK );

  m_drawarea [ i ] = gamma_curve;

  ::gtk_signal_connect ( GTK_OBJECT ( gamma_curve ), "event",
			 GTK_SIGNAL_FUNC ( gamma_curve_event ), this );


  ::gtk_container_add ( GTK_CONTAINER ( frame ), gamma_curve );

  ::gtk_widget_show ( gamma_curve );
  
  return frame;
}

/*------------------------------------------------------------*/
GtkWidget * gamma_correction::create_reset_button ( void )
{
  GtkWidget	* hbox;
  GtkWidget	* button;

  hbox = ::gtk_hbox_new ( FALSE, 5 );
  ::gtk_container_border_width ( GTK_CONTAINER ( hbox ), 5 );

  button = ::gtk_button_new_with_label ( _( "  Reset  " ) );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), button, TRUE, FALSE, 0 );
  ::gtk_signal_connect ( GTK_OBJECT ( button ), "clicked",
			 GTK_SIGNAL_FUNC ( ::click_reset ), this );
  ::gtk_widget_show ( button );

  m_widget [ WG_GAMMA_RESET ] = button;

  return hbox;
}

/*------------------------------------------------------------*/
void gamma_correction::reset_points ( int i )
{
  int j;

  m_grabpoint = -1;

  for ( j = 0; j < NUM_POINTS; j++ )
    {
      m_points [ i ] [ j ] [ 0 ] = -1;
      m_points [ i ] [ j ] [ 1 ] = -1;
    }
  
  for ( j = 0; j < GAMMA_WIDTH; j++ )
    m_curve [ i ] [ j ] = j;

  m_points [ i ] [ 0 ] [ 0 ] = 0;
  m_points [ i ] [ 0 ] [ 1 ] = 0;
  m_points [ i ] [ NUM_POINTS - 1 ] [ 0 ] = GAMMA_WIDTH - 1;
  m_points [ i ] [ NUM_POINTS - 1 ] [ 1 ] = GAMMA_HEIGHT - 1;
}

/*------------------------------------------------------------*/
void gamma_correction::update_gamma ( void )
{
  int i;
  GdkPoint points [ GAMMA_WIDTH ];
  GdkGC		* gc;

  if ( m_pixmap [ m_page ] == 0 )
    return;

  // clear the pixmap
  ::gdk_draw_rectangle ( m_pixmap [ m_page ],
			 m_drawarea [ m_page ]->style->bg_gc [ GTK_STATE_NORMAL ],
			 TRUE, 0, 0,
			 GAMMA_WIDTH + RADIUS * 2, GAMMA_HEIGHT + RADIUS * 2 );

  // draw the grid line
  for ( i = 0; i < 13; i++ )
    {
      ::gdk_draw_line ( m_pixmap [ m_page ],
			m_drawarea [ m_page ]->style->dark_gc [ GTK_STATE_NORMAL ],
			RADIUS, i * ( GAMMA_HEIGHT / 12 ) + RADIUS,
			GAMMA_WIDTH + RADIUS, i * ( GAMMA_HEIGHT / 12 ) + RADIUS );
      ::gdk_draw_line ( m_pixmap [ m_page ],
			m_drawarea [ m_page ]->style->dark_gc [ GTK_STATE_NORMAL ],
			i * ( GAMMA_WIDTH / 12 ) + RADIUS, RADIUS,
			i * ( GAMMA_WIDTH / 12 ) + RADIUS, GAMMA_HEIGHT + RADIUS );
    }

  if ( m_active [ m_page ] )
    gc = m_drawarea [ m_page ]->style->black_gc;
  else
    gc = m_drawarea [ m_page ]->style->dark_gc [ GTK_STATE_NORMAL ];

  // draw the curve
  for ( i = 0; i < GAMMA_WIDTH; i++ )
    {
      points [ i ].x = i + RADIUS;
      points [ i ].y = ( GAMMA_HEIGHT  ) - m_curve [ m_page ] [ i ] + RADIUS;
    }

  ::gdk_draw_points ( m_pixmap [ m_page ],
		      gc, points, GAMMA_WIDTH );

  // draw the points
  for ( i = 0; i < NUM_POINTS; i++ )
    {
      if ( m_points [ m_page ] [ i ] [ 0 ] != -1 )
	::gdk_draw_arc ( m_pixmap [ m_page ],
			 gc,
			 TRUE,
			 m_points [ m_page ] [ i ] [ 0 ],
			 ( GAMMA_HEIGHT - 1 ) - m_points [ m_page ] [ i ] [ 1 ],
			 RADIUS * 2, RADIUS * 2, 0, 23040 );
    }
  
  ::gdk_draw_pixmap ( m_drawarea [ m_page ]->window,
		      m_drawarea [ m_page ]->style->black_gc,
		      m_pixmap [ m_page ],
		      0, 0, 0, 0, GAMMA_WIDTH + RADIUS * 2,
		      GAMMA_HEIGHT + RADIUS * 2 );
  return;
}

/*------------------------------------------------------------*/
void gamma_correction::calculate_curve ( void )
{
  int i, num;
  int p1, p2, p3, p4;
  int points [ NUM_POINTS ];
  marquee * marq;

  num = 0;

  for ( i = 0; i < NUM_POINTS; i++ )
    if ( m_points [ m_page ] [ i ] [ 0 ] != -1 )
      points [ num++ ] = i;

  if ( 0 < num )
    {
      for ( i = 0; i < m_points [ m_page ] [ points [ 0 ] ] [ 0 ]; i++ )
	{
	  m_curve [ m_page ] [ i ] = 
	    m_points [ m_page ] [ points [ 0 ] ] [ 1 ];
	}
    
      for ( i = m_points [ m_page ] [ points [ num - 1 ] ] [ 0 ]; i < GAMMA_WIDTH; i++ )
	{
	  m_curve [ m_page ] [ i ] =
	    m_points [ m_page ] [ points [ num - 1 ] ] [ 1 ];
	}
    }

  for ( i = 0; i < num - 1; i++ )
    {
      p1 = ( i == 0 ) ? points [ i ] : points [ ( i - 1 ) ];
      p2 = points [ i ];
      p3 = points [ ( i + 1 ) ];
      p4 = ( i == ( num - 2 ) ) ? points [ ( num - 1 ) ] : points [ ( i + 2 ) ];

      plot_curve ( p1, p2, p3, p4 );
    }

  marq = & g_view_manager->get_marquee ();

  for ( i = 0; i < 256; i++ )
    marq->gamma_table [ m_page ] [ i ] =
      255 * ( m_curve [ m_page ] [ 191 * i / 255 ] ) / ( GAMMA_WIDTH - 1 );
}

/*------------------------------------------------------------*/
void gamma_correction::plot_curve ( int p1, int p2, int p3, int p4 )
{
  matrix geometry;
  matrix tmp1, tmp2;
  matrix deltas;
  double x, dx, dx2, dx3;
  double y, dy, dy2, dy3;
  double d, d2, d3;
  int lastx, lasty;
  int newx, newy;
  int i;

  for ( i = 0; i < 4; i++ )
    {
      geometry [ i ] [ 0 ] = 0;
      geometry [ i ] [ 1 ] = 0;
      geometry [ i ] [ 2 ] = 0;
      geometry [ i ] [ 3 ] = 0;
    }

  for ( i = 0; i < 2; i++ )
    {
      geometry [ 0 ] [ i ] = m_points [ m_page ] [ p1 ] [ i ];
      geometry [ 1 ] [ i ] = m_points [ m_page ] [ p2 ] [ i ];
      geometry [ 2 ] [ i ] = m_points [ m_page ] [ p3 ] [ i ];
      geometry [ 3 ] [ i ] = m_points [ m_page ] [ p4 ] [ i ];
    }

  d = 1.0 / 1000;
  d2 = d * d;
  d3 = d * d * d;

  tmp2 [ 0 ] [ 0 ] = 0;      tmp2 [ 0 ] [ 1 ] = 0;      tmp2 [ 0 ] [ 2 ] = 0; tmp2 [ 0 ] [ 3 ] = 1;
  tmp2 [ 1 ] [ 0 ] = d3;     tmp2 [ 1 ] [ 1 ] = d2;     tmp2 [ 1 ] [ 2 ] = d; tmp2 [ 1 ] [ 3 ] = 0;
  tmp2 [ 2 ] [ 0 ] = 6 * d3; tmp2 [ 2 ] [ 1 ] = 2 * d2; tmp2 [ 2 ] [ 2 ] = 0; tmp2 [ 2 ] [ 3 ] = 0;
  tmp2 [ 3 ] [ 0 ] = 6 * d3; tmp2 [ 3 ] [ 1 ] = 0;      tmp2 [ 3 ] [ 2 ] = 0; tmp2 [ 3 ] [ 3 ] = 0;

  compose_curve ( m_basis, geometry, tmp1 );

  compose_curve ( tmp2, tmp1, deltas );

  x	= deltas [ 0 ] [ 0 ];
  dx	= deltas [ 1 ] [ 0 ];
  dx2	= deltas [ 2 ] [ 0 ];
  dx3	= deltas [ 3 ] [ 0 ];

  y	= deltas [ 0 ] [ 1 ];
  dy	= deltas [ 1 ] [ 1 ];
  dy2	= deltas [ 2 ] [ 1 ];
  dy3	= deltas [ 3 ] [ 1 ];

  lastx = ( int ) ( CLAMP ( x, 0, GAMMA_WIDTH - 1 ) );
  lasty = ( int ) ( CLAMP ( y, 0, GAMMA_HEIGHT - 1 ));

  m_curve [ m_page ] [ lastx ] = lasty;

  for ( i = 0; i < 1000; i++ )
    {
      x += dx;
      dx += dx2;
      dx2 += dx3;
      
      y += dy;
      dy += dy2;
      dy2 += dy3;

      newx = CLAMP ( int ( x + 0.5 ), 0, GAMMA_WIDTH - 1 );
      newy = CLAMP ( int ( y + 0.5 ), 0, GAMMA_HEIGHT - 1 );

      if ( ( lastx != newx ) || ( lasty != newy ) )
	m_curve [ m_page ] [ newx ] = newy;

      lastx = newx;
      lasty = newy;
    }
}

/*------------------------------------------------------------*/
void gamma_correction::compose_curve ( matrix a, matrix b, matrix ab )
{
  int i, j;

  for ( i = 0; i < 4; i++ )
    {
      for ( j = 0; j < 4; j++ )
	{
	  ab [ i ] [ j ] = ( a [ i ] [ 0 ] * b [ 0 ] [ j ] +
			     a [ i ] [ 1 ] * b [ 1 ] [ j ] +
			     a [ i ] [ 2 ] * b [ 2 ] [ j ] +
			     a [ i ] [ 3 ] * b [ 3 ] [ j ] );
	}
    }
}
