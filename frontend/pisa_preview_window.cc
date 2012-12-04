/* pisa_preview_window.cc
   Copyright (C) 2001, 2004, 2005, 2008, 2009  SEIKO EPSON CORPORATION

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

/*------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*------------------------------------------------------------*/
#include "pisa_view_manager.h"
#include "pisa_preview_window.h"
#include "pisa_error.h"
#include "pisa_scan_tool.h"
#include "pisa_tool.h"
#include "pisa_default_val.h"
#include "pisa_aleart_dialog.h"
#include "pisa_change_unit.h"

/*------------------------------------------------------------*/
long g_prev_max_x		= 320;
long g_prev_max_y		= 440;

const double g_min_marq_size	= 0.4;

/*------------------------------------------------------------*/
static gint expose_event ( GtkWidget * widget,
			   GdkEventExpose * event )
{
  preview_window	* prev_cls;

  prev_cls = ( preview_window * ) ::g_view_manager->get_window_cls ( ID_WINDOW_PREV );

  return prev_cls->expose_event ( widget, event );
}


/*------------------------------------------------------------*/
static gint event ( GtkWidget * widget,
		    GdkEvent * event )
{
  preview_window	* prev_cls;

  prev_cls = ( preview_window * ) ::g_view_manager->get_window_cls ( ID_WINDOW_PREV );

  return prev_cls->event ( widget, event );
}

/*------------------------------------------------------------*/
static void size_allocate ( GtkWidget * widget )
{
  preview_window	* prev_cls;

  prev_cls = ( preview_window * ) ::g_view_manager->get_window_cls ( ID_WINDOW_PREV );

  prev_cls->size_allocate ( widget );
}

/*------------------------------------------------------------*/
int preview_window::init ( void )
{
  int	i;

  m_gc		= 0;

  m_img_width	= g_prev_max_x;
  m_img_height	= g_prev_max_y;

  m_is_prev	= 0;
  m_drag	= 0;

  m_img		= new unsigned char [ g_prev_max_x * g_prev_max_y * 3 ];

  if ( m_img == 0 )
    return PISA_ERR_OUTOFMEMORY;

  m_img_org	= new unsigned char [ g_prev_max_x * g_prev_max_y * 3 ];

  if ( m_img_org == 0 )
    {
      delete [ ] m_img;
      m_img = 0;
      return PISA_ERR_OUTOFMEMORY;
    }

  for ( i = 0; i < 11; i++ )
    m_cursor [ i ] = 0;

  m_on_preview = false;
  m_allocate_width = 0;
  m_allocate_height = 0;
  
  return PISA_ERR_SUCCESS;
}

/*------------------------------------------------------------*/
GtkWidget * preview_window::create_window ( GtkWidget * parent )
{
  m_win = create_darea ( parent );

  create_cursor ( );

  return m_win;
}

/*------------------------------------------------------------*/
int preview_window::close_window ( int destroy )
{
  destroy = destroy;

  if ( m_gc )
    {
      ::gdk_gc_destroy ( m_gc );
      m_gc = 0;
    }

  if ( m_img )
    {
      delete [ ] m_img;
      m_img = 0;
    }

  if ( m_img_org )
    {
      delete [ ] m_img_org;
      m_img_org = 0;
    }

  return PISA_ERR_SUCCESS;
}

/*------------------------------------------------------------*/
int preview_window::resize_window ( void )
{
  get_preview_resolution ( );
  resize_preview_window ( 0, 0 );

  clear_image ( );

  return 0;
}

/*------------------------------------------------------------*/
int preview_window::auto_exposure ( void )
{
  pisa_image_info	info;
  _rectL		region;
  marquee		tmp_marq, * marq;
  int			i, marq_num;

  scan_manager	*scan_mgr = g_view_manager->get_scan_manager ();

  if ( m_is_prev == 0 ||
       g_view_manager->get_settings ().imgtype.pixeltype == PISA_PT_BW )
    {
      return PISA_ERR_SUCCESS;
    }

  info.m_img	= m_img_org;
  info.m_width	= m_img_width;
  info.m_height	= m_img_height;
  info.m_rowbytes = g_prev_max_x * 3;

  marq_num = g_view_manager->get_marquee_size ();
  marq = & g_view_manager->get_marquee ();

  if ( marq_num < 2 )
    {
      region.left	= 0;
      region.top	= 0;
      region.right	= m_img_width - 1;
      region.bottom	= m_img_height - 1;
    }
  else
    {
      _pointL pt_lt, pt_rb;

      get_marquee_point ( marq_num - 1, & pt_lt, & pt_rb );

      region.left	= pt_lt.x;
      region.top	= pt_lt.y;
      region.right	= pt_rb.x;
      region.bottom	= pt_rb.y;
    }

  settings set = g_view_manager->get_settings ();
  scan_manager *sm = g_view_manager->get_scan_manager ();
  iscan::auto_expose (sm->get_scan_source (),
                      sm->get_film_type (),
                      info, region, tmp_marq,
		      set.imgtype.ae_option != PISA_AE_DOC,
		      !scan_mgr->has_zoom ());

  marq->gamma		= tmp_marq.gamma;
  marq->highlight	= tmp_marq.highlight;
  marq->shadow		= tmp_marq.shadow;
  marq->graybalance	= tmp_marq.graybalance;

  for ( i = 0; i < 3; i++ )
    {
      marq->film_gamma [ i ]	= tmp_marq.film_gamma [ i ];
      marq->film_yp [ i ]	= tmp_marq.film_yp [ i ];
      marq->grayl [ i ]		= tmp_marq.grayl [ i ];
    }

  ::g_view_manager->sensitive ( );
  ::g_view_manager->update_lut ( );

  return PISA_ERR_SUCCESS;
}

/*------------------------------------------------------------*/
int preview_window::update_img ( bool left )
{
  int			i;
  pisa_image_info	img_info;
  marquee		* marq;
  double                delta;
  long                  curr_width, adj_width, resolution;

  if ( m_is_prev == 0 )
    {
      clear_image ( );
      return PISA_ERR_SUCCESS;
    }

  settings set = g_view_manager->get_settings ();
  marq = & g_view_manager->get_marquee ();

  resolution = (set.resolution * (marq->scale)) / 100;
  curr_width = ::inch2pixel ( marq->area.x, resolution );
  adj_width  = ::inch2width ( marq->area.x, resolution );
  delta      = marq->area.x - (marq->area.x) * adj_width /curr_width ;

  // Resize the area 
  marq -> area.x -= delta;
  if ( left )
    {
      // If the left side -> increase the x offset
      marq ->offset.x += delta;
    }

  if ( m_img == 0 || m_img_org == 0 )
    return PISA_ERR_OUTOFMEMORY;

  ::memcpy ( m_img, m_img_org, g_prev_max_x * g_prev_max_y * 3 );

  img_info.m_img	= m_img;
  img_info.m_width	= m_img_width;
  img_info.m_height	= m_img_height;
  img_info.m_rowbytes	= g_prev_max_x * 3;

  ::tool_lut ( & img_info, & marq->lut );

  switch (set.imgtype.pixeltype)
    {
    case PISA_PT_RGB:
      tool_matrix (&img_info, marq->saturation, set.coef);
      if (set.usm)
	tool_usm (img_info);
      break;

    case PISA_PT_GRAY:
      build_gray (&img_info, set.imgtype.dropout);
      if (set.usm)
	tool_usm (img_info);
      break;

    case PISA_PT_BW:
      build_bw (&img_info,
		set.imgtype.dropout,
		set.imgtype.halftone,
		marq->threshold);
      break;
    }

  for ( i = 0; i < m_img_height; i++ )
    ::gtk_preview_draw_row ( GTK_PREVIEW ( m_prev ),
			     m_img + i * g_prev_max_x * 3,
			     0, i, m_img_width );
  
  ::gtk_widget_draw ( m_prev, 0 );
  ::gdk_flush ( );

  return PISA_ERR_SUCCESS;
}

void
preview_window::start_preview (bool zooming)
{
  scan_manager		* scan_mgr =
    g_view_manager->get_scan_manager ( );
  int			width, height;
  int			cancel;

  cancel = 0;

  m_is_prev = 0;

  m_on_preview = true;
  m_allocate_width = 0;
  m_allocate_height = 0;

  progress_window feedback
    (static_cast <main_window *> (g_view_manager
				  ->get_window_cls (ID_WINDOW_MAIN))
     ->get_widget());
  if (scan_mgr->push_button_needs_polling ())
  {
    feedback.set_text (progress_window::WAITING);
  }
  else
  {
    feedback.set_text (progress_window::WARMING_UP);
  }
  feedback.show ();

  while ( ::gtk_events_pending ( ) )
    ::gtk_main_iteration ( );

  // preview
  set_preview_param (zooming);

  reset_settings (zooming);
  ::g_view_manager->sensitive ( );

  try
    {
      if (scan_mgr->push_button_needs_polling ())
      {
        long usec = scan_mgr->get_polling_time ();
        while (!scan_mgr->is_button_pressed ()
               && !feedback.is_cancelled ()) {
          g_view_manager->microsleep (usec);
          while (::gtk_events_pending ()) {
            ::gtk_main_iteration ();
          }
        }
        if (feedback.is_cancelled ())
        {
          return;
        }
        scan_mgr->disable_wait_for_button();
      }

      scan_mgr->init_preview (&width, &height);

      if (!zooming)
	{
	  change_max_scan_area ( width, height );
	  ::g_view_manager->sensitive ( );
	}

      resize_preview_window ( width, height );
      clear_image ( );
      
      while ( ::gtk_events_pending ( ) )
	::gtk_main_iteration ( );

      for (int i = 0; i < height; i++ )
	{
	  scan_mgr->acquire_image ( m_img_org + i * g_prev_max_x * 3,
				    width * 3,
				    1,
				    cancel );

	  if ( i == 0 )
	    feedback.set_text (progress_window::PREVIEWING);

	  if (cancel)
	    break;

	  feedback.set_progress (i, height);
	  cancel = feedback.is_cancelled ();
	  
	  ::gtk_preview_draw_row ( GTK_PREVIEW ( m_prev ),
				   m_img_org + i * g_prev_max_x * 3,
				   0, i, width );
	  if ( i % 20 == 0 )
	    {
	      ::gtk_preview_put ( GTK_PREVIEW ( m_prev ),
				  m_prev->window,
				  m_prev->style->black_gc,
				  0, 0, 0, 0, width, height );
	    }
	  
	  while ( ::gtk_events_pending ( ) )
	    ::gtk_main_iteration ( );
	}
      scan_mgr->acquire_image (0, 1, 1, cancel);
      feedback.set_progress (height, height);
      cancel = feedback.is_cancelled ();
      if (cancel)
	{	
	  m_on_preview = false;
	  update_img ( );
	  change_max_disp_area ( m_allocate_width, m_allocate_height );
	  m_allocate_width = 0;
	  m_allocate_height = 0;
	}
      else
	{
	  m_is_prev = 1;
	  ::gtk_widget_draw ( m_prev, 0 );
	  ::gdk_flush ( );

	  m_on_preview = false;

	  auto_exposure ( );
	  update_img ( );
	  change_max_disp_area ( m_allocate_width, m_allocate_height );
	  m_allocate_width = 0;
	  m_allocate_height = 0;
	}
    }
  catch ( const pisa_error & err )
    {
      main_window * main_cls;
      aleart_dialog aleart_dlg;

      if ( PISA_STATUS_CANCELLED != err.get_error_id() )
        {
          main_cls = ( main_window * ) ::g_view_manager->get_window_cls ( ID_WINDOW_MAIN );
          aleart_dlg.message_box ( main_cls->get_widget ( ),
			           err.get_error_string ( ) );
        }

      delete_marquee ( );

      m_on_preview = false;
      update_img ( );
      change_max_disp_area ( m_allocate_width, m_allocate_height );
      m_allocate_width = 0;
      m_allocate_height = 0;
    }

  revert_resolution ();

  scan_mgr->release_memory ();
  scan_mgr->finish_acquire (true);

  ::g_view_manager->sensitive ( );

  {
    gint x, y;
    ::gdk_window_get_pointer ( m_prev->window, & x, & y, 0 );
    set_mouse_cursor ( x, y );
    change_cursor ( );
  }
}

/*--------------------------------------------------------------*/
gint preview_window::expose_event ( GtkWidget * widget,
				    GdkEventExpose * event )
{
  widget = widget;
  event = event;

  draw_marquee ( );

  return FALSE;
}

/*--------------------------------------------------------------*/
gint preview_window::event ( GtkWidget * widget, GdkEvent * event )
{
  gint	ret;

  widget = widget;

  switch ( event->type )
    {
    case GDK_EXPOSE:
      if ( m_gc )
	{
	  draw_marquee ( );
	}
      ret = FALSE;
      break;

    case GDK_BUTTON_PRESS:
      ret = mouse_down ( event );
      ::gtk_grab_add ( widget );
      break;

    case GDK_MOTION_NOTIFY:
      ret = mouse_move ( event );
      break;

    case GDK_BUTTON_RELEASE:
      ret = mouse_up ( event );
      ::gtk_grab_remove ( widget );
      break;

    default:
      ret = TRUE;
      break;
    }

  return ret;
}

/*------------------------------------------------------------*/
void preview_window::size_allocate ( GtkWidget * widget )
{
  int x, y;

  x = widget->allocation.width;
  y = widget->allocation.height;

  if ( m_on_preview )
    {
      m_allocate_width = x;
      m_allocate_height = y;
      return;
    }

  change_max_disp_area ( x, y );
}


/*------------------------------------------------------------*/
GtkWidget * preview_window::create_darea ( GtkWidget * parent )
{
  GtkWidget	* frame;

  parent = parent;

  frame = ::gtk_frame_new ( 0 );
  ::gtk_frame_set_shadow_type ( GTK_FRAME ( frame ), GTK_SHADOW_IN );
  ::gtk_container_border_width ( GTK_CONTAINER ( frame ), 3 );
  
  m_prev = ::gtk_preview_new ( GTK_PREVIEW_COLOR );
  ::gtk_preview_set_expand ( GTK_PREVIEW ( m_prev ), TRUE );

  // set size of preview window
  get_preview_resolution ( );
  resize_preview_window ( m_img_width, m_img_height );

  ::gtk_container_add ( GTK_CONTAINER ( frame ), m_prev );
  ::gtk_widget_show ( m_prev );

  clear_image ( );

  // signals
  ::gtk_signal_connect ( GTK_OBJECT ( m_prev ), "event",
			 GTK_SIGNAL_FUNC ( ::event ), 0 );
  ::gtk_signal_connect_after ( GTK_OBJECT ( m_prev ), "expose_event",
			       GTK_SIGNAL_FUNC ( ::expose_event ), 0 );
  ::gtk_signal_connect_after ( GTK_OBJECT ( m_prev ), "size_allocate",
			       GTK_SIGNAL_FUNC ( ::size_allocate ), 0 );

  ::gtk_widget_set_events ( m_prev ,
  			  GDK_EXPOSURE_MASK |
  			  GDK_LEAVE_NOTIFY_MASK |
  			  GDK_BUTTON_PRESS_MASK |
  			  GDK_POINTER_MOTION_MASK |
  			  GDK_POINTER_MOTION_HINT_MASK |
			  GDK_BUTTON_RELEASE_MASK );

  return frame;
}


/*------------------------------------------------------------*/
long preview_window::get_preview_resolution ( const _rectD * img_rect )
{
  long		resolution;
  _pointD	pt_scan_size;
  double	aspect_max;
  double	aspect_scan;

  if ( img_rect )
    {
      pt_scan_size.x = img_rect->right - img_rect->left;
      pt_scan_size.y = img_rect->bottom - img_rect->top;
      m_img_rect = * img_rect;
    }
  else
    {
      pt_scan_size.x = g_view_manager->get_settings ().max_area [ 0 ];
      pt_scan_size.y = g_view_manager->get_settings ().max_area [ 1 ];

      m_img_rect.left	= 0.0;
      m_img_rect.top	= 0.0;
      m_img_rect.right	= pt_scan_size.x;
      m_img_rect.bottom	= pt_scan_size.y;
    }

  aspect_max = ( float ) g_prev_max_y / g_prev_max_x;
  aspect_scan = ( float ) pt_scan_size.y / pt_scan_size.x;

  if ( aspect_max < aspect_scan )
    resolution = ( long ) ( g_prev_max_y / pt_scan_size.y );
  else
    resolution = ( long ) ( g_prev_max_x / pt_scan_size.x );

  m_img_width	= ( long ) ( resolution * pt_scan_size.x );
  m_img_height	= ( long ) ( resolution * pt_scan_size.y );

  return resolution;
}

/*------------------------------------------------------------*/
void preview_window::resize_preview_window ( long width, long height )
{
  if ( width != 0 && height != 0 )
    {
      m_img_width = width;
      m_img_height = height;
    }

  if ( m_img )
    {
      delete [ ] m_img;
      m_img = new unsigned char [ g_prev_max_x * g_prev_max_y * 3 ];
    }

  if ( m_img_org )
    {
      delete [ ] m_img_org;
      m_img_org = new unsigned char [ g_prev_max_x * g_prev_max_y * 3 ];
    }

  m_client_rect.left	= 0;
  m_client_rect.top	= 0;
  m_client_rect.right	= m_img_width - 1;
  m_client_rect.bottom	= m_img_height - 1;

  modify_max_val ( );
}

/*------------------------------------------------------------*/
void preview_window::change_max_scan_area ( long width, long height )
{
  _pointD	pt_scan_size;
  marquee	* marq;

  if ( m_img_width < width || m_img_height < height )
    return;

  marq = & g_view_manager->get_marquee ();

  pt_scan_size.x = g_view_manager->get_settings ().max_area [ 0 ];
  pt_scan_size.y = g_view_manager->get_settings ().max_area [ 1 ];

  m_img_rect.left	= 0.0;
  m_img_rect.top	= 0.0;
  m_img_rect.right	= pt_scan_size.x * width / m_img_width;
  m_img_rect.bottom	= pt_scan_size.y * height / m_img_height;

  marq->area.x = m_img_rect.right;
  marq->area.y = m_img_rect.bottom;
}

/*------------------------------------------------------------*/
void preview_window::change_max_disp_area ( long width, long height )
{
  long	save_prev_x, save_prev_y;
  long	save_img_width, save_img_height;
  unsigned char * save_img_org;
  _rectD tmp_rect;

  if ( width < 1 && height < 1 )
    return;

  if ( g_prev_max_x == width && g_prev_max_y == height )
    return;

  tmp_rect = m_img_rect;

  if ( m_is_prev == 0 )
    {
      g_prev_max_x = width;
      g_prev_max_y = height;

      get_preview_resolution ( & tmp_rect );
      resize_preview_window ( 0, 0 );

      clear_image ( );

      return;
    }

  // save old parameter and image
  save_prev_x		= g_prev_max_x;
  save_prev_y		= g_prev_max_y;
  save_img_width	= m_img_width;
  save_img_height	= m_img_height;
  save_img_org = new unsigned char [ save_prev_x * save_prev_y * 3 ];
 
  if ( save_img_org )
    {
      ::memcpy ( save_img_org, m_img_org, save_prev_x * save_prev_y * 3 );
    }

  // resize
  g_prev_max_x = width;
  g_prev_max_y = height;

  get_preview_resolution ( & tmp_rect );
  resize_preview_window ( 0, 0 );

  clear_image ( );
  m_is_prev = 1;

  // restore image
  if ( save_img_org )
    {
      struct resize_img_info parms;

      parms.in_width = save_img_width;
      parms.in_height = save_img_height;
      parms.in_rowbytes = save_prev_x * 3;
      parms.out_width = m_img_width;
      parms.out_height = m_img_height;
      parms.out_rowbytes = g_prev_max_x * 3;
      parms.bits_per_pixel = 24;
      parms.resize_flag = PISA_RS_BL;
      parms.resolution = 0;	// just to make sure it's set

      iscan::scale f (parms);

      f.exec (save_img_org, parms.in_height  * parms.in_rowbytes,
	      m_img_org, parms.out_height * parms.out_rowbytes);
 
      delete [] save_img_org;
    }

  update_img ( );
}

/*------------------------------------------------------------*/
void preview_window::clear_image ( void )
{
  int	i;

  m_is_prev = 0;

  ::memset ( m_img, 0xff, m_img_width * 3 );
  ::memset ( m_img + ( m_img_width * 3 ),
	     0xC4,
	     ( g_prev_max_x - m_img_width ) * 3 );  

  for ( i = 0; i < m_img_height; i++ )
    ::gtk_preview_draw_row ( GTK_PREVIEW ( m_prev ), m_img,
  			     0, i, g_prev_max_x );

  ::memset ( m_img, 0xC4, g_prev_max_x * 3 );

  for ( ; i < g_prev_max_y; i++ )
    ::gtk_preview_draw_row ( GTK_PREVIEW ( m_prev ), m_img,
			     0, i, g_prev_max_x );

  ::gtk_widget_draw ( m_prev, 0 );
  ::gdk_flush ( );
}


/*--------------------------------------------------------------*/
void preview_window::draw_marquee ( void )
{
  long		i, marq_num;
  _pointL	pt_lefttop, pt_rightbottom;

  if ( m_gc == 0 )
    {
      m_gc = ::gdk_gc_new ( m_prev->window );
      ::gdk_gc_set_function ( m_gc, GDK_INVERT );
      ::gdk_gc_set_line_attributes ( m_gc, 1, GDK_LINE_ON_OFF_DASH,
				     GDK_CAP_BUTT, GDK_JOIN_MITER );
    }

  marq_num = g_view_manager->get_settings ().get_marquee_size ( );

  if ( marq_num < 2 || m_on_preview )
    return;

  for ( i = 1; i < marq_num; i++ )
    {
      get_marquee_point ( i, & pt_lefttop, & pt_rightbottom );

      draw_rect ( pt_lefttop, pt_rightbottom );
    }
}

/*--------------------------------------------------------------*/
void
preview_window::reset_settings (bool zooming)
{
  long		i, n, m, marq_num;
  marquee	* marq;
  gamma_correction * gamma_cls;
  scan_manager	* scan_mgr;

  gamma_cls = g_view_manager->get_gamma_correction ();

  scan_mgr = g_view_manager->get_scan_manager ();
  marq_num = g_view_manager->get_marquee_size ();

  if (!zooming)
    {
      while ( 1 < marq_num )
	{
	  g_view_manager->del_marquee (marq_num - 1);
	  marq_num = g_view_manager->get_marquee_size ();
	}

      gamma_cls->reset ( 1 );
    }

  marq_num = g_view_manager->get_marquee_size ();

  for ( i = 0; i < marq_num; i++ )
    {
      marq = & g_view_manager->get_marquee (i);

      if (!zooming)
	{
	  marq->gamma		= ( long ) ( 100 * DEFGAMMA );
	  marq->highlight	= DEFHIGHLIGHT;
	  marq->shadow		= DEFSHADOW;
	  marq->threshold	= DEFTHRESHOLD;
	  
	  for ( n = 0; n < 4; n++ )
	    for ( m = 0; m < 256; m++ )
	      marq->gamma_table [ n ] [ m ] = m;
	  
	  marq->graybalance	= DEFGRAYBALANCE;
	  marq->saturation	= DEFSATURATION;
	}

      for ( n = 0; n < 3; n++ )
	{
	  marq->film_gamma [ n ]	= 1.0;
	  marq->film_yp [ n ]		= 0.0;
	  marq->grayl [ n ]		= 0.0;
	}
      
      iscan::build_LUT (scan_mgr->get_scan_source (),
                        scan_mgr->get_film_type (),
                        g_view_manager->get_settings (),
                        *marq, !scan_mgr->has_zoom ());
    }
}

int
preview_window::set_preview_param (bool zooming)
{
  settings set = g_view_manager->get_settings ();
  marquee marq;

  if (!zooming)
    {
      marq.offset.x	= 0.0;
      marq.offset.y	= 0.0;
      marq.area.x	= set.max_area[0];
      marq.area.y	= set.max_area[1];
    }
  else
    {
      float rate = 0.1f;
      _pointD pt_offset, pt_area, pt_max;

      marquee cur_marq = g_view_manager->get_marquee ();

      marq.focus = cur_marq.focus;

      pt_offset	= cur_marq.offset;
      pt_area	= cur_marq.area;

      marquee whole_marq = g_view_manager->get_marquee (0);

      pt_max.x = whole_marq.area.x;
      pt_max.y = whole_marq.area.y;

      zoom_boundary (pt_offset, pt_area, pt_max, rate);

      marq.offset.x	= pt_offset.x;
      marq.offset.y	= pt_offset.y;
      marq.area.x	= pt_area.x;
      marq.area.y	= pt_area.y;
    }

  _rectD	img_rect;
  img_rect.left		= marq.offset.x;
  img_rect.top		= marq.offset.y;
  img_rect.right	= marq.offset.x + marq.area.x;
  img_rect.bottom	= marq.offset.y + marq.area.y;

  long resolution = get_preview_resolution (&img_rect);

  pisa_error_id err = (g_view_manager->get_scan_manager ()
		       ->set_scan_parameters (set, marq, resolution));

  return err;
}

void
preview_window::revert_resolution ( void )
{
  long set_res = g_view_manager->get_settings ().resolution;

  g_view_manager->set_resolution (set_res);
}

/*------------------------------------------------------------*/
void preview_window::tool_usm (const pisa_image_info& info)
{
  iscan::focus f (info);

  f.exec (info.m_img, info.m_height * info.m_rowbytes,
	  info.m_img, info.m_height * info.m_rowbytes);
}

void
preview_window::zoom_boundary (_pointD& pt_offset, _pointD& pt_area,
			       const _pointD& pt_max, float rate) const
{
  _pointD pt_result_offset, pt_result_area;
  double bound_size;

  if (pt_area.x < pt_area.y)
    bound_size = pt_area.y * rate;
  else
    bound_size = pt_area.x * rate;

  bound_size = (0.0 == bound_size) ? 0.01 : bound_size;

  // left
  pt_result_offset.x = pt_offset.x - bound_size;
  if (0.0 > pt_result_offset.x)
    pt_result_offset.x = 0.0;

  // top
  pt_result_offset.y = pt_offset.y - bound_size;
  if (0.0 > pt_result_offset.y)
    pt_result_offset.y = 0.0;

  // right
  pt_result_area.x = ((pt_offset.x - pt_result_offset.x) +
		      pt_area.x + bound_size);
  if (pt_max.x < pt_result_offset.x + pt_result_area.x)
    pt_result_area.x = pt_max.x - pt_result_offset.x;

  // bottom
  pt_result_area.y = ((pt_offset.y - pt_result_offset.y) +
		      pt_area.y + bound_size);
  if (pt_max.y < pt_result_offset.y + pt_result_area.y)
    pt_result_area.y = pt_max.y - pt_result_offset.y;

  pt_offset = pt_result_offset;
  pt_area   = pt_result_area;
}

/*--------------------------------------------------------------*/
void preview_window::create_cursor ( void )
{
  m_cursor [ PISA_CS_ARROW       ] = ::gdk_cursor_new ( GDK_ARROW               );
  m_cursor [ PISA_CS_HAND        ] = ::gdk_cursor_new ( GDK_HAND2               );
  m_cursor [ PISA_CS_CROSS       ] = ::gdk_cursor_new ( GDK_CROSS               );
  m_cursor [ PISA_CS_TOP         ] = ::gdk_cursor_new ( GDK_TOP_SIDE            );
  m_cursor [ PISA_CS_BOTTOM      ] = ::gdk_cursor_new ( GDK_BOTTOM_SIDE         );
  m_cursor [ PISA_CS_LEFT        ] = ::gdk_cursor_new ( GDK_LEFT_SIDE           );
  m_cursor [ PISA_CS_RIGHT       ] = ::gdk_cursor_new ( GDK_RIGHT_SIDE          );
  m_cursor [ PISA_CS_LEFTTOP     ] = ::gdk_cursor_new ( GDK_TOP_LEFT_CORNER     );
  m_cursor [ PISA_CS_LEFTBOTTOM  ] = ::gdk_cursor_new ( GDK_BOTTOM_LEFT_CORNER  );
  m_cursor [ PISA_CS_RIGHTTOP    ] = ::gdk_cursor_new ( GDK_TOP_RIGHT_CORNER    );
  m_cursor [ PISA_CS_RIGHTBOTTOM ] = ::gdk_cursor_new ( GDK_BOTTOM_RIGHT_CORNER );
}


/*--------------------------------------------------------------*/
int preview_window::set_mouse_cursor ( int x, int y )
{
  const	long ADDSIZE = 3;
  long marq_num;
  _pointL pt_lefttop, pt_rightbottom;
  int cursor_state;
  _pointL pt;
  int pt_in_cur_marq;
  _rectL judge_rect;
  _pointL pt_lt, pt_rb;
  int i;

  if ( m_drag == 1 )
    return FALSE;

  pt.x = x;
  pt.y = y;

  if ( 0 == ::pt_in_rect ( m_client_rect, pt ) )
    {
      m_cursor_state = PISA_CS_ARROW;
      return FALSE;
    }

  marq_num = g_view_manager->get_settings ().get_marquee_size ( );

  if ( marq_num < 2 )
    {
      m_cursor_state = PISA_CS_CROSS;
      return FALSE;
    }

  get_marquee_point ( marq_num - 1, & pt_lefttop, & pt_rightbottom );

  cursor_state = search_cursor_state ( pt_lefttop, pt_rightbottom, pt );
  if ( cursor_state != PISA_CS_CROSS )
    {
      m_cursor_state = cursor_state;
      return FALSE;
    }

  judge_rect.left	= pt_lefttop.x;
  judge_rect.right	= pt_rightbottom.x;
  judge_rect.top	= pt_lefttop.y;
  judge_rect.bottom	= pt_rightbottom.y;
  if ( pt_in_rect ( judge_rect, pt ) )
    pt_in_cur_marq = 1;
  else
    pt_in_cur_marq = 0;

  for ( i = marq_num - 1; 0 < i; i-- )
    {
      if ( marq_num - 1 == i )
	continue;

      get_marquee_point ( i, & pt_lt, & pt_rb );

      if ( pt_in_cur_marq )
	{
	  cursor_state = search_cursor_state ( pt_lt, pt_rb, pt );
	  if ( PISA_CS_CROSS != cursor_state )
	    {
	      m_cursor_state = PISA_CS_ARROW;
	      return i;
	    }
	}
      else
	{
	  judge_rect.left	= pt_lt.x - ADDSIZE;
	  judge_rect.right	= pt_rb.x + ADDSIZE;
	  judge_rect.top	= pt_lt.y - ADDSIZE;
	  judge_rect.bottom	= pt_rb.y + ADDSIZE;
	  if ( pt_in_rect ( judge_rect, pt ) )
	    {
	      m_cursor_state = PISA_CS_ARROW;
	      return i;
	    }
	}
    }

  if ( pt_in_cur_marq )
    {
      m_cursor_state = PISA_CS_HAND;
      return 0;
    }

  m_cursor_state = PISA_CS_CROSS;

  return 0;
}

/*--------------------------------------------------------------*/
int preview_window::search_cursor_state ( const _pointL & pt_lt,
					  const _pointL & pt_rb,
					  const _pointL & pt )
{
  const long ADDSIZE	= 3;
  _rectL judge_rect;

  // check left top
  judge_rect.left	= pt_lt.x - ADDSIZE;
  judge_rect.right	= pt_lt.x + ADDSIZE;
  judge_rect.top	= pt_lt.y - ADDSIZE;
  judge_rect.bottom	= pt_lt.y + ADDSIZE;
  if ( ::pt_in_rect ( judge_rect, pt ) )
    return PISA_CS_LEFTTOP;

  // check right bottom
  judge_rect.left	= pt_rb.x - ADDSIZE;
  judge_rect.right	= pt_rb.x + ADDSIZE;
  judge_rect.top	= pt_rb.y - ADDSIZE;
  judge_rect.bottom	= pt_rb.y + ADDSIZE;
  if ( ::pt_in_rect ( judge_rect, pt ) )
    return PISA_CS_RIGHTBOTTOM;
  
  // check right top
  judge_rect.left	= pt_rb.x - ADDSIZE;
  judge_rect.right	= pt_rb.x + ADDSIZE;
  judge_rect.top	= pt_lt.y - ADDSIZE;
  judge_rect.bottom	= pt_lt.y + ADDSIZE;
  if ( ::pt_in_rect ( judge_rect, pt ) )
    return PISA_CS_RIGHTTOP;

  // check left bottom
  judge_rect.left	= pt_lt.x - ADDSIZE;
  judge_rect.right	= pt_lt.x + ADDSIZE;
  judge_rect.top	= pt_rb.y - ADDSIZE;
  judge_rect.bottom	= pt_rb.y + ADDSIZE;
  if ( ::pt_in_rect ( judge_rect, pt ) )
    return PISA_CS_LEFTBOTTOM;

  // check left resizing
  judge_rect.left	= pt_lt.x - ADDSIZE;
  judge_rect.right	= pt_lt.x + ADDSIZE;
  judge_rect.top	= pt_lt.y;
  judge_rect.bottom	= pt_rb.y;
  if ( ::pt_in_rect ( judge_rect, pt ) )
    return PISA_CS_LEFT;

  // check right resizing
  judge_rect.left	= pt_rb.x - ADDSIZE;
  judge_rect.right	= pt_rb.x + ADDSIZE;
  judge_rect.top	= pt_lt.y;
  judge_rect.bottom	= pt_rb.y;
  if ( ::pt_in_rect ( judge_rect, pt ) )
    return PISA_CS_RIGHT;

  // check top resizing
  judge_rect.left	= pt_lt.x;
  judge_rect.right	= pt_rb.x;
  judge_rect.top	= pt_lt.y - ADDSIZE;
  judge_rect.bottom	= pt_lt.y + ADDSIZE;
  if ( ::pt_in_rect ( judge_rect, pt ) )
    return PISA_CS_TOP;

  // check bottom resizing
  judge_rect.left	= pt_lt.x;
  judge_rect.right	= pt_rb.x;
  judge_rect.top	= pt_rb.y - ADDSIZE;
  judge_rect.bottom	= pt_rb.y + ADDSIZE;
  if ( ::pt_in_rect ( judge_rect, pt ) )
    return PISA_CS_BOTTOM;

  return PISA_CS_CROSS;
}

/*--------------------------------------------------------------*/
void preview_window::change_cursor ( void )
{
  ::gdk_window_set_cursor ( m_prev->window,
			    m_cursor [ m_cursor_state ] );
}

/*--------------------------------------------------------------*/
void preview_window::modify_max_val ( void )
{
  marquee	* whole_marq;
  _pointD	pt_min_D, pt_max_D;

  whole_marq = & g_view_manager->get_marquee (0);
  m_max_img_rect.top	= 0.0;
  m_max_img_rect.left	= 0.0;
  m_max_img_rect.right	= whole_marq->area.x;
  m_max_img_rect.bottom	= whole_marq->area.y;

  pt_min_D.x	= m_max_img_rect.left;
  pt_min_D.y	= m_max_img_rect.top;
  pt_max_D.x	= m_max_img_rect.right;
  pt_max_D.y	= m_max_img_rect.bottom;
  m_pt_max_rb	= inches2clientpix ( pt_max_D );
  m_pt_max_lt.x = 0;
  m_pt_max_lt.y = 0;
  m_pt_max_rb.x = m_img_width - 1;
  m_pt_max_rb.y = m_img_height - 1;
}

/*--------------------------------------------------------------*/
gint preview_window::mouse_down ( GdkEvent * event )
{
  marquee	* cur_marq;
  _pointL	pt_lefttop, pt_rightbottom;
  long		id_cur_marquee;

  if ( event->button.button != 1 )
    return TRUE;

  if ( m_img_width - 1 < ( int ) event->button.x || m_img_height - 1 < ( int ) event->button.y )
    return TRUE;

  m_pt_begin.x = ( int ) event->button.x;
  m_pt_begin.y = ( int ) event->button.y;

  m_drag = 1;
  
  m_pt_old = m_pt_begin;

  id_cur_marquee = g_view_manager->get_marquee_size () - 1;
  get_marquee_point ( id_cur_marquee, & pt_lefttop, & pt_rightbottom );

  cur_marq = & g_view_manager->get_marquee ();

  m_pt_save_offset	= cur_marq->offset;
  m_pt_save_area	= cur_marq->area;

  switch ( m_cursor_state )
    {
    case PISA_CS_CROSS:
      m_pt_old_lt = m_pt_old_rb = m_pt_begin;
      delete_marquee ( );
      create_marquee ( m_pt_old_lt, m_pt_old_rb );
      break;
    case PISA_CS_HAND:
    case PISA_CS_TOP:
    case PISA_CS_BOTTOM:
    case PISA_CS_LEFT:
    case PISA_CS_RIGHT:
    case PISA_CS_LEFTTOP:
    case PISA_CS_LEFTBOTTOM:
    case PISA_CS_RIGHTTOP:
    case PISA_CS_RIGHTBOTTOM:
      m_pt_old_lt = pt_lefttop;
      m_pt_old_rb = pt_rightbottom;
      break;
    }

  return FALSE;
}

/*--------------------------------------------------------------*/
gint preview_window::mouse_move ( GdkEvent * event )
{
  int	x, y;
  GdkModifierType	state;
  _pointL pt_mouse, pt_move, pt_tmp_lt, pt_tmp_rb, pt_min_L;
  _pointD pt_lefttop, pt_rightbottom, pt_min_D, pt_zero_D;

  if ( event->motion.is_hint )
    {
      ::gdk_window_get_pointer ( m_prev->window, & x, & y, & state );
    }
  else
    return TRUE;

  if ( m_drag == 0 )
    {
      set_mouse_cursor ( x, y );
      change_cursor ( );
      return FALSE;
    }

  if ( x < 0 ) x = 0;
  if ( y < 0 ) y = 0;
  if ( m_client_rect.right  < x ) x = m_client_rect.right;
  if ( m_client_rect.bottom < y ) y = m_client_rect.bottom;

  pt_mouse.x	= x;
  pt_mouse.y	= y;

  // clear previous marquee
  draw_rect ( m_pt_old_lt, m_pt_old_rb );

  pt_lefttop		= m_pt_save_offset;
  pt_rightbottom	= m_pt_save_offset + m_pt_save_area;

  pt_tmp_lt	= inches2clientpix ( pt_lefttop );
  pt_tmp_rb	= inches2clientpix ( pt_rightbottom );

  pt_zero_D.x	= 0.0;
  pt_zero_D.y	= 0.0;
  pt_min_D.x	= g_min_marq_size;
  pt_min_D.y	= g_min_marq_size;
  pt_min_L	= inches2clientpix ( pt_min_D ) - inches2clientpix ( pt_zero_D );

  pt_move = pt_mouse - m_pt_begin;

  // initialize
  begin_mouse_move ( & pt_tmp_lt, & pt_tmp_rb );

  move_rect ( & pt_tmp_lt, & pt_tmp_rb, pt_move );

  switch ( m_cursor_state )
    {
    case PISA_CS_CROSS:
      m_pt_old_rb = pt_mouse;
      break;
    case PISA_CS_HAND:
      m_pt_old_rb = pt_tmp_lt + ( m_pt_old_rb - m_pt_old_lt );
      m_pt_old_lt = pt_tmp_lt;
      break;
    case PISA_CS_TOP:
      if ( m_pt_old_rb.y - pt_min_L.y > pt_tmp_lt.y )
	m_pt_old_lt.y = pt_tmp_lt.y;
      else
	m_pt_old_lt.y = m_pt_old_rb.y - pt_min_L.y;
      break;
    case PISA_CS_BOTTOM:
      if ( m_pt_old_lt.y + pt_min_L.y  < pt_tmp_rb.y )
	m_pt_old_rb.y = pt_tmp_rb.y;
      else
	m_pt_old_rb.y = m_pt_old_lt.y + pt_min_L.y;
      break;
    case PISA_CS_LEFT:
      if ( m_pt_old_rb.x - pt_min_L.x > pt_tmp_lt.x )
	m_pt_old_lt.x = pt_tmp_lt.x;
      else
	m_pt_old_lt.x = m_pt_old_rb.x - pt_min_L.x;
      break;
    case PISA_CS_RIGHT:
      if ( m_pt_old_lt.x + pt_min_L.x < pt_tmp_rb.x )
	m_pt_old_rb.x = pt_tmp_rb.x;
      else
	m_pt_old_rb.x = m_pt_old_lt.x + pt_min_L.x;
      break;
    case PISA_CS_LEFTTOP:
      if ( m_pt_old_rb.y - pt_min_L.y > pt_tmp_lt.y )
	m_pt_old_lt.y = pt_tmp_lt.y;
      else
	m_pt_old_lt.y = m_pt_old_rb.y - pt_min_L.y;
      if ( m_pt_old_rb.x - pt_min_L.x > pt_tmp_lt.x )
	m_pt_old_lt.x = pt_tmp_lt.x;
      else
	m_pt_old_lt.x = m_pt_old_rb.x - pt_min_L.x;
      break;
    case PISA_CS_LEFTBOTTOM:
      if ( m_pt_old_rb.x - pt_min_L.x > pt_tmp_lt.x )
	m_pt_old_lt.x = pt_tmp_lt.x;
      else
	m_pt_old_lt.x = m_pt_old_rb.x - pt_min_L.x;
      if ( m_pt_old_lt.y + pt_min_L.y  < pt_tmp_rb.y )
	m_pt_old_rb.y = pt_tmp_rb.y;
      else
	m_pt_old_rb.y = m_pt_old_lt.y + pt_min_L.y;
      break;
    case PISA_CS_RIGHTTOP:
      if ( m_pt_old_rb.y - pt_min_L.y > pt_tmp_lt.y )
	m_pt_old_lt.y = pt_tmp_lt.y;
      else
	m_pt_old_lt.y = m_pt_old_rb.y - pt_min_L.y;
      if ( m_pt_old_lt.x + pt_min_L.x < pt_tmp_rb.x )
	m_pt_old_rb.x = pt_tmp_rb.x;
      else
	m_pt_old_rb.x = m_pt_old_lt.x + pt_min_L.x;
      break;
    case PISA_CS_RIGHTBOTTOM:
      if ( m_pt_old_lt.y + pt_min_L.y  < pt_tmp_rb.y )
	m_pt_old_rb.y = pt_tmp_rb.y;
      else
	m_pt_old_rb.y = m_pt_old_lt.y + pt_min_L.y;
      if ( m_pt_old_lt.x + pt_min_L.x < pt_tmp_rb.x )
	m_pt_old_rb.x = pt_tmp_rb.x;
      else
	m_pt_old_rb.x = m_pt_old_lt.x + pt_min_L.x;
      break;
    }

  draw_rect ( m_pt_old_lt, m_pt_old_rb );

  if ( m_cursor_state != PISA_CS_HAND )
    resize_marquee ( m_pt_old_lt, m_pt_old_rb );

  m_pt_old = pt_mouse;

  return FALSE;  
}

/*--------------------------------------------------------------*/
gint preview_window::mouse_up ( GdkEvent * event )
{
  int min_check;
  _pointL pt_mouse;
  long	marq_num;

  if ( 0 == m_drag )
    return FALSE;

  pt_mouse.x = ( int ) event->button.x;
  pt_mouse.y = ( int ) event->button.y;

  // clear previous marquee
  marq_num = g_view_manager->get_marquee_size ();
  if ( 1 < marq_num )
    draw_rect ( m_pt_old_lt, m_pt_old_rb );

  switch ( m_cursor_state )
    {
    case PISA_CS_CROSS:
      min_check = check_min_size ( m_pt_old_lt, m_pt_old_rb );
      if ( min_check == 0 )
	delete_marquee ( );
      update_img ( );
      break;
    case PISA_CS_ARROW:      
      break;
    case PISA_CS_HAND:
      move_marquee ( m_pt_old_lt, m_pt_old_rb );
      update_img ( );
      break;
    case PISA_CS_LEFT:
    case PISA_CS_LEFTTOP:
    case PISA_CS_LEFTBOTTOM:
    case PISA_CS_RIGHT:
    case PISA_CS_RIGHTTOP:
    case PISA_CS_RIGHTBOTTOM:
    case PISA_CS_TOP:
    case PISA_CS_BOTTOM:
      if ( 0 == check_min_size ( m_pt_old_lt, m_pt_old_rb ) )
	delete_marquee ( );
      update_img (   PISA_CS_LEFT       == m_cursor_state
		  || PISA_CS_LEFTTOP    == m_cursor_state
		  || PISA_CS_LEFTBOTTOM == m_cursor_state);
      break;
    }

  m_drag = 0;

  set_mouse_cursor ( pt_mouse.x, pt_mouse.y );
  change_cursor ( );
  
  return FALSE;
}

/*--------------------------------------------------------------*/
int preview_window::create_marquee ( const _pointL & pt_lt, const _pointL & pt_rb )
{
  _pointL	pt_lefttop_L, pt_rightbottom_L;
  _pointD	pt_lefttop_D, pt_rightbottom_D, pt_area_D;
  marquee	* new_marq, * whole_marq;

  pt_lefttop_L		= pt_lt;
  pt_rightbottom_L	= pt_rb;
  check_ltrb ( & pt_lefttop_L, & pt_rightbottom_L );

  pt_lefttop_D		= clientpix2inches ( pt_lefttop_L );
  pt_rightbottom_D	= clientpix2inches ( pt_rightbottom_L );
  pt_area_D		= pt_rightbottom_D - pt_lefttop_D;

  check_max_size ( & pt_lefttop_D, & pt_area_D, 0 );

  whole_marq = & g_view_manager->get_marquee (0);
  new_marq = new marquee;
  * new_marq = * whole_marq;

  new_marq->offset	= pt_lefttop_D;
  new_marq->area	= pt_area_D;

  g_view_manager->add_marquee (&new_marq);

  ::g_view_manager->sensitive ( );

  return 1;
}

/*--------------------------------------------------------------*/
int preview_window::delete_marquee ( void )
{
  long		num_marq;
  int		i, j;
  marquee	* cur_marq, * whole_marq;

  num_marq = g_view_manager->get_marquee_size ();

  // no marquee
  if ( num_marq < 2 )
    return PISA_ERR_SUCCESS;

  whole_marq = & g_view_manager->get_marquee (0);
  cur_marq = & g_view_manager->get_marquee ();

  whole_marq->gamma		= cur_marq->gamma;
  whole_marq->highlight		= cur_marq->highlight;
  whole_marq->shadow		= cur_marq->shadow;
  whole_marq->threshold		= cur_marq->threshold;

  for ( i = 0; i < 4; i++ )
    for ( j = 0; j < 256; j++ )
      whole_marq->gamma_table [ i ] [ j ] = cur_marq->gamma_table [ i ] [ j ];

  whole_marq->graybalance	= cur_marq->graybalance;
  whole_marq->saturation	= cur_marq->saturation;

  whole_marq->scale		= cur_marq->scale;

  whole_marq->focus		= cur_marq->focus;

  for ( i = 0; i < 3; i++ )
    {
      whole_marq->film_gamma [ i ]	= cur_marq->film_gamma [ i ];
      whole_marq->film_yp [ i ]		= cur_marq->film_yp [ i ];
      whole_marq->grayl [ i ]		= cur_marq->grayl [ i ];
    }

  for ( i = 0; i < 256; i++ )
    {
      whole_marq->lut.gamma_r [ i ]	= cur_marq->lut.gamma_r [ i ];
      whole_marq->lut.gamma_g [ i ]	= cur_marq->lut.gamma_g [ i ];
      whole_marq->lut.gamma_b [ i ]	= cur_marq->lut.gamma_b [ i ];
    }

  g_view_manager->del_marquee ();

  ::g_view_manager->sensitive ( );
  update_img ( );

  return PISA_ERR_SUCCESS;
}

/*--------------------------------------------------------------*/
void preview_window::move_marquee ( const _pointL & pt_lt,
				    const _pointL & pt_rb )
{
  _pointL pt_lefttop_L, pt_rightbottom_L, pt_tmp;
  _pointD pt_lefttop_D, pt_rightbottom_D, pt_offset, pt_area;
  marquee	* cur_marq;

  pt_tmp = pt_rb;

  pt_lefttop_L		= pt_lt;
  pt_lefttop_D		= clientpix2inches ( pt_lefttop_L );
  pt_rightbottom_D	= clientpix2inches ( pt_rightbottom_L );

  pt_offset	= pt_lefttop_D;
  pt_area	= pt_rightbottom_D - pt_lefttop_D;

  check_max_size ( & pt_offset, & pt_area, 1 );

  cur_marq = & g_view_manager->get_marquee ();

  cur_marq->offset	= pt_offset;

  ::g_view_manager->sensitive ( );
}

/*--------------------------------------------------------------*/
void preview_window::resize_marquee ( const _pointL & pt_lt,
				      const _pointL & pt_rb )
{
  _pointL pt_lefttop_L, pt_rightbottom_L;
  _pointD pt_lefttop_D, pt_rightbottom_D, pt_offset, pt_area;
  marquee	* cur_marq;

  pt_lefttop_L		= pt_lt;
  pt_rightbottom_L	= pt_rb;
  
  check_ltrb ( & pt_lefttop_L, & pt_rightbottom_L );
  
  pt_lefttop_D		= clientpix2inches ( pt_lefttop_L );
  pt_rightbottom_D	= clientpix2inches ( pt_rightbottom_L );
 
  pt_offset	= pt_lefttop_D;
  pt_area	= pt_rightbottom_D - pt_lefttop_D;

  check_max_size ( & pt_offset, & pt_area, 0 );

  cur_marq = & g_view_manager->get_marquee ();

  cur_marq->offset	= pt_offset;
  cur_marq->area	= pt_area;

  ::g_view_manager->sensitive ( );
}

/*--------------------------------------------------------------*/
void preview_window::begin_mouse_move ( _pointL * pt_lt, _pointL * pt_rb )
{
  switch ( m_cursor_state )
    {
    case PISA_CS_CROSS:
    case PISA_CS_HAND:
      break;
    case PISA_CS_TOP:
      pt_lt->x = pt_rb->x;
      pt_rb->y = pt_lt->y;
      break;
    case PISA_CS_BOTTOM:
      * pt_lt = * pt_rb;
      break;
    case PISA_CS_LEFT:
      pt_lt->y = pt_rb->y;
      pt_rb->x = pt_lt->x;
      break;
    case PISA_CS_RIGHT:
      * pt_lt = * pt_rb;
      break;
    case PISA_CS_LEFTTOP:
      * pt_rb = * pt_lt;
      break;
    case PISA_CS_LEFTBOTTOM:
      pt_lt->y = pt_rb->y;
      pt_rb->x = pt_lt->x;
      break;
    case PISA_CS_RIGHTBOTTOM:
      * pt_lt = * pt_rb;
      break;
    case PISA_CS_RIGHTTOP:
      pt_lt->x = pt_rb->x;
      pt_rb->y = pt_lt->y;
      break;
    }
}

/*--------------------------------------------------------------*/
void preview_window::move_rect ( _pointL * pt_lt, _pointL * pt_rb,
				 const _pointL & pt_move )
{
  // move direction
  typedef enum { DIR_LT, DIR_LB, DIR_RT, DIR_RB } MOVE_DIR;
  MOVE_DIR direction;
  _pointL pt_tmp, pt_tmp_move ( pt_move );
  _pointL pt_size;
  _pointD pt_lefttop_D, pt_rightbottom_D;
  _pointL pt_lefttop_L, pt_rightbottom_L;

  if ( pt_move.x < 0 )
    {
      if ( pt_move.y < 0 )
	direction = DIR_LT;
      else
	direction = DIR_LB;
    }
  else
    {
      if ( pt_move.y < 0 )
	direction = DIR_RT;
      else
	direction = DIR_RB;
    }

  pt_lefttop_D		= m_pt_save_offset;
  pt_rightbottom_D	= m_pt_save_offset + m_pt_save_area;

  pt_lefttop_L		= inches2clientpix ( pt_lefttop_D );
  pt_rightbottom_L	= inches2clientpix ( pt_rightbottom_D );

  pt_size.x	= pt_rightbottom_L.x - pt_lefttop_L.x + 1;
  pt_size.y	= pt_rightbottom_L.y - pt_lefttop_L.y + 1;

  switch ( direction )
    {
    case DIR_LT:
      pt_tmp.x = pt_lt->x + pt_tmp_move.x;
      if ( pt_tmp.x < m_pt_max_lt.x )
	pt_tmp.x = m_pt_max_lt.x;
      pt_tmp.y = pt_lt->y + pt_tmp_move.y;
      if ( pt_tmp.y < m_pt_max_lt.y )
	pt_tmp.y = m_pt_max_lt.y;
      * pt_rb = pt_tmp + ( * pt_rb - * pt_lt );
      * pt_lt = pt_tmp;
      break;
    case DIR_LB:
      pt_tmp.x = pt_lt->x + pt_tmp_move.x;
      if ( pt_tmp.x < m_pt_max_lt.x )
	pt_tmp.x = m_pt_max_lt.x;
      pt_tmp.y = pt_rb->y + pt_tmp_move.y;
      if ( m_pt_max_rb.y < pt_tmp.y )
	pt_tmp.y = m_pt_max_rb.y;
      pt_lt->y = pt_tmp.y - ( pt_rb->y - pt_lt->y );
      pt_rb->x = pt_tmp.x - ( pt_rb->x - pt_lt->x );
      pt_lt->x = pt_tmp.x;
      pt_rb->y = pt_tmp.y;
      break;
    case DIR_RT:
      pt_tmp.x = pt_rb->x + pt_tmp_move.x;
      if ( m_pt_max_rb.x < pt_tmp.x )
	pt_tmp.x = m_pt_max_rb.x;
      pt_tmp.y = pt_lt->y + pt_tmp_move.y;
      if ( pt_tmp.y < m_pt_max_lt.y )
	pt_tmp.y = m_pt_max_lt.y;
      pt_lt->x = pt_tmp.x - ( pt_rb->x - pt_lt->x );
      pt_rb->y = pt_tmp.y + ( pt_rb->y - pt_lt->y );
      pt_lt->y = pt_tmp.y;
      pt_rb->x = pt_tmp.x;
      break;
    case DIR_RB:
      pt_tmp.x = pt_rb->x + pt_tmp_move.x;
      if ( m_pt_max_rb.x < pt_tmp.x )
	pt_tmp.x = m_pt_max_rb.x;
      pt_tmp.y = pt_rb->y + pt_tmp_move.y;
      if ( m_pt_max_rb.y < pt_tmp.y )
	pt_tmp.y = m_pt_max_rb.y;
      * pt_lt = pt_tmp - ( * pt_rb - * pt_lt );
      * pt_rb = pt_tmp;
      break;
    }
}

/*--------------------------------------------------------------*/
_pointD preview_window::clientpix2inches ( _pointL & pt )
{
  _pointD	pt_result;
  _pointD	pt_tmp;
  double	da, db;

  da	= m_client_rect.right - m_client_rect.left;
  db	= m_img_rect.right - m_img_rect.left;
  pt_tmp.x = similarity ( ( double ) pt.x, da, db );

  da	= m_client_rect.bottom - m_client_rect.top;
  db	= m_img_rect.bottom - m_img_rect.top;
  pt_tmp.y = similarity ( ( double ) pt.y, da, db );


  pt_result.x = m_img_rect.left + pt_tmp.x;
  pt_result.y = m_img_rect.top  + pt_tmp.y;

  return pt_result;
}

/*--------------------------------------------------------------*/
_pointL preview_window::inches2clientpix ( _pointD & pt )
{
  _pointD	pt_tmp;
  _pointL	pt_result;
  double	da, db;

  pt_tmp.x = pt.x - m_img_rect.left;
  pt_tmp.y = pt.y - m_img_rect.top;

  da	= m_img_rect.right - m_img_rect.left;
  db	= m_client_rect.right - m_client_rect.left;
  pt_result.x = ( long ) similarity ( pt_tmp.x, da, db );

  da	= m_img_rect.bottom - m_img_rect.top;
  db	= m_client_rect.bottom - m_client_rect.top;
  pt_result.y = ( long ) similarity ( pt_tmp.y, da, db );

  return pt_result;
}

/*--------------------------------------------------------------*/
int preview_window::get_marquee_point ( long i, _pointL * pt_lt, _pointL * pt_rb )
{
  marquee * marq;
  _pointD pt_tmp_lt, pt_tmp_rb;

  if (0 == (marq = & g_view_manager->get_marquee (i)))
    return 0;

  pt_tmp_lt = marq->offset;
  pt_tmp_rb = marq->offset + marq->area;

  * pt_lt = inches2clientpix ( pt_tmp_lt );
  * pt_rb = inches2clientpix ( pt_tmp_rb );

  return 1;
}

/*--------------------------------------------------------------*/
int preview_window::check_min_size ( const _pointL & pt_lt, const _pointL & pt_rb )
{
  _pointD pt_zero_D, pt_min_D;
  _pointL pt_min_L;

  pt_zero_D.x	= 0.0;
  pt_zero_D.y	= 0.0;
  pt_min_D.x	= g_min_marq_size;
  pt_min_D.y	= g_min_marq_size;
  pt_min_L	= inches2clientpix ( pt_min_D ) - inches2clientpix ( pt_zero_D );

  if ( ::abs ( pt_lt.x - pt_rb.x ) < pt_min_L.x ||
       ::abs ( pt_lt.y - pt_rb.y ) < pt_min_L.y )
    return 0;
  else
    return 1;
}

/*--------------------------------------------------------------*/
void preview_window::check_max_size ( _pointD * pt_offset, _pointD * pt_area,
				      int offset )
{
  if ( pt_offset->x < 0.0 ) pt_offset->x = 0.0;
  if ( pt_offset->y < 0.0 ) pt_offset->y = 0.0;

  if ( pt_area->x > m_max_img_rect.right  ) pt_area->x = m_max_img_rect.right;
  if ( pt_area->y > m_max_img_rect.bottom ) pt_area->y = m_max_img_rect.bottom;

  if ( offset )
    {
      if ( m_max_img_rect.right < pt_offset->x + pt_area->x )
	pt_area->x = m_max_img_rect.right - pt_offset->x;
      if ( m_max_img_rect.bottom < pt_offset->y + pt_area->y )
	pt_area->y = m_max_img_rect.bottom - pt_offset->y;
    }
  else
    {
      if ( m_max_img_rect.right < pt_offset->x + pt_area->x )
	pt_offset->x = m_max_img_rect.right - pt_area->x;
      if ( m_max_img_rect.bottom < pt_offset->y + pt_area->y )
	pt_offset->y = m_max_img_rect.bottom - pt_area->y;
    }
}

/*--------------------------------------------------------------*/
void preview_window::check_ltrb ( _pointL * pt_lt, _pointL * pt_rb )
{
  _pointL pt_ret_lt, pt_ret_rb;

  pt_ret_lt.x = ( pt_lt->x < pt_rb->x ) ? pt_lt->x : pt_rb->x;
  pt_ret_lt.y = ( pt_lt->y < pt_rb->y ) ? pt_lt->y : pt_rb->y;
  pt_ret_rb.x = ( pt_lt->x > pt_rb->x ) ? pt_lt->x : pt_rb->x;
  pt_ret_rb.y = ( pt_lt->y > pt_rb->y ) ? pt_lt->y : pt_rb->y;

  * pt_lt = pt_ret_lt;
  * pt_rb = pt_ret_rb;
}

/*--------------------------------------------------------------*/
void preview_window::draw_rect ( const _pointL & pt_lt,
				 const _pointL & pt_rb )
{
  int x, y, w, h;

  if ( m_gc == 0 )
    {
      m_gc = ::gdk_gc_new ( m_prev->window );
      ::gdk_gc_set_function ( m_gc, GDK_INVERT );
      ::gdk_gc_set_line_attributes ( m_gc, 1, GDK_LINE_ON_OFF_DASH,
				     GDK_CAP_BUTT, GDK_JOIN_MITER );
    }

  x = ( pt_lt.x < pt_rb.x ) ? pt_lt.x : pt_rb.x;
  y = ( pt_lt.y < pt_rb.y ) ? pt_lt.y : pt_rb.y;
  w = ( pt_lt.x > pt_rb.x ) ? pt_lt.x - pt_rb.x : pt_rb.x - pt_lt.x;
  h = ( pt_lt.y > pt_rb.y ) ? pt_lt.y - pt_rb.y : pt_rb.y - pt_lt.y;

  ::gdk_draw_rectangle ( m_prev->window, m_gc, FALSE,
			 x, y, w, h );
}


