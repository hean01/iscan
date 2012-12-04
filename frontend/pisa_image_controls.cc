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
#include <string.h>

/*------------------------------------------------------------*/
#include "pisa_view_manager.h"
#include "pisa_image_controls.h"
#include "pisa_tool.h"
#include "pisa_error.h"
#include "pisa_structs.h"
#include "pisa_default_val.h"

/*------------------------------------------------------------*/
static void value_changed ( GtkAdjustment * adjust, gpointer data );

/*------------------------------------------------------------*/
#define ID_SCALE_GAMMA		0x0001
#define ID_SCALE_HIGHLIGHT	0x0002
#define ID_SCALE_SHADOW		0x0003
#define ID_SCALE_THRESHOLD	0x0004
#define ID_SCALE_BRIGHTNESS	0x0005
#define ID_SCALE_CONTRAST	0x0006

scale_items g_scale_gamma = { ID_SCALE_GAMMA, 0, 0,
			      ( GtkSignalFunc * ) value_changed, 
			      DEFGAMMA,
			      MINGAMMA,
			      MAXGAMMA,
			      LINEGAMMA,
			      PAGEGAMMA };
scale_items g_scale_highlight = { ID_SCALE_HIGHLIGHT, 0, 0,
				  ( GtkSignalFunc * ) value_changed,
				  DEFHIGHLIGHT,
				  MINHIGHLIGHT,
				  MAXHIGHLIGHT,
				  LINEHIGHLIGHT,
				  PAGEHIGHLIGHT };
scale_items g_scale_shadow = { ID_SCALE_SHADOW, 0, 0,
			       ( GtkSignalFunc * ) value_changed,
			       DEFSHADOW,
			       MINSHADOW,
			       MAXSHADOW,
			       LINESHADOW,
			       PAGESHADOW };
scale_items g_scale_threshold = { ID_SCALE_THRESHOLD, 0, 0,
				  ( GtkSignalFunc * ) value_changed,
				  DEFTHRESHOLD,
				  MINTHRESHOLD,
				  MAXTHRESHOLD,
				  LINETHRESHOLD,
				  PAGETHRESHOLD };
scale_items g_scale_brightness = { ID_SCALE_BRIGHTNESS, 0, 0,
				  ( GtkSignalFunc * ) value_changed,
				  0, /* get value from backend */
				  0, /* get range.max from backend */
				  0, /* get range.min from backend */
				  0, /* get range.quant from backend */
				  0 };
scale_items g_scale_contrast = { ID_SCALE_CONTRAST, 0, 0,
				  ( GtkSignalFunc * ) value_changed,
				  0, /* get value from backend */
				  0, /* get range.max from backend */
				  0, /* get range.min from backend */
				  0, /* get range.quant from backend */
				  0 };

/*------------------------------------------------------------*/

static void value_changed ( GtkAdjustment * adjust, gpointer data )
{
  long		id;
  long		val;
  int		change;
  marquee	* marq;

  id = * ( long * ) data;
  change = 1;
  marq = & g_view_manager->get_marquee ();

  switch ( id )
    {
    case ID_SCALE_GAMMA:
      val = ( long ) ( adjust->value * 100 );
      if ( marq->gamma == val )
	change = 0;
      else
	marq->gamma = val;
      break;

    case ID_SCALE_HIGHLIGHT:
      val = ( long ) ( adjust->value );
      if ( marq->highlight == val )
	change = 0;
      else
	marq->highlight = val;
      break;

    case ID_SCALE_SHADOW:
      val = ( long ) ( adjust->value );
      if ( marq->shadow == val )
	change = 0;
      else
	marq->shadow = val;
      break;

    case ID_SCALE_THRESHOLD:
      val = ( long ) ( adjust->value );
      if ( marq->threshold == val )
	change = 0;
      else
	marq->threshold = val;
      break;

    case ID_SCALE_BRIGHTNESS:
      val = ( long ) ( adjust->value );
      if ( marq->brightness != val )
        {
          marq->brightness = val;
        }
      change = 0;
      break;

    case ID_SCALE_CONTRAST:
      val = ( long ) ( adjust->value );
      if ( marq->contrast != val )
        {
          marq->contrast = val;
        }
      change = 0;
      break;
    }

  if ( change )
    {
      ::g_view_manager->update_lut ( );
      preview_window *prev_cls =
	(preview_window *) ::g_view_manager->get_window_cls (ID_WINDOW_PREV);

      prev_cls->update_img ( );
    }
}

/*------------------------------------------------------------*/
void image_controls::sensitive ( int is_prev_img )
{
  marquee	* marq;
  gboolean	enable_color, enable_bw;
  gboolean	enable_brightness_contrast;
  scan_manager	* scan_mgr;

  scan_mgr = g_view_manager->get_scan_manager ();

  marq = & g_view_manager->get_marquee ();

  // update value
  ::gtk_adjustment_set_value ( ::g_scale_gamma.adjust, marq->gamma / 100.0f );
  ::gtk_adjustment_set_value ( ::g_scale_highlight.adjust, marq->highlight );
  ::gtk_adjustment_set_value ( ::g_scale_shadow.adjust, marq->shadow );
  ::gtk_adjustment_set_value ( ::g_scale_threshold.adjust, marq->threshold );
  ::gtk_adjustment_set_value ( ::g_scale_brightness.adjust, marq->brightness );
  ::gtk_adjustment_set_value ( ::g_scale_contrast.adjust, marq->contrast );

  // grayout
  if ( is_prev_img
       && g_view_manager->get_settings ().imgtype.pixeltype != PISA_PT_BW )
    enable_color = TRUE;
  else
    enable_color = FALSE;

  if ( is_prev_img
       && g_view_manager->get_settings ().imgtype.pixeltype == PISA_PT_BW )
    enable_bw = TRUE;
  else
    enable_bw = FALSE;

  ::gtk_widget_set_sensitive ( ::g_scale_gamma.widget, enable_color );
  ::gtk_widget_set_sensitive ( ::g_scale_highlight.widget, enable_color );
  ::gtk_widget_set_sensitive ( ::g_scale_shadow.widget, enable_color );
  ::gtk_widget_set_sensitive ( ::g_scale_threshold.widget, enable_bw );

  // reset brightness, contrast
  enable_brightness_contrast = TRUE;
  if ( is_prev_img )
    {
      ::gtk_range_set_value ( GTK_RANGE (::g_scale_brightness.widget ),
                              brightness_default );
      ::gtk_range_set_value ( GTK_RANGE (::g_scale_contrast.widget ),
                              contrast_default );
      enable_brightness_contrast = FALSE;
    }
  ::gtk_widget_set_sensitive ( ::g_scale_brightness.widget,
                               ( enable_brightness_contrast
                                 && scan_mgr->has_brightness() ) );
  ::gtk_widget_set_sensitive ( ::g_scale_contrast.widget,
                               ( enable_brightness_contrast
                                 && scan_mgr->has_contrast() ) );
}

/*------------------------------------------------------------*/
GtkWidget * image_controls::create_controls ( void )
{
  GtkWidget	* vbox;
  GtkWidget	* table;
  GtkWidget	* label;
  GtkWidget	* scale;
  GtkWidget	* separator;
  scan_manager	* scan_mgr;

  scan_mgr = g_view_manager->get_scan_manager ();

  table = ::gtk_table_new ( 4, 2, FALSE );
  ::gtk_container_border_width ( GTK_CONTAINER ( table ), 5 );
  ::gtk_table_set_row_spacings ( GTK_TABLE ( table ), 3 );
  ::gtk_table_set_col_spacings ( GTK_TABLE ( table ), 5 );

  // gamma
  label = ::gtk_label_new ( _( "Gamma:" ) );
  ::gtk_misc_set_alignment ( GTK_MISC ( label ), 1.0, 0.5 );
  ::gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 0, 1,
		       GTK_FILL, GTK_FILL, 0, 0 );
  ::gtk_widget_show ( label );

  scale = ::pisa_create_scale ( & ::g_scale_gamma );
  ::gtk_widget_set_usize ( scale, 200, -1 );
  ::gtk_scale_set_digits ( GTK_SCALE ( scale ), 2 );
  ::gtk_range_set_update_policy ( GTK_RANGE ( scale ), GTK_UPDATE_DELAYED );
  ::gtk_table_attach ( GTK_TABLE ( table ), scale, 1, 2, 0, 1,
		       GTK_FILL, GTK_FILL, 0, 0 );
  ::gtk_widget_show ( scale );

  // highlight
  label = ::gtk_label_new ( _( "Highlight:" ) );
  ::gtk_misc_set_alignment ( GTK_MISC ( label ), 1.0, 0.5 );
  ::gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 1, 2,
		       GTK_FILL, GTK_FILL, 0, 0 );
  ::gtk_widget_show ( label );

  scale = ::pisa_create_scale ( & ::g_scale_highlight );
  ::gtk_widget_set_usize ( scale, 200, -1 );
  ::gtk_scale_set_digits ( GTK_SCALE ( scale ), 0 );
  ::gtk_range_set_update_policy ( GTK_RANGE ( scale ), GTK_UPDATE_DELAYED );
  ::gtk_table_attach ( GTK_TABLE ( table ), scale, 1, 2, 1, 2,
		       GTK_FILL, GTK_FILL, 0, 0 );
  ::gtk_widget_show ( scale );

  // shadow
  label = ::gtk_label_new ( _( "Shadow:" ) );
  ::gtk_misc_set_alignment ( GTK_MISC ( label ), 1.0, 0.5 );
  ::gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 2, 3,
		       GTK_FILL, GTK_FILL, 0, 0 );
  ::gtk_widget_show ( label );

  scale = pisa_create_scale ( & ::g_scale_shadow );
  ::gtk_widget_set_usize ( scale, 200, -1 );
  ::gtk_scale_set_digits ( GTK_SCALE ( scale ), 0 );
  ::gtk_range_set_update_policy ( GTK_RANGE ( scale ), GTK_UPDATE_DELAYED );
  ::gtk_table_attach ( GTK_TABLE ( table ), scale, 1, 2, 2, 3,
		       GTK_FILL, GTK_FILL, 0, 0 );
  ::gtk_widget_show ( scale );

  // threshold
  label = ::gtk_label_new ( _( "Threshold:" ) );
  ::gtk_misc_set_alignment ( GTK_MISC ( label ), 1.0, 0.5 );
  ::gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 3, 4,
		       GTK_FILL, GTK_FILL, 0, 0 );
  ::gtk_widget_show ( label );

  scale = pisa_create_scale ( & ::g_scale_threshold );
  ::gtk_widget_set_usize ( scale, 200, -1 );
  ::gtk_scale_set_digits ( GTK_SCALE ( scale ), 0 );
  ::gtk_range_set_update_policy ( GTK_RANGE ( scale ), GTK_UPDATE_DELAYED );
  ::gtk_table_attach ( GTK_TABLE ( table ), scale, 1, 2, 3, 4,
		       GTK_FILL, GTK_FILL, 0, 0 );
  ::gtk_widget_show ( scale );

  ::gtk_widget_show ( table );

  vbox = gtk_vbox_new (false, 5);
  gtk_box_pack_start (GTK_BOX (vbox), table, false, false, 0);

  separator = ::gtk_hseparator_new ( );
  ::gtk_box_pack_start ( GTK_BOX ( vbox ), separator, FALSE, FALSE, 2 );
  ::gtk_widget_show ( separator );

  // new table
  table = ::gtk_table_new ( 2, 2, FALSE );
  ::gtk_container_border_width ( GTK_CONTAINER ( table ), 5 );
  ::gtk_table_set_row_spacings ( GTK_TABLE ( table ), 3 );
  ::gtk_table_set_col_spacings ( GTK_TABLE ( table ), 5 );

  // Set brightness-method before you get brightness.
  scan_mgr->set_brightness_method(br_iscan);

  // brightness
  label = ::gtk_label_new ( _( "Brightness:" ) );
  ::gtk_misc_set_alignment ( GTK_MISC ( label ), 1.0, 0.5 );
  ::gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 0, 1,
		       GTK_FILL, GTK_FILL, 0, 0 );
  ::gtk_widget_show ( label );

  scan_mgr->get_value (SANE_NAME_BRIGHTNESS, &g_scale_brightness.val);
  scan_mgr->get_range (SANE_NAME_BRIGHTNESS,
                       &g_scale_brightness.max,
                       &g_scale_brightness.min,
                       &g_scale_brightness.step);
  g_scale_brightness.page = g_scale_brightness.step * 5;
  brightness_default = g_scale_brightness.val;

  scale = pisa_create_scale ( & ::g_scale_brightness );
  ::gtk_widget_set_usize ( scale, 200, -1 );
  ::gtk_scale_set_digits ( GTK_SCALE ( scale ), 0 );
  ::gtk_range_set_update_policy ( GTK_RANGE ( scale ), GTK_UPDATE_DELAYED );
  ::gtk_table_attach ( GTK_TABLE ( table ), scale, 1, 2, 0, 1,
		       GTK_FILL, GTK_FILL, 0, 0 );
  ::gtk_widget_show ( scale );

  // contrast
  label = ::gtk_label_new ( _( "Contrast:" ) );
  ::gtk_misc_set_alignment ( GTK_MISC ( label ), 1.0, 0.5 );
  ::gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 1, 2,
		       GTK_FILL, GTK_FILL, 0, 0 );
  ::gtk_widget_show ( label );

  scan_mgr->get_value (SANE_NAME_CONTRAST, &g_scale_contrast.val);
  scan_mgr->get_range (SANE_NAME_CONTRAST,
                       &g_scale_contrast.max,
                       &g_scale_contrast.min,
                       &g_scale_contrast.step);
  g_scale_contrast.page = g_scale_contrast.step * 5;
  contrast_default = g_scale_contrast.val;

  scale = pisa_create_scale ( & ::g_scale_contrast );
  ::gtk_widget_set_usize ( scale, 200, -1 );
  ::gtk_scale_set_digits ( GTK_SCALE ( scale ), 0 );
  ::gtk_range_set_update_policy ( GTK_RANGE ( scale ), GTK_UPDATE_DELAYED );
  ::gtk_table_attach ( GTK_TABLE ( table ), scale, 1, 2, 1, 2,
		       GTK_FILL, GTK_FILL, 0, 0 );
  ::gtk_widget_show ( scale );

  ::gtk_widget_show ( table );

  gtk_box_pack_start (GTK_BOX (vbox), table, false, false, 0);

  return vbox;
}
