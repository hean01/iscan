/* 
   SANE EPSON backend
   Copyright (C) 2001, 2005, 2008, 2009  SEIKO EPSON CORPORATION

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
#define  _(msg_id)      gettext (msg_id)
#define N_(msg_id)              (msg_id)

/*------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <cstdlib>

/*------------------------------------------------------------*/
#include "pisa_main_window.h"
#include "pisa_error.h"
#include "pisa_tool.h"
#include "xpm_data.h"
#include "pisa_view_manager.h"
#include "pisa_change_unit.h"
#include "pisa_default_val.h"
#include "pisa_scan_selector.h"
#include "pisa_aleart_dialog.h"
#include "../lib/pngstream.hh"

#if !GTK_CHECK_VERSION (2,12,0)
#define gtk_widget_set_tooltip_text(widget,text)
#endif

/*------------------------------------------------------------*/
static gint delete_event ( GtkWidget * widget, gpointer data );
static void click_preview_btn ( void );
static void click_zoom_btn ( void );
static void click_expose_btn ( void );
static void click_scan_btn ( void );
static void click_scan (GtkWidget *widget, gpointer data);
static void click_cleaning ( GtkWidget * widget, gpointer data );
static void click_calibration ( GtkWidget * widget, gpointer data );
static void click_close ( void );

static void change_destination ( GtkWidget * widget, gpointer data );
static void change_docsrc ( GtkWidget * widget, gpointer data );
static void change_imgtype ( GtkWidget * widget, gpointer data );
static void change_resolution ( GtkWidget * widget, gpointer data );
static void change_unit ( GtkWidget * widget, gpointer data );
static void value_changed ( GtkAdjustment * adjust, gpointer data );
static void toggled_start_button (GtkWidget *widget, gpointer data);
static void toggled_draft_mode (GtkWidget *widget, gpointer data);
static void toggled_size_check (GtkWidget *widget, gpointer data);
static void toggled_deskew (GtkWidget *widget, gpointer data);
static void change_dfd (GtkWidget *widget, gpointer data);
static void toggled_usm ( GtkWidget * widget, gpointer data );
static void check_focus ( GtkWidget * widget, gpointer data );
static void switch_window_waiting (GtkWidget *widget, gboolean is_waiting, GdkCursor **cursor);

/*------------------------------------------------------------*/
// destination
#define ID_MENU_FILE	0x0001
#define ID_MENU_PRINT	0x0002
menu_items g_destination_menu_item [ ] =
{
  { ID_MENU_FILE,  0, N_("File"),    ( GtkSignalFunc * ) change_destination },
  { ID_MENU_PRINT, 0, N_("Printer"), ( GtkSignalFunc * ) change_destination },
  { 0,             0, "",                                             0 }
};

/*------------------------------------------------------------*/
// document source
#define ID_MENU_FLATBED	0x0001
#define ID_MENU_TPU_NEG	0x0002
#define ID_MENU_TPU_POS 0x0003
#define ID_MENU_ADF_SIM 0x0004
#define ID_MENU_ADF_DUP 0x0005
menu_items g_docsrc_menu_item [ ] =
{
  { ID_MENU_FLATBED, 0, N_("Flatbed"), ( GtkSignalFunc * ) change_docsrc },
  { ID_MENU_TPU_NEG, 0, N_("TPU - Negative film"), ( GtkSignalFunc * ) change_docsrc },
  { ID_MENU_TPU_POS, 0, N_("TPU - Positive film"), ( GtkSignalFunc * ) change_docsrc },
  { ID_MENU_ADF_SIM, 0, N_("ADF - Single-sided"), ( GtkSignalFunc * ) change_docsrc },
  { ID_MENU_ADF_DUP, 0, N_("ADF - Double-sided"), ( GtkSignalFunc * ) change_docsrc },
  { 0,               0, "",                                        0 }
};

/*------------------------------------------------------------*/
// image type
#define ID_MENU_24C_PHOTO	0x0001
#define ID_MENU_24C_DOC		0x0002
#define ID_MENU_8G_PHOTO	0x0003
#define ID_MENU_8G_DOC		0x0004
#define ID_MENU_LINEART		0x0005
menu_items g_imgtype_menu_item [ ] =
{
  { ID_MENU_24C_PHOTO, 0, N_("Color Photo"),            ( GtkSignalFunc * ) change_imgtype },
  { ID_MENU_24C_DOC,   0, N_("Color Document"),         ( GtkSignalFunc * ) change_imgtype },
  { ID_MENU_8G_PHOTO,  0, N_("Black & White Photo"),    ( GtkSignalFunc * ) change_imgtype },
  { ID_MENU_8G_DOC,    0, N_("Black & White Document"), ( GtkSignalFunc * ) change_imgtype },
  { ID_MENU_LINEART,   0, N_("Line Art"),               ( GtkSignalFunc * ) change_imgtype },
  { 0,                 0, "",                                                     0 }
};

imagetype g_imagetype_list [ ] =
{
  { ID_MENU_24C_PHOTO, PISA_PT_RGB,  PISA_BD_8, PISA_DESCREEN_OFF, PISA_AE_PHOTO,  PISA_DO_NONE, PISA_MO_NONE, PISA_HT_NONE },
  { ID_MENU_24C_DOC,   PISA_PT_RGB,  PISA_BD_8, PISA_DESCREEN_ON,  PISA_AE_DOC,    PISA_DO_NONE, PISA_MO_NONE, PISA_HT_NONE },
  { ID_MENU_8G_PHOTO,  PISA_PT_GRAY, PISA_BD_8, PISA_DESCREEN_OFF, PISA_AE_PHOTO,  PISA_DO_NONE, PISA_MO_NONE, PISA_HT_NONE },
  { ID_MENU_8G_DOC,    PISA_PT_GRAY, PISA_BD_8, PISA_DESCREEN_ON,  PISA_AE_DOC,    PISA_DO_NONE, PISA_MO_NONE, PISA_HT_NONE },
  { ID_MENU_LINEART,   PISA_PT_BW,   PISA_BD_1, PISA_DESCREEN_OFF, PISA_AE_GRAYED, PISA_DO_NONE, PISA_MO_NONE, PISA_HT_NONE }
};

/*------------------------------------------------------------*/
// resolution
menu_items g_resolution_menu_item [ ] =
{
  { 50,   0, "50dpi",   ( GtkSignalFunc * ) change_resolution },
  { 72,   0, "72dpi",   ( GtkSignalFunc * ) change_resolution },
  { 96,   0, "96dpi",   ( GtkSignalFunc * ) change_resolution },
  { 150,  0, "150dpi",  ( GtkSignalFunc * ) change_resolution },
  { 200,  0, "200dpi",  ( GtkSignalFunc * ) change_resolution },
  { 240,  0, "240dpi",  ( GtkSignalFunc * ) change_resolution },
  { 300,  0, "300dpi",  ( GtkSignalFunc * ) change_resolution },
  { 360,  0, "360dpi",  ( GtkSignalFunc * ) change_resolution },
  { 400,  0, "400dpi",  ( GtkSignalFunc * ) change_resolution },
  { 600,  0, "600dpi",  ( GtkSignalFunc * ) change_resolution },
  { 720,  0, "720dpi",  ( GtkSignalFunc * ) change_resolution },
  { 800,  0, "800dpi",  ( GtkSignalFunc * ) change_resolution },
  { 1200, 0, "1200dpi", ( GtkSignalFunc * ) change_resolution },
  { 1600, 0, "1600dpi", ( GtkSignalFunc * ) change_resolution },
  { 2400, 0, "2400dpi", ( GtkSignalFunc * ) change_resolution },
  { 3200, 0, "3200dpi", ( GtkSignalFunc * ) change_resolution },
  { 4800, 0, "4800dpi", ( GtkSignalFunc * ) change_resolution },
  { 6400, 0, "6400dpi", ( GtkSignalFunc * ) change_resolution },
  { 9600, 0, "9600dpi", ( GtkSignalFunc * ) change_resolution },
  {12800, 0,"12800dpi", ( GtkSignalFunc * ) change_resolution },
  { 0,    0, "",                                           0  }
};

/*------------------------------------------------------------*/
// unit
menu_items g_unit_menu_item [ ] =
{
  { PISA_UNIT_INCHES, 0, N_("inches"), ( GtkSignalFunc * ) change_unit },
  { PISA_UNIT_PIXELS, 0, N_("pixels"), ( GtkSignalFunc * ) change_unit },
  { PISA_UNIT_CM ,    0, N_("cm"),     ( GtkSignalFunc * ) change_unit },
  { 0,                0, "",                                     0 }
};

/*------------------------------------------------------------*/
// scale
#define ID_SCALE_ZOOM	0x0001

scale_items g_scale_zoom = { ID_SCALE_ZOOM, 0, 0,
			     ( GtkSignalFunc * ) value_changed,
			     100, 50, 200, 1, 10 };

/*------------------------------------------------------------*/
// focus

long	g_focus [ ] = { 0, 25 };

/*------------------------------------------------------------*/
// double feed detection
menu_items g_double_feed_detection[] =
{
  { PISA_DFD_OFF,  0, N_("Off"),      (GtkSignalFunc *) change_dfd },
  { PISA_DFD_STD,  0, N_("Standard"), (GtkSignalFunc *) change_dfd },
  { PISA_DFD_THIN, 0, N_("Thin"),     (GtkSignalFunc *) change_dfd },
  { 0,             0, "",                               0 }
};

static void
change_dfd (GtkWidget *widget, gpointer data)
{
  scan_manager *sm = g_view_manager->get_scan_manager ();
  if (sm && sm->has_dfd ()) sm->set_dfd (*(long *) data);
}

/*------------------------------------------------------------*/
static gint delete_event ( GtkWidget * widget, gpointer data )
{
  widget = widget;
  data = data;

  ::g_view_manager->close_window ( ID_WINDOW_MAIN, 0 );

  return FALSE;
}

/*------------------------------------------------------------*/
void main_window::start_preview ( bool zooming )
{
  preview_window	* prev_cls;
  main_window           * main_cls;

  prev_cls = ( preview_window * ) ::g_view_manager->get_window_cls ( ID_WINDOW_PREV );

  main_cls = ( main_window * ) ::g_view_manager->get_window_cls ( ID_WINDOW_MAIN );

  scan_manager *sm = g_view_manager->get_scan_manager ();

  if (sm->has_preview ()) sm->set_preview (true);
  main_cls->do_sensitive_size_check (false);

  if (zooming)
    {
      prev_cls->start_zoom ();
    }
  else
    {
      prev_cls->start_preview ();
    }

  main_cls->do_sensitive_size_check (true);
  if (sm->has_preview ()) sm->set_preview (false);
}

/*------------------------------------------------------------*/
static void click_preview_btn ( void )
{
  main_window *main_cls = ( main_window * ) ::g_view_manager->get_window_cls ( ID_WINDOW_MAIN );
  main_cls->start_preview (false);
}

/*------------------------------------------------------------*/
static void click_zoom_btn ( void )
{
  main_window *main_cls = ( main_window * ) ::g_view_manager->get_window_cls ( ID_WINDOW_MAIN );
  main_cls->start_preview (true);
}

/*------------------------------------------------------------*/
static void click_expose_btn ( void )
{
  preview_window	* prev_cls;

  prev_cls = ( preview_window * ) ::g_view_manager->get_window_cls ( ID_WINDOW_PREV );

  prev_cls->auto_exposure ( );
  prev_cls->update_img ( );
}

/*------------------------------------------------------------*/
static void click_scan_btn ( void )
{
  ::g_view_manager->start_scan ( );
}

static void
click_scan (GtkWidget *destination, gpointer data)
{
  main_window *mw = static_cast <main_window *> (data);
  mw->start_scan (destination);
}

static void
click_calibration ( GtkWidget * widget, gpointer data )
{
  scan_manager *scan_mgr = static_cast< scan_manager * > (data);
  GdkCursor *cur = NULL;

  if (scan_mgr->has_calibrate ())
  {
    switch_window_waiting (widget, true, &cur);

    try
    {
      scan_mgr->calibrate ();
    }
    catch (pisa_error & err)
    {
      aleart_dialog aleart_dlg;
      aleart_dlg.message_box (0, _("Calibration is failed."));
    }

    switch_window_waiting (widget, false, &cur);
  }

  return;
}

static void
click_cleaning ( GtkWidget * widget, gpointer data )
{
  scan_manager *scan_mgr = static_cast< scan_manager * > (data);
  GdkCursor *cur = NULL;

  if (scan_mgr->has_clean ())
  {
    switch_window_waiting (widget, true, &cur);

    try
    {
      scan_mgr->clean ();
    }
    catch (pisa_error & err)
    {
      aleart_dialog aleart_dlg;
      aleart_dlg.message_box (0, _("Cleaning is failed."));
    }

    switch_window_waiting (widget, false, &cur);
  }

  return;
}

/*------------------------------------------------------------*/
static void click_close ( void )
{
  ::g_view_manager->close_window ( ID_WINDOW_MAIN, 1 );
}

/*------------------------------------------------------------*/
static void click_config ( void )
{
  ::g_view_manager->create_window ( ID_WINDOW_CONFIG );
}

/*------------------------------------------------------------*/
static void change_destination ( GtkWidget * widget, gpointer data )
{
  char	dest;
  long	id;

  widget = widget;
  id = * ( long * ) data;

  switch ( id )
    {
    case ID_MENU_FILE:
      dest	= PISA_DE_FILE;
      break;

    case ID_MENU_PRINT:
      dest	= PISA_DE_PRINTER;
      break;

    default:
      return;
    }

  g_view_manager->set_destination (dest);
}

/*------------------------------------------------------------*/
static void change_docsrc ( GtkWidget * widget, gpointer data )
{
  widget = widget;

  // ASSUMPTION: flatbed is the default at program startup
  static long current_id = ID_MENU_FLATBED;

  long id = *(long *) data;
  if (current_id == id) return;         // no change

  scan_manager *sm = g_view_manager->get_scan_manager ();

  switch (id)
    {
    case ID_MENU_FLATBED:
      sm->select_flatbed ();
      break;
    case ID_MENU_TPU_NEG:
    case ID_MENU_TPU_POS:
      sm->select_tpu (ID_MENU_TPU_POS == id);
      break;
    case ID_MENU_ADF_SIM:
    case ID_MENU_ADF_DUP:
      sm->select_adf (ID_MENU_ADF_DUP == id);
      break;
    default:
      return;
    }
  current_id = id;

  if (!g_view_manager->change_document_source (g_imagetype_list))
    {                           // reset drop down menu to default
      gtk_menu_set_active
        (GTK_MENU (gtk_widget_get_ancestor (widget, GTK_TYPE_MENU)), 0);
    }

  main_window *mw = (static_cast <main_window *>
                     (g_view_manager->get_window_cls (ID_WINDOW_MAIN)));
  if (mw->get_widget (WG_MAIN_DFD))
    gtk_widget_set_sensitive (mw->get_widget (WG_MAIN_DFD), sm->using_adf ());
}

/*------------------------------------------------------------*/
static void change_imgtype ( GtkWidget * widget, gpointer data )
{
  unsigned	i;
  long		id;

  widget = widget;
  id = * ( long * ) data;

  for ( i = 0; i < sizeof ( ::g_imagetype_list ) /
	  sizeof ( ::g_imagetype_list [ 0 ] ); i++ )
    {
      if ( id == g_imagetype_list [ i ].id )
	{
	  preview_window	* prev_cls;

	  g_view_manager->set_image_type (&g_imagetype_list[i]);
	  g_view_manager->sensitive ();

	  prev_cls = ( preview_window * ) ::g_view_manager->get_window_cls ( ID_WINDOW_PREV );
	  
	  prev_cls->auto_exposure ( );

	  ::g_view_manager->update_lut ( );

	  prev_cls->update_img ( );

	  return;
	}
    }
}

/*------------------------------------------------------------*/
static void change_resolution ( GtkWidget * widget, gpointer data )
{
  widget = widget;

  g_view_manager->set_resolution (* (long *) data);

  ::g_view_manager->sensitive ( );
}

/*------------------------------------------------------------*/
static void change_unit ( GtkWidget * widget, gpointer data )
{
  widget = widget;

  g_view_manager->set_unit (* (long *) data);

  ::g_view_manager->sensitive ( );
}

/*------------------------------------------------------------*/
static void value_changed ( GtkAdjustment * adjust, gpointer data )
{
  long	id;
  long	val;
  int	change;
  marquee	* marq;

  id = * ( long * ) data;
  change = 1;
  marq = & g_view_manager->get_marquee ();

  switch ( id )
    {
    case ID_SCALE_ZOOM:
      val = ( long ) ( adjust->value );
      if ( marq->scale == val )
	change = 0;
      else
	marq->scale = val;
      break;
    }

  if ( change )
    {
      ::g_view_manager->sensitive ( );
    }
}

static void
toggled_start_button (GtkWidget *widget, gpointer data)
{
  data = data;

  g_view_manager->enable_start_button (gtk_toggle_button_get_active
				       (GTK_TOGGLE_BUTTON (widget)));
}

static void
toggled_draft_mode (GtkWidget *widget, gpointer data)
{
  data = data;

  g_view_manager->enable_draft_mode (gtk_toggle_button_get_active
				     (GTK_TOGGLE_BUTTON (widget)));
}

static void
toggled_size_check (GtkWidget *widget, gpointer data)
{
  main_window *mw = (main_window *) data;
  scan_manager *sm = g_view_manager->get_scan_manager ();
  mw->update_zoom_button ();

  gboolean is_active =
    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
  if ( sm->has_size_check () )
    {
      sm->set_size_check (is_active);
    }
  if ( sm->has_autocrop () )
    {
      sm->set_autocrop (is_active);
    }
  mw->sensitive_option();
}

static void
toggled_deskew (GtkWidget *widget, gpointer data)
{
  main_window *mw = (main_window *) data;

  g_view_manager
    ->get_scan_manager()
    ->set_deskew (gtk_toggle_button_get_active
		      (GTK_TOGGLE_BUTTON (widget)));
  mw->sensitive_option();
}

/*------------------------------------------------------------*/
static void toggled_usm ( GtkWidget * widget, gpointer data )
{
  data = data;

  if ( ::gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( widget ) ) )
    g_view_manager->enable_unsharp_mask (true);
  else
    g_view_manager->enable_unsharp_mask (false);

  preview_window *prev_cls =
    (preview_window *) ::g_view_manager->get_window_cls (ID_WINDOW_PREV);

  prev_cls->update_img ( );
}

/*------------------------------------------------------------*/
static void check_focus ( GtkWidget * widget, gpointer data )
{
  long		id;
  marquee	* marq;

  widget = widget;

  marq = & g_view_manager->get_marquee ();
  id = * ( long * ) data;

  if ( id == g_focus [ 1 ] )
    marq->focus = 25;
  else
    marq->focus = 0;
}

/*------------------------------------------------------------*/
static void
switch_window_waiting (GtkWidget *widget, gboolean is_waiting, GdkCursor **cur)
{
  GtkWidget *main_win = ::gtk_widget_get_toplevel (widget);
  GdkWindow *window = main_win->window;

  if (is_waiting)
  {
    ::gtk_widget_set_sensitive (main_win, false);

    *cur = ::gdk_cursor_new (GDK_WATCH);
    ::gdk_window_set_cursor (window, *cur);
  }
  else
  {
    ::gdk_window_set_cursor (window, NULL);
    ::gdk_cursor_unref (*cur);

    ::gtk_widget_set_sensitive (main_win, true);
  }

  while (::gtk_events_pending ())
    ::gtk_main_iteration ();
}

/*------------------------------------------------------------*/
int main_window::init ( void )
{
  unsigned int	i;

  for ( i = 0; i < WG_MAIN_NUM; i++ )
    m_widget [ i ] = 0;

  g_view_manager->set_image_type (&g_imagetype_list[0]);

  _done_preview = false;
  _do_sensitive_size_check = true;

  return PISA_ERR_SUCCESS;
}


/*------------------------------------------------------------*/
GtkWidget * main_window::create_window ( GtkWidget * parent )
{
  GtkWidget	* top;
  GtkWidget	* hbox;
  GtkWidget	* left_area, * right_area;

  parent = parent;

  // determine GUI mode
  _compact_mode = false;
  gint padding = 10;
  GdkScreen * screen = gdk_screen_get_default ();
  if ((screen && 768 > gdk_screen_get_height (screen))
      || getenv ("ISCAN_COMPACT_GUI"))
    {
      padding = 2;
      _compact_mode = true;
      if (getenv ("ISCAN_DEBUG"))
        fprintf (stderr, "Compact GUI mode enabled.\n");
    }
  else
    if (getenv ("ISCAN_DEBUG"))
      {
        if (!screen) fprintf (stderr, "No default screen found. ");
        fprintf (stderr, "Standard GUI mode enabled.\n");
      }


  // main window
  top = m_widget [ WG_MAIN_TOP ] = ::gtk_window_new ( GTK_WINDOW_TOPLEVEL );
  ::gtk_container_border_width ( GTK_CONTAINER ( top ), padding );
  ::gtk_window_set_title ( GTK_WINDOW ( top ), PACKAGE_STRING );
  ::gtk_widget_set_uposition ( top, POS_MAIN_X, POS_MAIN_Y );
  
  ::gtk_signal_connect ( GTK_OBJECT ( top ), "delete_event",
			 GTK_SIGNAL_FUNC ( ::delete_event ), 0 );
  
  hbox = ::gtk_hbox_new ( FALSE, 5 );
  ::gtk_container_add ( GTK_CONTAINER ( top ), hbox );
  ::gtk_widget_show ( hbox );

  // The icons for our buttons rely on the main widget's top level to
  // be realized.  These buttons were created in `create_left_area()`
  // in the past but as we are heavily rearranging parts of the GUI,
  // we'd better make sure it's done early.
  ::gtk_widget_realize ( m_widget [ WG_MAIN_TOP ] );

  create_controls ();

  left_area = create_left_area ( );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), left_area, TRUE, TRUE, 0 );
  ::gtk_widget_show ( left_area );

  right_area = create_right_area ( );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), right_area, FALSE, FALSE, 0 );
  ::gtk_widget_show ( right_area );
  
  ::gtk_widget_show ( top );

  gtk_widget_grab_focus (prev_btn_);

  ::g_view_manager->sensitive ( );

  return top;
}

/*------------------------------------------------------------*/
int main_window::close_window ( int destroy )
{
  ::g_view_manager->close_window ( ID_WINDOW_PREV, 1 );

  if ( destroy && m_widget [ WG_MAIN_TOP ] )
    ::gtk_widget_destroy ( m_widget [ WG_MAIN_TOP ] );
  
  m_widget [ WG_MAIN_TOP ] = 0;

  return PISA_ERR_SUCCESS;
}

void
main_window::start_scan (const GtkWidget *destination) const
{
  if (destination == scan_btn_)
    g_view_manager->set_destination (PISA_DE_FILE);
  else if (destination == print_btn_)
    g_view_manager->set_destination (PISA_DE_PRINTER);
  else {
    fprintf (stderr, "Unsupported scan destination.\n");
    return;
  }
  g_view_manager->start_scan ();
}

void
main_window::set_double_feed_detection (const GtkWidget *widget)
{
  long value = PISA_DFD_OFF;

  if (widget == m_widget[WG_MAIN_DFD]) {
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) {
      gtk_widget_set_sensitive (m_widget[WG_MAIN_DFD_BOX], true);
      if (gtk_toggle_button_get_active
          (GTK_TOGGLE_BUTTON (m_widget[WG_MAIN_DFD_STD]))) {
        value = PISA_DFD_STD;
      } else if (gtk_toggle_button_get_active
                 (GTK_TOGGLE_BUTTON (m_widget[WG_MAIN_DFD_THIN]))) {
        value = PISA_DFD_THIN;
      } else {
        fprintf (stderr, "No known active DFD value.  "
                 "Falling back to 'Standard'.\n");
        value = PISA_DFD_STD;
      }
    } else {
      gtk_widget_set_sensitive (m_widget[WG_MAIN_DFD_BOX], false);
      value = PISA_DFD_OFF;
    }
  } else {
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) {
      if (widget == m_widget[WG_MAIN_DFD_STD]) {
        value = PISA_DFD_STD;
      } else if (widget == m_widget[WG_MAIN_DFD_THIN]) {
        value = PISA_DFD_THIN;
      } else {
        fprintf (stderr, "Unsupported DFD value.\n");
        return;
      }
    } else {
      return;                   // nothing to do
    }
  }

  scan_manager *sm = g_view_manager->get_scan_manager ();
  if (sm && sm->has_dfd ()) sm->set_dfd (value);
}

/*------------------------------------------------------------*/
void main_window::sensitive ( int is_prev_img )
{
  gboolean enable_expose, enable_dest, enable_config;
  
  _done_preview = bool (is_prev_img);

  enable_expose = enable_dest = enable_config= TRUE;

  if ( is_prev_img == 0 ||
       g_view_manager->get_settings ().imgtype.pixeltype == PISA_PT_BW )
    enable_expose = FALSE;

  if ( ::g_view_manager->is_gimp ( ) )
    {
      enable_dest = FALSE;
      enable_config = FALSE;
    }

  update_zoom_button ();

  gtk_widget_set_sensitive (expo_btn_, enable_expose);
  gtk_widget_set_sensitive ((_compact_mode ? print_btn_ : dest_menu_),
                            enable_dest);
  gtk_widget_set_sensitive (config_, enable_config);

  sensitive_option ();
}

void main_window::sensitive_option ()
{
  sensitive_image_type ( );
  sensitive_scale ( );
  sensitive_resolution ( );
  sensitive_target ( );
  sensitive_focus ( );
  sensitive_usm ( );
  sensitive_size_check ();
  sensitive_deskew ();
}

void
main_window::enable_start_button (bool yes)
{
  gtk_widget_set_sensitive (scan_navi_, yes);
}

void
main_window::update_zoom_button ()
{
  long marq_size = 0;
  gboolean size_check_enabled = FALSE;
  gboolean enable_zoom = TRUE;
  
  marq_size = g_view_manager->get_marquee_size ();
  size_check_enabled = gtk_toggle_button_get_active
                       (GTK_TOGGLE_BUTTON (size_check_));
  
  if (marq_size < 2 || size_check_enabled || !_done_preview)
    {
      enable_zoom = FALSE;
    }
 
  gtk_widget_set_sensitive (zoom_btn_, enable_zoom);
}

static GtkWidget *
create_scan_button_(main_window *mw, bool toolbar)
{
  GtkWidget *button;

  if (toolbar) {
    button = GTK_WIDGET (gtk_tool_button_new_from_stock (GTK_STOCK_SAVE_AS));
    gtk_widget_set_tooltip_text (button, _("Scan to File"));
    gtk_signal_connect (GTK_OBJECT (button), "clicked",
                        GTK_SIGNAL_FUNC (click_scan), mw);
  } else {
    button = gtk_button_new ();
    gtk_container_add (GTK_CONTAINER (button),
                       xpmlabelbox (mw->get_widget (), scan_xpm, _("Scan")));
    gtk_signal_connect (GTK_OBJECT (button), "clicked",
                        GTK_SIGNAL_FUNC (click_scan_btn), mw);
  }

  return button;
}

static GtkWidget *
create_pref_button_(main_window *mw, bool toolbar)
{
  GtkWidget *button;
  const char *text = _("Configuration");

  if (toolbar) {
    button = GTK_WIDGET (gtk_tool_button_new_from_stock
                         (GTK_STOCK_PREFERENCES));
    gtk_widget_set_tooltip_text (button, text);
  } else {
    button = gtk_button_new_with_label (text);
  }
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (click_config), mw);

  return button;
}

//! A constructor-like tool button builder.
GtkWidget *
main_window::tool_button (const char *text, GCallback callback, gchar **icon)
{
  GtkWidget *widget;

  if (_compact_mode) {
    widget = GTK_WIDGET (gtk_tool_button_new
                         (xpm2widget (get_widget (), icon), text));
    gtk_widget_set_tooltip_text (widget, text);
  } else {
    widget = gtk_button_new ();
    gtk_container_add (GTK_CONTAINER (widget),
                       xpmlabelbox (get_widget (), icon, text));
  }
  g_signal_connect (G_OBJECT (widget), "clicked", callback, this);

  return widget;
}

//! A constructor-like check button builder.
GtkWidget *
main_window::check_button (const char *text,
                           void (*callback) (GtkWidget *, gpointer))
{
  GtkWidget *widget;

  widget = gtk_check_button_new_with_label (text);
  g_signal_connect (G_OBJECT (widget), "toggled",
                    G_CALLBACK (callback), this);

  return widget;
}

//! A constructor-like close button builder.
GtkWidget *
main_window::close_button ()
{
  GtkWidget *widget;
  const char *text = _("Close");

  if (_compact_mode) {
    widget = GTK_WIDGET (gtk_tool_button_new_from_stock (GTK_STOCK_CLOSE));
    gtk_widget_set_tooltip_text (widget, text);
  } else {
    widget = gtk_button_new_with_label (text);
  }
  g_signal_connect (G_OBJECT (widget), "clicked",
                    G_CALLBACK (click_close), this);

  return widget;
}

void
main_window::create_controls ()
{
  scan_manager *sm = g_view_manager->get_scan_manager ();

  prev_btn_ = tool_button (_("Preview"), click_preview_btn, preview_xpm);
  zoom_btn_ = tool_button (_("Zoom"), click_zoom_btn, zoom_xpm);
  expo_btn_ = tool_button (_("Auto Exposure"), click_expose_btn, auto_xpm);
  scan_btn_ = create_scan_button_(this, _compact_mode);

  scan_navi_  = check_button (_("enable Start button"), toggled_start_button);
  draft_mode_ = check_button (_("Speed priority scanning"),
                              toggled_draft_mode);
  size_check_ = check_button (_("Document Size - Auto Detect"),
                              toggled_size_check);
  deskew_ = check_button (_("Correct Document Skew"),
                              toggled_deskew);

  config_ = create_pref_button_(this, _compact_mode);

  unsharp_mask_ = check_button (_("Unsharp mask"), toggled_usm);

  if (getenv ("ISCAN_GUI_SHOW_ALL") || sm->has_dfd ()) {
    GtkWidget *widget;
    const char *text = _("Detect Double Feed (Paper Thickness)");
      GtkWidget *menu  = pisa_create_option_menu (g_double_feed_detection);
      GtkWidget *label = gtk_label_new (text);
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);

      widget = gtk_hbox_new (false, 5);
      gtk_box_pack_start (GTK_BOX (widget), label, false, false, 5);
      gtk_box_pack_start (GTK_BOX (widget), menu, false, false, 5);
    m_widget[WG_MAIN_DFD] = widget;
    gtk_widget_set_sensitive (m_widget[WG_MAIN_DFD], sm->using_adf ());
  }

  if (sm->push_button_needs_polling ())
  {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (scan_navi_), true);
    gtk_widget_set_sensitive (scan_navi_, false);
  }
  else
  {
    gtk_widget_set_sensitive (scan_navi_, sm->has_flatbed () || sm->has_tpu ());
  }
}

/*------------------------------------------------------------*/
GtkWidget * main_window::create_left_area ( void )
{
  GtkWidget *vbox = gtk_vbox_new (false, 0);

  if (!_compact_mode)
    gtk_box_pack_start (GTK_BOX (vbox), action_button_box (), false, false, 0);

  GtkWidget *widget = g_view_manager->create_window (ID_WINDOW_PREV);
  // In order to avoid a segmentation error in the preview_window
  // class (resulting from sloppy image size logic), we add a size
  // kludge here so that the conditions that trigger it become very
  // unlikely.
  // FIXME: remove kludge once preview_window gets fixed.
  ::gtk_widget_set_usize (GTK_WIDGET (widget), 360, 480);
  ::gtk_box_pack_start ( GTK_BOX ( vbox ), widget, TRUE, TRUE, 0 );
  ::gtk_widget_show ( widget );

  // destination menu and scan button
  if (!_compact_mode)
    {
      widget = create_scan_button ( );
      ::gtk_box_pack_end ( GTK_BOX ( vbox ), widget, FALSE, FALSE, 0 );
      ::gtk_widget_show ( widget );
    }

  return vbox;
}

//!  Assembles a widget with selected "action" buttons.
/*!  \note  Intended for use with the non-compact GUI mode only.
 */
GtkWidget *
main_window::action_button_box ()
{
  if (_compact_mode) return NULL;

  GtkWidget *hbox = gtk_hbox_new (false, 5);

  gtk_box_pack_start (GTK_BOX (hbox), prev_btn_, false, false, 5);
  gtk_box_pack_start (GTK_BOX (hbox), zoom_btn_, false, false, 5);
  gtk_box_pack_start (GTK_BOX (hbox), expo_btn_, false, false, 5);
  gtk_widget_show_all (hbox);

  return hbox;
}

//!  Assembles a toolbar widget from selected controls.
/*!  \note  Intended for use in compact GUI mode only.
 */
GtkWidget *
main_window::toolbar ()
{
  if (!_compact_mode) return NULL;

  const gint  at_end  = -1;
  GtkToolbar *toolbar = GTK_TOOLBAR (gtk_toolbar_new ());
  gtk_toolbar_set_style (toolbar, GTK_TOOLBAR_ICONS);
  gtk_toolbar_set_show_arrow (toolbar, false);

  GtkToolItem *btn, *sep;

  btn = gtk_tool_button_new_from_stock (GTK_STOCK_PRINT);
  gtk_widget_set_tooltip_text (GTK_WIDGET (btn), _("Scan to Print"));
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (click_scan), this);
  print_btn_ = GTK_WIDGET (btn);

  // Spring separator to push remaining buttons to the far end of
  // the toolbar
  sep = gtk_separator_tool_item_new ();
  gtk_tool_item_set_expand (sep, true);
  gtk_separator_tool_item_set_draw (GTK_SEPARATOR_TOOL_ITEM (sep),
                                    false);

  // Final toolbar tweaks
  if (!iscan::pngstream::is_usable ()) {
    gtk_widget_set_sensitive (print_btn_, false);
  }

  gtk_toolbar_insert (toolbar, GTK_TOOL_ITEM (prev_btn_), at_end);
  gtk_toolbar_insert (toolbar, GTK_TOOL_ITEM (zoom_btn_), at_end);
  gtk_toolbar_insert (toolbar, GTK_TOOL_ITEM (expo_btn_), at_end);
  gtk_toolbar_insert (toolbar, gtk_separator_tool_item_new (), at_end);
  gtk_toolbar_insert (toolbar, GTK_TOOL_ITEM (scan_btn_), at_end);
  gtk_toolbar_insert (toolbar, GTK_TOOL_ITEM (print_btn_), at_end);
  gtk_toolbar_insert (toolbar, gtk_separator_tool_item_new (), at_end);
  gtk_toolbar_insert (toolbar, GTK_TOOL_ITEM (config_), at_end);
  gtk_toolbar_insert (toolbar, sep, at_end);
  gtk_toolbar_insert (toolbar, GTK_TOOL_ITEM (close_button ()), at_end);

  gtk_widget_show_all (GTK_WIDGET (toolbar));

  return GTK_WIDGET (toolbar);
}

/*------------------------------------------------------------*/
GtkWidget * main_window::create_scan_button ( void )
{
  GtkWidget	* table;
  GtkWidget	* label;
  GtkWidget	* menu;
  GtkWidget	* vbox;

  table = ::gtk_table_new ( 2, 3, FALSE );
  ::gtk_container_border_width ( GTK_CONTAINER ( table ), 5 );
  ::gtk_table_set_row_spacings ( GTK_TABLE ( table ), 3 );
  ::gtk_table_set_col_spacings ( GTK_TABLE ( table ), 8 );
  
  label = ::gtk_label_new ( _( "Destination:" ) );
  ::gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 0, 1,
		       GTK_FILL, GTK_FILL, 0, 0 );
  ::gtk_widget_show ( label );

  menu = ::pisa_create_option_menu ( g_destination_menu_item );
  ::gtk_table_attach ( GTK_TABLE ( table ), menu, 1, 2, 0, 1,
		       GTK_FILL, GTK_FILL, 0, 0 );
  if (!iscan::pngstream::is_usable ()) {
    gtk_widget_set_sensitive (g_destination_menu_item[1].widget, FALSE);
  }
  ::gtk_widget_show ( menu );
  dest_menu_ = menu;

  gtk_table_attach (GTK_TABLE (table), scan_btn_,
                    2, 3, 0, 2, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show_all (scan_btn_);

  vbox = gtk_vbox_new ( FALSE, 0 );
  gtk_table_attach ( GTK_TABLE ( table ), vbox, 0, 2, 1, 2,
                     GTK_FILL, GTK_FILL, 0, 0 );
  gtk_widget_show ( vbox );

  gtk_box_pack_start (GTK_BOX (vbox), scan_navi_ , false, false, 0);
  gtk_box_pack_start (GTK_BOX (vbox), draft_mode_, false, false, 0);

  scan_manager *sm = g_view_manager->get_scan_manager ();
  if (sm->has_start_button ())
    gtk_widget_show (scan_navi_);
  if (sm->has_draft_mode ())
    gtk_widget_show (draft_mode_);

  gtk_widget_show (table);
  return table;
}

/*------------------------------------------------------------*/
GtkWidget * main_window::create_right_area ( void )
{
  GtkWidget	* vbox;
  GtkWidget	* widget;

  vbox = ::gtk_vbox_new ( FALSE, 5 );

  if (!_compact_mode)
    {
      #if THE_ORIGINAL_SOURCES_WERE_NOT_SUCH_A_MESS
        scan_selector  *ss = new scan_selector();
        ::gtk_box_pack_start( GTK_BOX( vbox ), ss->widget(), FALSE, FALSE, 0 );
        ss->update();			// seed list of available scanners
        ss->select();			// pick one
        ss->show();
      #else
        widget = create_scan_label ( );
        ::gtk_box_pack_start ( GTK_BOX ( vbox ), widget, FALSE, FALSE, 0 );
        ::gtk_widget_show ( widget );
      #endif /* THE_ORIGINAL_SOURCES_WERE_NOT_SUCH_A_MESS */
    }
  else
    gtk_box_pack_start (GTK_BOX (vbox), toolbar (), false, false, 0);

  widget = create_notebook ( );
  ::gtk_box_pack_start ( GTK_BOX ( vbox ), widget, FALSE, FALSE, 0 );
  ::gtk_widget_show ( widget );

  if (!_compact_mode)
    gtk_box_pack_end (GTK_BOX (vbox), close_button_box (), false, false, 0);

  return vbox;
}

/*------------------------------------------------------------*/
GtkWidget * main_window::create_scan_label ( void )
{
  GtkWidget *hbox;
  GtkWidget *widget;

  hbox = ::gtk_hbox_new( FALSE, 0 );

  widget = ::gtk_label_new( _("Scanner:") );
  ::gtk_box_pack_start( GTK_BOX( hbox ), widget, FALSE, FALSE, 5 );
  ::gtk_widget_show( widget );

  widget = ::gtk_label_new( ::g_view_manager->get_device_name() );
  ::gtk_label_set_line_wrap ( GTK_LABEL (widget), TRUE );
  ::gtk_box_pack_start( GTK_BOX( hbox ), widget, FALSE, FALSE, 5 );
  ::gtk_widget_show( widget );

  return hbox;
}

/*------------------------------------------------------------*/
GtkWidget * main_window::create_notebook ( void )
{
  GtkWidget	* notebook;
  GtkWidget	* label;
  GtkWidget	* widget;

  notebook = ::gtk_notebook_new ( );
  ::gtk_container_border_width ( GTK_CONTAINER ( notebook ), 5 );

  // document tab
  label = ::gtk_label_new ( _( "Document" ) );
  widget = create_document_tab ( );

  ::gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ), widget, label );
  ::gtk_widget_show ( label );
  ::gtk_widget_show ( widget );

    {
      GtkWidget *center, *vbox;
      const guint top = 15;

      label  = gtk_label_new (_("Image Controls"));
      vbox = g_view_manager->get_image_controls ()->create_controls ();

      add_maintenance_ui (vbox);

      gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, label);
      gtk_widget_show (label);
      gtk_widget_show_all (vbox);

      label = gtk_label_new (_("Tone Correction"));
      widget = (::g_view_manager
                ->get_gamma_correction ()->create_controls (get_widget ()));

      center = gtk_alignment_new (0.5, 0, 0, 0);
      gtk_alignment_set_padding (GTK_ALIGNMENT (center), top, 0, 0, 0);
      gtk_container_add (GTK_CONTAINER (center), widget);

      gtk_notebook_append_page (GTK_NOTEBOOK (notebook), center, label);
      gtk_widget_show (label);
      gtk_widget_show_all (center);
    }
  return notebook;
}

/*------------------------------------------------------------*/
GtkWidget * main_window::create_document_tab ( void )
{
  GtkWidget	* vbox;
  GtkWidget	* table;
  GtkWidget	* label;
  GtkWidget	* menu;
  GtkWidget	* size;
  GtkWidget	* focus;
  GtkWidget	* widget;
  scan_manager	* scan_mgr;

  scan_mgr = g_view_manager->get_scan_manager ();

  vbox = ::gtk_vbox_new ( FALSE, 0 );

  table = ::gtk_table_new ( 3, 2, FALSE );
  ::gtk_container_border_width ( GTK_CONTAINER ( table ), 5 );
  ::gtk_table_set_row_spacings ( GTK_TABLE ( table ), 3 );
  ::gtk_table_set_col_spacings ( GTK_TABLE ( table ), 5 );
  ::gtk_box_pack_start ( GTK_BOX ( vbox ), table, FALSE, FALSE, 5 );
  ::gtk_widget_show ( table );

  label = ::gtk_label_new ( _( "Document Source:" ) );
  ::gtk_misc_set_alignment ( GTK_MISC ( label ), 1.0, 0.5 );
  ::gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 0, 1,
		       GTK_FILL, GTK_FILL, 0, 0 );
  ::gtk_widget_show ( label );

  menu = ::pisa_create_option_menu ( g_docsrc_menu_item );
  ::gtk_table_attach ( GTK_TABLE ( table ), menu, 1, 2, 0, 1,
		       GTK_FILL, GTK_FILL, 0, 0 );
  ::gtk_widget_show ( menu );

  if (!scan_mgr->has_flatbed ())
    {
      gtk_widget_set_sensitive (g_docsrc_menu_item[0].widget, FALSE);
    }
  if (!scan_mgr->has_tpu ())
    {
      gtk_widget_set_sensitive (g_docsrc_menu_item[1].widget, FALSE);
      gtk_widget_set_sensitive (g_docsrc_menu_item[2].widget, FALSE);
    }
  if (!scan_mgr->has_adf ())
    {
      gtk_widget_set_sensitive (g_docsrc_menu_item[3].widget, FALSE);
    }
  if (!scan_mgr->has_duplex ())
    {
      gtk_widget_set_sensitive (g_docsrc_menu_item[4].widget, FALSE);
    }
  ::gtk_option_menu_set_history (GTK_OPTION_MENU (menu),
                                 (scan_mgr->has_flatbed ()
                                  ? 0 : (scan_mgr->has_adf ()
                                         ? 3 : 1)));

  label = ::gtk_label_new ( _( "Image Type:" ) );
  ::gtk_misc_set_alignment ( GTK_MISC ( label ), 1.0, 0.5 );
  ::gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 1, 2,
		       GTK_FILL, GTK_FILL, 0, 0 );
  ::gtk_widget_show ( label );

  menu = ::pisa_create_option_menu ( g_imgtype_menu_item );
  ::gtk_table_attach ( GTK_TABLE ( table ), menu, 1, 2, 1, 2,
		       GTK_FILL, GTK_FILL, 0, 0 );
  ::gtk_widget_show ( menu );
  m_widget [ WG_MAIN_IMG_MENU ] = menu;

  label = ::gtk_label_new ( _( "Resolution:" ) );
  ::gtk_misc_set_alignment ( GTK_MISC ( label ), 1.0, 0.5 );
  ::gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 2, 3,
		       GTK_FILL, GTK_FILL, 0, 0 );
  ::gtk_widget_show ( label );

  menu = ::pisa_create_option_menu ( g_resolution_menu_item );
  ::gtk_table_attach ( GTK_TABLE ( table ), menu, 1, 2, 2, 3,
		       GTK_FILL, GTK_FILL, 0, 0 );
  ::gtk_widget_show ( menu );
  m_widget [ WG_MAIN_RES_MENU ] = menu;

  size = create_target (scan_mgr);
  ::gtk_box_pack_start ( GTK_BOX ( vbox ), size, FALSE, FALSE, 0 );
  ::gtk_widget_show ( size );

  if (scan_mgr->has_focus ())
    {
      focus = create_focus ( );
      ::gtk_box_pack_start ( GTK_BOX ( vbox ), focus, FALSE, FALSE, 0 );
      ::gtk_widget_show ( focus );
    }

  widget = create_options ();
  gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0 );
  gtk_widget_show (widget);

  if (!_compact_mode)
    {
  widget = xpm2widget (get_widget (), penguin_xpm);
  gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 10 );
  gtk_widget_show (widget);
    }

  return vbox;
}

/*------------------------------------------------------------*/
GtkWidget * main_window::create_target (const scan_manager *sm)
{
  GtkWidget	* frame;
  GtkWidget	* vbox;
  GtkWidget	* hbox;
  GtkWidget	* label;
  GtkWidget	* entry;
  GtkWidget	* menu;
  GtkWidget	* scale;
  GtkWidget	* separator;

  frame = ::gtk_frame_new ( _( "Target" ) );
  ::gtk_container_border_width ( GTK_CONTAINER ( frame ), 5 );

  vbox = ::gtk_vbox_new ( FALSE, 5 );
  ::gtk_container_add ( GTK_CONTAINER ( frame ), vbox );
  ::gtk_widget_show ( vbox );

  hbox = ::gtk_hbox_new ( FALSE, 5 );
  ::gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 2 );
  ::gtk_widget_show ( hbox );

  label = ::gtk_label_new ( _( "W:" ) );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );
  ::gtk_widget_show ( label );

  entry = ::gtk_entry_new ( );
  ::gtk_widget_set_usize ( entry, 60, -1 );
  ::gtk_entry_set_text ( GTK_ENTRY ( entry ), "8.5" );
  ::gtk_widget_set_sensitive ( entry, FALSE );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), entry, FALSE, FALSE, 2 );
  ::gtk_widget_show ( entry );
  m_widget [ WG_MAIN_WIDTH ] = entry;

  label = ::gtk_label_new ( _( "H:" ) );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );
  ::gtk_widget_show ( label );

  entry = ::gtk_entry_new ( );
  ::gtk_widget_set_usize ( entry, 60, -1 );
  ::gtk_entry_set_text ( GTK_ENTRY ( entry ), "11.7" );
  ::gtk_widget_set_sensitive ( entry, FALSE );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), entry, FALSE, FALSE, 2 );
  ::gtk_widget_show ( entry );
  m_widget [ WG_MAIN_HEIGHT ] = entry;

  menu = ::pisa_create_option_menu ( ::g_unit_menu_item );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), menu, FALSE, FALSE, 5 );
  ::gtk_widget_show ( menu );

  separator = ::gtk_hseparator_new ( );
  ::gtk_box_pack_start ( GTK_BOX ( vbox ), separator, FALSE, FALSE, 2 );
  ::gtk_widget_show ( separator );

  // scale
  hbox = ::gtk_hbox_new ( FALSE, 0 );
  ::gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 2 );
  ::gtk_widget_show ( hbox );
  
  label = ::gtk_label_new ( _( "Scale" ) );
  ::gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.5 );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );
  ::gtk_widget_show ( label );

  scale = ::pisa_create_scale ( & ::g_scale_zoom );
  ::gtk_widget_set_usize ( scale, 200, -1 );
  ::gtk_scale_set_digits ( GTK_SCALE ( scale ), 0 );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), scale, TRUE, TRUE, 5 );
  ::gtk_widget_show ( scale );

  gtk_widget_show_all (frame);

  return frame;
}

/*------------------------------------------------------------*/
GtkWidget * main_window::create_focus ( void )
{
  GtkWidget	* frame;
  GtkWidget	* hbox;
  GtkWidget	* radio;
  GSList	* group;

  frame = ::gtk_frame_new ( _( "Focus" ) );
  ::gtk_container_border_width ( GTK_CONTAINER ( frame ), 5 );

  hbox = ::gtk_hbox_new ( FALSE, 5 );
  ::gtk_container_border_width ( GTK_CONTAINER ( hbox ), 5 );
  ::gtk_container_add ( GTK_CONTAINER ( frame ), hbox );
  ::gtk_widget_show ( hbox );
 
  radio = ::gtk_radio_button_new_with_label ( 0, "0.0" );
  ::gtk_signal_connect ( GTK_OBJECT ( radio ), "toggled",
			 GTK_SIGNAL_FUNC ( check_focus ), & ::g_focus [ 0 ] );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), radio, TRUE, TRUE, 0 );
  ::gtk_widget_show ( radio );
  m_widget [ WG_MAIN_FOCUS_0 ] = radio;

  group = ::gtk_radio_button_group ( GTK_RADIO_BUTTON ( radio ) );
  radio = ::gtk_radio_button_new_with_label ( group, "2.5" );
  ::gtk_signal_connect ( GTK_OBJECT ( radio ), "toggled",
			 GTK_SIGNAL_FUNC ( check_focus ), & ::g_focus [ 1 ] );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), radio, TRUE, TRUE, 0 );
  ::gtk_widget_show ( radio );
  m_widget [ WG_MAIN_FOCUS_25 ] = radio;

  return frame;
}

/*------------------------------------------------------------*/
GtkWidget *
main_window::create_options (void)
{
  GtkContainer *frame = GTK_CONTAINER (gtk_frame_new (_("Options")));
  GtkBox *vbox = GTK_BOX (gtk_vbox_new (false, 5));
  scan_manager *sm = g_view_manager->get_scan_manager ();

  gtk_container_border_width (frame, 5);
  gtk_container_add (frame, GTK_WIDGET (vbox));
  if (_compact_mode)
    {
      gtk_box_pack_start (vbox, scan_navi_ , false, false, 0);
      gtk_box_pack_start (vbox, draft_mode_, false, false, 0);
    }
  gtk_box_pack_start (vbox, unsharp_mask_, false, false, 0);
  gtk_box_pack_start (vbox, size_check_, false, false, 0);
  gtk_box_pack_start (vbox, deskew_, false, false, 0);

  if (getenv ("ISCAN_GUI_SHOW_ALL") || sm->has_dfd ())
    {                           // double feed detection
          gtk_box_pack_start (vbox, m_widget[WG_MAIN_DFD], false, false, 5);
  }

  gtk_widget_show_all (GTK_WIDGET (frame));
  if (!sm->has_start_button ())
    gtk_widget_hide (scan_navi_);
  if (!sm->has_draft_mode ())
    gtk_widget_hide (draft_mode_);

  return GTK_WIDGET (frame);
}

//!  Assembles a widget with the Configuration and Close buttons.
/*!  \note  Intended for use in non-compact GUI mode only.
 */
GtkWidget *
main_window::close_button_box ()
{
  if (_compact_mode) return NULL;

  GtkWidget *vbox = gtk_vbox_new (true, 5);
  GtkWidget *hbox = gtk_hbutton_box_new ();

  gtk_button_box_set_layout (GTK_BUTTON_BOX (hbox), GTK_BUTTONBOX_SPREAD);

  gtk_box_pack_start (GTK_BOX (vbox), gtk_hseparator_new (), false, false, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 0);
  gtk_box_pack_start (GTK_BOX (hbox), config_, false, false, 0);
  gtk_box_pack_start (GTK_BOX (hbox), close_button (), false, false, 0);

  gtk_widget_show_all (vbox);

  return vbox;
}

/*------------------------------------------------------------*/
void main_window::sensitive_image_type ( void )
{
  gboolean	enable_bw;
  int		i;

  if (g_view_manager->get_scan_manager ()->using_tpu ())
    enable_bw = FALSE;
  else
    enable_bw = TRUE;

  for ( i = 0; ::g_imgtype_menu_item [ i ].id != 0; i++ )
    {
      if ( ::g_imagetype_list [ i ].pixeltype == PISA_PT_BW )
      ::gtk_widget_set_sensitive ( ::g_imgtype_menu_item [ i ].widget, enable_bw );
    }

  for ( i = 0; ::g_imgtype_menu_item [ i ].id != 0; i++ )
    {
      if ( ::g_imgtype_menu_item [ i ].id == g_view_manager->get_settings ().imgtype.id )
	{
	  ::gtk_option_menu_set_history ( GTK_OPTION_MENU ( m_widget [ WG_MAIN_IMG_MENU ] ), i );
	  break;
	}
    }
}

/*------------------------------------------------------------*/
void main_window::sensitive_target ( void )
{
  long		resolution, unit, scale, scale_px;
  double	w, h;
  char		w_buf [ 32 ], h_buf [ 32 ];
  marquee	* marq;
  scan_manager	* scan_mgr;

  scan_mgr = g_view_manager->get_scan_manager ();

  resolution	= g_view_manager->get_settings ().resolution;
  unit		= g_view_manager->get_settings ().unit;

  marq = & g_view_manager->get_marquee ();

  w = marq->area.x;
  h = marq->area.y;
  scale = marq->scale;

  resolution = (resolution * scale + 50) / 100;

  scan_mgr->adjust_scan_param (&resolution, &scale_px);
  
  switch ( unit )
    {
    case PISA_UNIT_INCHES:
      w = ::inches_reflect_zoom ( w, scale ) + 0.005;
      h = ::inches_reflect_zoom ( h, scale ) + 0.005;
      ::sprintf ( w_buf, "%u.%02u",
		  ( int ) w,
		  ( int ) ( w * 100 - ( int ) w * 100 ) );
      ::sprintf ( h_buf, "%u.%02u",
		  ( int ) h,
		  ( int ) ( h * 100 - ( int ) h * 100 ) );
      break;
      
    case PISA_UNIT_PIXELS:
      w = ::inch2width ( w, resolution, scale_px,
			 PISA_PT_BW == g_view_manager->get_settings ().imgtype.pixeltype);
      h = ::inch2height ( h, resolution, scale_px );
      ::sprintf ( w_buf, "%u", ( int ) w );
      ::sprintf ( h_buf, "%u", ( int ) h );
      break;

    case PISA_UNIT_CM:
      w = ::inch2centi ( w, scale ) + 0.005;
      h = ::inch2centi ( h, scale ) + 0.005;
      ::sprintf ( w_buf, "%u.%02u",
		  ( int ) w,
		  ( int ) ( w * 100 - ( int ) w * 100 ) );
      ::sprintf ( h_buf, "%u.%02u",
		  ( int ) h,
		  ( int ) ( h * 100 - ( int ) h * 100 ) );
      break;
    }

  ::gtk_entry_set_text ( GTK_ENTRY ( m_widget [ WG_MAIN_WIDTH ] ), w_buf );
  ::gtk_entry_set_text ( GTK_ENTRY ( m_widget [ WG_MAIN_HEIGHT ] ), h_buf );
}

/*------------------------------------------------------------*/
void main_window::sensitive_resolution ( void )
{
  long	max_resolution, i, resolution;

  max_resolution = ::g_view_manager->get_scan_manager()->get_max_resolution ();

  for ( i = 0; ::g_resolution_menu_item [ i ].id != 0; i++ )
    {
      if ( max_resolution < ::g_resolution_menu_item [ i ].id )
	::gtk_widget_set_sensitive ( ::g_resolution_menu_item [ i ].widget,
				     FALSE );
      else
	::gtk_widget_set_sensitive ( ::g_resolution_menu_item [ i ].widget,
				     TRUE );
    }

  resolution = g_view_manager->get_settings ().resolution;

  for ( i = 0; ::g_resolution_menu_item [ i ].id != 0; i++ )
    {
      if ( ::g_resolution_menu_item [ i ].id == resolution )
	{
	  ::gtk_option_menu_set_history ( GTK_OPTION_MENU ( m_widget [ WG_MAIN_RES_MENU ] ), i );
	  break;
	}
    }
}

/*------------------------------------------------------------*/
void main_window::sensitive_scale ( void )
{
  const long	min_scale =  50;
  const long	max_scale = 200;
  GtkAdjustment	* adjust;
  marquee	* marq;

  marq = & g_view_manager->get_marquee ();

  if ( marq->scale < min_scale )
    marq->scale = min_scale;
  if ( marq->scale > max_scale )
    marq->scale = max_scale;

  adjust = ::gtk_range_get_adjustment ( GTK_RANGE ( ::g_scale_zoom.widget ) );
  adjust->lower = min_scale;
  adjust->upper = max_scale;
  adjust->value = marq->scale;

  ::gtk_signal_emit_by_name ( GTK_OBJECT ( ::g_scale_zoom.adjust ),
			      "changed" );  
}

/*------------------------------------------------------------*/
void main_window::sensitive_focus ( void )
{
  scan_manager	* scan_mgr;
  marquee	* marq;

  scan_mgr = g_view_manager->get_scan_manager ();

  if (!scan_mgr->has_focus ())
    return;

  marq = & g_view_manager->get_marquee ();

  if (m_widget[WG_MAIN_FOCUS_25] && marq->focus == 25)
    ::gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( m_widget [ WG_MAIN_FOCUS_25 ] ),
				     TRUE );
  if (m_widget[WG_MAIN_FOCUS_0 ] && marq->focus ==  0)
    ::gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( m_widget [ WG_MAIN_FOCUS_0 ] ),
				     TRUE );
    
}

/*------------------------------------------------------------*/
void main_window::sensitive_usm ( void )
{
  settings s = g_view_manager->get_settings ();

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (unsharp_mask_), s.usm);
  gtk_widget_set_sensitive (unsharp_mask_, PISA_PT_BW != s.imgtype.pixeltype);
}

void
main_window::do_sensitive_size_check (bool yes)
{
  _do_sensitive_size_check = yes;
}

void
main_window::sensitive_size_check (void)
{
  scan_manager *sm = g_view_manager->get_scan_manager ();
  bool size_check_active = false;
  bool autocrop_active = false;

  if (!_do_sensitive_size_check) return;

  if (sm->has_size_check ())
  {
    size_check_active = sm->get_size_check ();
  }
  if (sm->has_autocrop ())
  {
    autocrop_active = sm->get_autocrop ();
  }
  if (sm->has_size_check () || sm->has_autocrop ())
  {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (size_check_),
                                  (size_check_active || autocrop_active) );
  }
  gtk_widget_set_sensitive (size_check_,
                            (sm->has_size_check () || sm->has_autocrop () ));
}

void
main_window::sensitive_deskew (void)
{
  scan_manager *sm = g_view_manager->get_scan_manager ();

  if (!_do_sensitive_size_check) return;

  if (sm->has_deskew ())
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (deskew_),
                                    sm->get_deskew () );
    }
  gtk_widget_set_sensitive (deskew_, sm->has_deskew ());
}

void
main_window::add_maintenance_ui (GtkWidget *vbox)
{
  GtkWidget *separator;
  GtkWidget *bbox;
  GtkWidget *button;
  scan_manager *scan_mgr = g_view_manager->get_scan_manager ();

  if (scan_mgr->has_clean () || scan_mgr->has_calibrate ())
  {
    // add Calibration & Cleaning button
    separator = ::gtk_hseparator_new ( );
    ::gtk_box_pack_start ( GTK_BOX ( vbox ), separator, FALSE, FALSE, 2 );
    ::gtk_widget_show ( separator );

    bbox = ::gtk_vbutton_box_new ();
    ::gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), GTK_BUTTONBOX_SPREAD);

    if (scan_mgr->has_calibrate ())
    {
      button = ::gtk_button_new_with_label ( _( "Calibration" ) );
      ::gtk_container_border_width ( GTK_CONTAINER ( button ), 5 );
      ::gtk_box_pack_start (GTK_BOX (bbox), button, false, false, 0);
      ::g_signal_connect (GTK_OBJECT (button), "clicked",
                          GTK_SIGNAL_FUNC (click_calibration), scan_mgr);

      ::gtk_widget_show ( button );
    }

    if (scan_mgr->has_clean ())
    {
      button = ::gtk_button_new_with_label ( _( "Cleaning" ) );
      ::gtk_container_border_width ( GTK_CONTAINER ( button ), 5 );
      ::gtk_box_pack_start (GTK_BOX (bbox), button, false, false, 0);
      ::g_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (click_cleaning), scan_mgr);

      ::gtk_widget_show ( button );
    }

    ::gtk_box_pack_start (GTK_BOX (vbox), bbox, true, false, 0);
    ::gtk_widget_show ( bbox );

    // for padding
    ::gtk_box_pack_start (GTK_BOX (vbox), gtk_vbox_new (false, 0), true, true, 0);
  }
}
