/* pisa_view_manager.cc
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

#include <config.h>

#include "gettext.h"
#define  _(msg_id)	gettext (msg_id)

/*------------------------------------------------------------*/
#include <gtk/gtk.h>
#ifndef HAVE_GTK_2
#include <gdk_imlib.h>
#endif
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include <dirent.h>
#include <locale.h>
#include <libgen.h>
#include <time.h>

/*------------------------------------------------------------*/
#include "pisa_view_manager.h"
#include "pisa_error.h"
#include "pisa_main.h"
#include "pisa_scan_tool.h"
#include "pisa_default_val.h"
#include "pisa_gimp.h"
#include "pisa_aleart_dialog.h"
#include "pisa_change_unit.h"
#include "pisa_preference.h"
#include "pisa_scan_selector.h"

#include "file-selector.h"

#include "imgstream.hh"


#define DEFAULT_RESOLUTION	300	// dpi

/*------------------------------------------------------------*/
view_manager * g_view_manager = 0;

/*------------------------------------------------------------*/
int view_manager::create_view_manager ( int argc, char * argv [ ] )
{
  ::gtk_set_locale ( );
  ::setlocale (LC_NUMERIC, "C");
  ::gtk_init ( & argc, & argv );
  
#ifndef HAVE_GTK_2
  ::gdk_imlib_init ( );
  ::gtk_widget_push_visual ( ::gdk_imlib_get_visual ( ) );
  ::gtk_widget_push_colormap ( ::gdk_imlib_get_colormap ( ) );
#endif
  
  try
    {
      if ( ::g_view_manager )
	throw pisa_error (  PISA_ERR_PARAMETER );

      ::g_view_manager = new view_manager;
      if ( ::g_view_manager == 0 )
	throw pisa_error (  PISA_ERR_OUTOFMEMORY );

      // initialize parameters
      ::g_view_manager->init ( );

    }
  catch ( pisa_error & err )
    {
      aleart_dialog aleart_dlg;

      aleart_dlg.message_box ( 0, err.get_error_string ( ) );

      if ( ::g_view_manager )
	{
	  delete ::g_view_manager;
	  ::g_view_manager = 0;
	}

      return err.get_error_id ( );
    }

  return PISA_ERR_SUCCESS;
}

/*------------------------------------------------------------*/
void view_manager::release_view_manager ( void )
{
  if ( ::g_view_manager )
    {
      delete ::g_view_manager;
      ::g_view_manager = 0;
    }
}

/*------------------------------------------------------------*/
int view_manager::main ( void )
{
  gtk_main ( );

  return PISA_ERR_SUCCESS;
}

/*------------------------------------------------------------*/
void view_manager::init ( void )
{
  // initialize
  m_scanmanager_cls	= 0;
  m_main_cls		= 0;
  m_prev_cls		= 0;
  m_imgctrl_cls		= 0;
  m_gamma_cls		= 0;
  m_config_cls		= 0;
  m_filsel_cls		= 0;
  m_scansel_cls		= 0;

  // open scanner
  m_scanmanager_cls = new scan_manager;

  if ( ! m_scanmanager_cls )
    throw pisa_error ( PISA_ERR_OUTOFMEMORY );

  open_device ( );

  m_set.resolution	= DEFAULT_RESOLUTION;
  m_set.enable_start_button =
    m_scanmanager_cls->push_button_needs_polling ();
  m_set.enable_draft_mode = false;
  m_set.usm		= 1;
  m_set.unit		= PISA_UNIT_INCHES;

  if ( pisa_gimp_plugin ( ) )
    m_set.destination	= PISA_DE_GIMP;
  else
    m_set.destination	= PISA_DE_FILE;

  m_main_cls	= new main_window;
  m_prev_cls	= new preview_window;
  m_imgctrl_cls	= new image_controls;
  m_gamma_cls	= new gamma_correction;
  m_config_cls	= new config_window;
  m_filsel_cls	= new file_selector;

  if ( ! m_main_cls ||
       ! m_prev_cls ||
       ! m_imgctrl_cls ||
       ! m_gamma_cls ||
       ! m_config_cls ||
       ! m_filsel_cls )
    {
      destroy ( );
      throw ( PISA_ERR_OUTOFMEMORY );
    }

  init_img_info ();

  m_main_cls->init ( );
  m_prev_cls->init ( );
  m_gamma_cls->init ( );
  m_config_cls->init ( );
  m_filsel_cls->init ( );

  load_preference ( );

}

/*------------------------------------------------------------*/
void view_manager::destroy ( void )
{
  save_preference ( );

  m_set.delete_all ( );

  if ( m_scanmanager_cls )
    {
      try
	{
	  close_device ( );
	}
      catch ( pisa_error & err )
	{
	  aleart_dialog aleart_dlg;
	  
	  aleart_dlg.message_box ( 0, err.get_error_string ( ) );
	}

      delete m_scanmanager_cls;
      m_scanmanager_cls = 0;
    }
  
  if ( m_main_cls )
    {
      delete m_main_cls;
      m_main_cls = 0;
    }

  if ( m_prev_cls )
    {
      delete m_prev_cls;
      m_prev_cls = 0;
    }

  if ( m_imgctrl_cls )
    {
      delete m_imgctrl_cls;
      m_imgctrl_cls = 0;
    }

  if ( m_gamma_cls )
    {
      delete m_gamma_cls;
      m_gamma_cls = 0;
    }
  
  if ( m_config_cls )
    {
      delete m_config_cls;
      m_config_cls = 0;
    }

  if ( m_filsel_cls )
    {
      delete m_filsel_cls;
      m_filsel_cls = 0;
    }

  if ( m_scansel_cls )
    {
      delete m_scansel_cls;
      m_scansel_cls = 0;
    }

  ::pisa_quit ( );
}

/*------------------------------------------------------------*/
GtkWidget * view_manager::create_window ( pisa_window_id id )
{
  GtkWidget * ret = 0;

  switch ( id )
    {
    case ID_WINDOW_MAIN:
      ret = m_main_cls->create_window ( 0 );
      break;

    case ID_WINDOW_PREV:
      ret = m_prev_cls->create_window ( 0 );
      break;

    case ID_WINDOW_CONFIG:
      ret = m_config_cls->create_window ( m_main_cls->get_widget ( ) );
      break;
    }

  return ret;
}

/*------------------------------------------------------------*/
int view_manager::close_window ( pisa_window_id id, int destroy_flag )
{
  switch ( id )
    {
    case ID_WINDOW_MAIN:
      m_main_cls->close_window ( destroy_flag );
      destroy ( );
      break;

    case ID_WINDOW_PREV:
      m_prev_cls->close_window ( destroy_flag );
      break;

    case ID_WINDOW_CONFIG:
      m_config_cls->close_window ( destroy_flag );
      break;

    default:
      return PISA_ERR_PARAMETER;
    }

  return PISA_ERR_PARAMETER;
}

image_controls *
view_manager::get_image_controls ()
{
  return m_imgctrl_cls;
}

gamma_correction *
view_manager::get_gamma_correction ()
{
  return m_gamma_cls;
}

/*------------------------------------------------------------*/
void * view_manager::get_window_cls ( pisa_window_id id )
{
  void * ret = 0;

  switch ( id )
    {
    case ID_WINDOW_MAIN:
      ret = m_main_cls;
      break;

    case ID_WINDOW_PREV:
      ret = m_prev_cls;
      break;

    case ID_WINDOW_CONFIG:
      ret = m_config_cls;
      break;
    }

  return ret;
}

/*------------------------------------------------------------*/
void view_manager::sensitive ( void )
{
  int is_img = m_prev_cls->is_prev_img ( );
  
  m_main_cls->sensitive ( is_img );
  m_imgctrl_cls->sensitive ( is_img );
  m_gamma_cls->sensitive ( is_img );

  m_scanmanager_cls->has_prev_img ( is_img );
}

/*------------------------------------------------------------*/
int view_manager::is_gimp ( void )
{
  return pisa_gimp_plugin ( );
}

bool
view_manager::needs_duplex_rotation (void) const
{
  if (!m_scanmanager_cls->using_duplex ())
    return false;

  if (!m_scanmanager_cls->adf_duplex_direction_matches ())
    {                           // double pass ADF
      return  m_filsel_cls->has_left_edge_binding ();
    }
  else
    {                           // single pass ADF
      return !m_filsel_cls->has_left_edge_binding ();
    }
}

/*------------------------------------------------------------*/
void view_manager::start_scan ( void )
{
  using iscan::file_opener;
  using iscan::imgstream;

  if (PISA_ERR_SUCCESS != init_scan_param ())
    return;

  file_opener *fo = NULL;
  imgstream *is = NULL;

  bool remove = false;

  // All types of scans need to provide visual feedback on their
  // progress and for consecutive scans (via ADF or with the start
  // button enabled) it is better to create it here.  That way, it
  // will not disappear and reappear between calls to scan_file() or
  // scan_gimp(), eliminating rather annoying flicker.
  _feedback = new progress_window (m_main_cls->get_widget ());
  switch ( m_set.destination )
    {
    case PISA_DE_FILE:
      {
        m_filsel_cls->init ();
#ifndef HAVE_GTK_2
        m_filsel_cls->create_window (m_main_cls->get_widget (), m_set.option,
                                     m_set.enable_start_button,
                                     PISA_PT_BW == m_set.imgtype.pixeltype);
#else
        GtkWidget *w = m_main_cls->get_widget ();
        m_filsel_cls->create_window (GTK_WINDOW (gtk_widget_get_parent (w)), w,
                                     (m_scanmanager_cls->using_adf ()
                                      || m_set.enable_start_button),
                                     m_scanmanager_cls->using_duplex (),
                                     _image_format.c_str ());
#endif /* HAVE_GTK_2 */
        m_filsel_cls->hide ();
        if (!m_filsel_cls->get_pathname ()) // dialog cancelled
          break;

        // FIXME: the file_selector (or file name generator) should
        //        signal a change in file format and all interested
        //        parties, e.g. an object managing the configuration
        //        file, should take note and appropriate action
        _image_format = m_filsel_cls->get_format_name ();

        if (is_multi_image ()
            && !m_filsel_cls->multi_page_mode ())
          fo = new file_opener (std::string (m_filsel_cls->get_pathname ()),
                                m_filsel_cls->get_sequence_number ());
        else
          fo = new file_opener (std::string (m_filsel_cls->get_pathname ()));

        try
          {
            try
              {
                is = iscan::create_imgstream (*fo, m_filsel_cls->get_type (),
                                              needs_duplex_rotation ());
              }
            catch (std::exception& oops)
              {
	        throw pisa_error ( PISA_ERR_OUTOFMEMORY );
              }
          }
        catch (pisa_error& err)
          {
            aleart_dialog aleart_dlg;
            aleart_dlg.message_box (m_main_cls->get_widget (),
			      err.get_error_string ());
            m_filsel_cls->destroy ();
            remove = true;
            break;
          }
        remove = do_scan (*is, *fo);
        m_filsel_cls->destroy ();
      }
     break;

    case PISA_DE_PRINTER:
      {
        fo = new file_opener (false);
        is = new imgstream (*fo, iscan::PNG);
        remove = do_scan (*is, *fo);
      }
      break;

    case PISA_DE_GIMP:
      do_scan_gimp ();
      break;
    }
  delete is;
  try
    {
      if (fo && remove) fo->remove ();
    }
  catch (std::exception& oops) {}
  delete fo;
  delete _feedback;

  m_scanmanager_cls->finish_acquire ();
}

int
view_manager::update_lut (long index)
{
  return update_lut (m_set.get_marquee (index));
}

int
view_manager::update_lut (marquee& m)
{
  iscan::build_LUT (m_scanmanager_cls->get_scan_source (),
                    m_scanmanager_cls->get_film_type (),
                    m_set, m, !m_scanmanager_cls->has_zoom ());

  return PISA_ERR_SUCCESS;
}

bool
view_manager::change_document_source (const imagetype *img_type)
{
  bool success = true;

  try
    {
      set_image_type (img_type);
      m_set.delete_all ();
      init_img_info ();         // calls m_set.init()

      main_window *main_cls
	= static_cast <main_window *> (get_window_cls (ID_WINDOW_MAIN));
      main_cls->enable_start_button (!m_scanmanager_cls->using_adf ());
    }
  catch (pisa_error& err)
    {
      aleart_dialog aleart_dlg;

      aleart_dlg.message_box (m_main_cls->get_widget (),
			      err.get_error_string ());
      success = false;
    }

  set_resolution (DEFAULT_RESOLUTION);
  m_gamma_cls->reset (1);
  m_prev_cls->resize_window ();
  sensitive ();

  return success;
}

void view_manager::set_device( char * name )
{
  m_scanmanager_cls->close_device();
  m_scanmanager_cls->open_device( name );
}

char * view_manager::get_device_name() const
{
  return m_scansel_cls->get_device( true );
}

/*------------------------------------------------------------*/
void view_manager::open_device ( void )
{
  sane_init( 0, 0 );
  if (!m_scansel_cls)
    m_scansel_cls = new scan_selector( true );	// dialog box
  m_scanmanager_cls->open_device ( m_scansel_cls->get_device() );
}

/*------------------------------------------------------------*/
void view_manager::close_device ( void )
{
  m_scanmanager_cls->close_device ( );
}

/*------------------------------------------------------------*/
void view_manager::load_preference ( void )
{
  char pref_path [ 256 ];
  char pips_path [ 1024 ] = "lpr";
  char image_format [ 1024 ] = "PNG";

  cfg_struct cfg [ ] =
  {
    { "IMG", CFG_STRING, image_format },
    { "PIPS", CFG_STRING, pips_path }
  };

  ::strcpy ( pref_path, ::getenv ( "HOME" ) );
  ::strcat ( pref_path, "/" );
  ::strcat ( pref_path, PREFERENCE );

  ::get_cfg ( pref_path, cfg, sizeof ( cfg ) / sizeof ( cfg [ 0 ] ) );

  ::strcpy ( m_config_cls->m_cmd, pips_path );
  _image_format = image_format;
}

/*------------------------------------------------------------*/
void view_manager::save_preference ( void )
{
  char pref_path [ 256 ];
  char pips_path [ 1024 ] = "";

  cfg_struct cfg [ ] =
  {
    { "IMG", CFG_STRING, const_cast<char*> (_image_format.c_str ()) },
    { "PIPS", CFG_STRING, pips_path }
  };

  ::strcpy ( pref_path, ::getenv ( "HOME" ) );
  ::strcat ( pref_path, "/" );
  ::strcat ( pref_path, PREFERENCE );

  ::strcpy ( pips_path, m_config_cls->m_cmd );

  ::set_cfg ( pref_path, cfg, sizeof ( cfg ) / sizeof ( cfg [ 0 ] ) );
}


int
view_manager::init_img_info (void)
{
  int		i;
  marquee	* marq;
  double	max_width, max_height;
  double	coef [ 9 ];
  float		brightness, contrast;
  long		focus;


  m_scanmanager_cls->set_brightness_method(br_iscan);
  m_scanmanager_cls->get_value(SANE_NAME_BRIGHTNESS, &brightness);
  m_scanmanager_cls->get_value(SANE_NAME_CONTRAST, &contrast);

  if (m_scanmanager_cls->using_tpu ())
    focus		= 25;
  else
    focus		= 0;

  // get max area
  m_scanmanager_cls->get_current_max_size ( & max_width, & max_height );
  m_scanmanager_cls->get_color_profile ( coef );

  // initialize marquee
  marq = new marquee ( max_width, max_height, focus, brightness, contrast );
  m_set.init ( & marq );

  m_set.max_area [ 0 ]	= max_width;
  m_set.max_area [ 1 ]	= max_height;

  for ( i = 0; i < 9; i++ )
    m_set.coef [ i ] = coef [ i ];

  return PISA_ERR_SUCCESS;
}

int
view_manager::init_scan_param (void)
{
  marquee marq = get_marquee (m_set.get_marquee_size ( ) - 1);

  pisa_error_id err = m_scanmanager_cls->set_scan_parameters (m_set, marq);

  if (PISA_ERR_SUCCESS != err)
    {
      aleart_dialog dlg;

      dlg.message_box (m_main_cls->get_widget (),
		       pisa_error (err).get_error_string ());
    }
  return err;
}

// A pile of status flags used to track what's going on in the scan
// loop.  There are probably a few more than really needed, but at
// least these cover the individual situations we can encounter.

#define SCAN_BUTTON   0x10
#define SCAN_ADF      0x20
#define SCAN_NEXT     0x40
#define SCAN_FINISH   0x80
#define SCAN_ERROR    0x01
#define SCAN_CANCEL   0x02
#define SCAN_SINGLE   0x04
#define SCAN_DATA     0x08

bool
view_manager::do_scan (iscan::imgstream& is, const iscan::file_opener& fo,
                       bool first_time_around)
{
  static int status;

  if (first_time_around) status = 0;
  if (!m_filsel_cls->multi_page_mode ()) status |= SCAN_SINGLE;
  if (wait_for_button ()) status |= SCAN_BUTTON | SCAN_NEXT;
  if (m_scanmanager_cls->using_adf ()) status |= SCAN_ADF | SCAN_NEXT;

  try
    {
      do
        {
          is.next ();
          status &= ~SCAN_DATA;
          scan_file (is, &status, first_time_around);
          first_time_around = false;
          if (status & SCAN_ERROR ) status &= ~SCAN_NEXT;
          if (status & SCAN_CANCEL) status &= ~SCAN_NEXT;
          if (status & SCAN_FINISH) status &= ~SCAN_NEXT;
          if ((status & SCAN_DATA)
              && !(status & (SCAN_ERROR | SCAN_CANCEL)))
            {
              is.flush ();
              print (fo.name ());
            }
        }
      while (status & SCAN_NEXT);
    }
  catch (std::exception& oops)
    {
      return false;
    }

  // Signal whether the file should be removed.  Note that ADF and
  // scan button based scanning both open a temporary file ahead of
  // scanning.  Here we signal whether that temporary file needs to
  // be removed.  Also note that ADF scans ignore the scan button
  // setting, so we should check for ADF stuff first.

  if (status & (SCAN_ERROR | SCAN_CANCEL)) return true;

  if (status & SCAN_ADF)
    if (status & SCAN_FINISH) return (status & SCAN_SINGLE);

  if (status & SCAN_BUTTON)
    if (status & SCAN_FINISH) return (status & SCAN_SINGLE);

  return false;
}

int
view_manager::do_scan_gimp (bool first_time_around)
{
  int cancel = 0;

  if (!m_scanmanager_cls->using_adf ()
      && !m_set.enable_start_button)
    return scan_gimp (&cancel);

  bool eos    = false;

  while (!eos)
    {
      eos = scan_gimp (&cancel, first_time_around);
      first_time_around = false;
    }
  if (!cancel)
    do_scan_gimp (first_time_around);

  return PISA_ERR_SUCCESS;
}

int
view_manager::dialog_reply( const pisa_error& err ) const
{
  int reply = 0;
  
  aleart_dialog dlg;

  if ( PISA_STATUS_CANCELLED == err.get_error_id() )
    {
      // suppress message box when user cancelled from scanner side
      reply = SCAN_ERROR;
    }
  else if ( ( PISA_STATUS_GOOD < err.get_error_id() ) &&
       (m_scanmanager_cls->using_adf ()) )
    {
      int i = dlg.message_box( m_main_cls->get_widget(),
			       err.get_error_string(),
			       _("  Continue  "), _("  Cancel  ") );
      if (2 == i)
        reply = SCAN_FINISH;
      else
        reply = SCAN_NEXT;
    }
  else if ( ( PISA_STATUS_GOOD == err.get_error_id() ) &&
	    m_set.enable_start_button )
    {

      int i = dlg.message_box( m_main_cls->get_widget(),
			       "Waiting for ...",
			       _("  Finish  ") );

      if ( 1 == i )
	reply = SCAN_FINISH;
    }
  else
    {
      dlg.message_box( m_main_cls->get_widget(),
		       err.get_error_string() );

      if (PISA_ERR_FILEOPEN != err.get_error_id())
	reply = SCAN_ERROR;
    }

  return reply;
}

/*------------------------------------------------------------*/
bool
view_manager::scan_gimp (int *cancel, bool first_time_around)
{

#ifndef HAVE_ANY_GIMP

  *cancel = 1;			// can't scan to GIMP
  return true;

#else
  bool error = false;		// be optimistic
  try
    {
      *cancel = 0;

      bool reset_params = false;
      if (wait_for_button ()
	  && m_scanmanager_cls->is_button_pressed ()) {
	// We have an impatient user here who pressed the scanner's
	// button *before* we even got a chance to show our WAITING
	// message.
        m_scanmanager_cls->clear_button_status ();
	reset_params = true;
      }

      _feedback->set_text (wait_for_button ()
			   ? progress_window::WAITING
			   : progress_window::WARMING_UP);
      _feedback->set_progress (0, 1);
      _feedback->show ();

      if (wait_for_button ()) {
	long usec = m_scanmanager_cls->get_polling_time ();
	while (!m_scanmanager_cls->is_button_pressed ()
	       && !_feedback->is_cancelled ()) {
	  microsleep (usec);
	  while (gtk_events_pending ()) {
	    gtk_main_iteration ();
	  }
	}
	if (_feedback->is_cancelled ()) {
	  *cancel = 1;
	  return true;
	}
	if (m_scanmanager_cls->push_button_needs_polling ()) {
	  m_scanmanager_cls->disable_wait_for_button();
	}
      }

      _feedback->set_text (progress_window::WARMING_UP);

      int width, height;
      m_scanmanager_cls->init_scan (&width, &height,
				    first_time_around || reset_params);

      while ( ::gtk_events_pending ( ) )
	::gtk_main_iteration ( );

      char depth = 8;
      int rowbytes;
      switch (m_set.imgtype.pixeltype)
	{
	case PISA_PT_RGB:
	  rowbytes = width * 3;
	  break;
	case PISA_PT_GRAY:
	  rowbytes = width;
	  break;
	case PISA_PT_BW:
	  depth	   = 1;
	  rowbytes = (width + 7) / 8;
	  break;
	default:
	  rowbytes = 0;
	}

      gimp_scan gimp_cls;
      if (PISA_ERR_SUCCESS !=
	  gimp_cls.create_gimp_image (width, height,
				      m_set.imgtype.pixeltype, depth))
	{
	  m_scanmanager_cls->acquire_image ( 0, 0, 1, 1 );
	  
	  throw pisa_error ( PISA_ERR_OUTOFMEMORY );
	}

      for (int i = 0; i < height; i++)
	{
	  m_scanmanager_cls->acquire_image ( gimp_cls.get_next_buf ( ),
					     rowbytes,
					     1,
					     *cancel );

	  if ( i == 0 )
	    _feedback->set_text (progress_window::SCANNING);

	  if (*cancel)
	    {
	      error = true;
	      break;
	    }

	  _feedback->set_progress (i, height);
	  *cancel = _feedback->is_cancelled ();

	  gimp_cls.set_image_rect ( );

	  while ( ::gtk_events_pending ( ) )
	    ::gtk_main_iteration ( );
	}
      m_scanmanager_cls->acquire_image (0, 1, 1, *cancel);
      _feedback->set_progress (height, height);

      gimp_cls.finish_scan ( *cancel );
    }
  catch ( pisa_error & err )
    {
      error = true;
      *cancel = dialog_reply( err );
    }

  m_scanmanager_cls->release_memory ();
  
  return error;
#endif	// HAVE_ANY_GIMP
}

void
view_manager::scan_file (iscan::imgstream& is, int *status,
			 bool first_time_around)
{
  try
    {
      bool reset_params = false;
      if (wait_for_button ()
	  && m_scanmanager_cls->is_button_pressed ()) {
	// We have an impatient user here who pressed the scanner's
	// button *before* we even got a chance to show our WAITING
	// message.
        m_scanmanager_cls->clear_button_status ();
	reset_params = true;
      }

      _feedback->set_text (wait_for_button ()
			   ? progress_window::WAITING
			   : progress_window::WARMING_UP);
      _feedback->set_progress (0, 1);
      _feedback->show ();

      while (::gtk_events_pending())
	::gtk_main_iteration();

      if (wait_for_button ()) {
	long usec = m_scanmanager_cls->get_polling_time ();
	while (!m_scanmanager_cls->is_button_pressed ()
	       && !_feedback->is_cancelled ()) {
	  microsleep (usec);
	  while (gtk_events_pending ()) {
	    gtk_main_iteration ();
	  }
	}
	if (_feedback->is_cancelled ()) {
          // user clicked the Finish button!
          *status |= SCAN_FINISH;
          *status &= ~SCAN_NEXT;
          return;
	}
	if (m_scanmanager_cls->push_button_needs_polling ()) {
	  m_scanmanager_cls->disable_wait_for_button();
	}
      }

      _feedback->set_text (progress_window::WARMING_UP);

      int width, height;
      m_scanmanager_cls->init_scan (&width, &height,
				    first_time_around || reset_params);

      while (::gtk_events_pending())
	::gtk_main_iteration();

      int rowbytes;
      iscan::colour_space cs;

      switch (m_set.imgtype.pixeltype)
	{
	case PISA_PT_RGB:
	  rowbytes = width * 3;
	  cs = iscan::RGB;
	  break;
	case PISA_PT_GRAY:
	  rowbytes = width;
	  cs = iscan::gray;
	  break;
	case PISA_PT_BW:
	  rowbytes = (width + 7) / 8;
	  cs = iscan::mono;
	  break;
	default:
	  rowbytes = 0;
	  throw pisa_error (PISA_ERR_PARAMETER);
	}

      try
        {
          is.size (width, height);
          is.depth (PISA_PT_BW == m_set.imgtype.pixeltype ? 1 : 8);
          is.colour (cs);
          is.resolution (m_set.resolution, m_set.resolution);
        }
      catch (pisa_error& oops)
	{
	  m_scanmanager_cls->acquire_image( 0, 0, 1, 1 );
	  throw oops;
	}

      unsigned char *img = new unsigned char[rowbytes];
      for (int i = 0; i < height; ++i)
	{
          m_scanmanager_cls->acquire_image (img, rowbytes, 1,
                                            *status & SCAN_CANCEL);

	  if (0 == i)
	    _feedback->set_text (progress_window::SCANNING);

	  if (*status & SCAN_CANCEL)
	    {
              //error = true;
	      break;
	    }

	  _feedback->set_progress (i, height);
          if (_feedback->is_cancelled ())
            *status |= SCAN_CANCEL;

          try
            {
              try
                {
                  is.write ((const char *)img, rowbytes);
                  *status |= SCAN_DATA;
                }
              catch (std::exception& oops)
                {               // map to old API and rethrow
                  throw (pisa_error (PISA_ERR_OUTOFMEMORY));
                }
            }
	  catch (pisa_error& oops)
	    {
	      *status |= SCAN_CANCEL;

	      if (i < height)
                m_scanmanager_cls->acquire_image (img, rowbytes, 1,
                                                  *status & SCAN_CANCEL);
	      aleart_dialog aleart_dlg;
	      aleart_dlg.message_box( m_main_cls->get_widget(),
				      oops.get_error_string() );
	      break;
	    }
	  while (::gtk_events_pending())
	    ::gtk_main_iteration();
	}
      delete[] img;

      m_scanmanager_cls->acquire_image (0, 1, 1, *status & SCAN_CANCEL);
      _feedback->set_progress (height, height);
    }
  catch (pisa_error& oops)
    {
      //error = true;
      *status |= dialog_reply (oops);
    }

  m_scanmanager_cls->release_memory ();

  while (::gtk_events_pending())
    ::gtk_main_iteration();

  return;
}

void
view_manager::print (const std::string& filename) const
{
  if (PISA_DE_PRINTER == m_set.destination)
    {
      char cmd[1024];           // FIXME: buffer overflow!
      sprintf (cmd, "%s %s", m_config_cls->m_cmd, filename.c_str ());
      system (cmd);             // FIXME: check cmd exit status

      while (gtk_events_pending ()) gtk_main_iteration ();

      remove (filename.c_str ());
    }
}

bool
view_manager::is_multi_image (void) const
{
  return (m_scanmanager_cls->using_adf () || m_set.enable_start_button);
}

bool
view_manager::wait_for_button (void) const
{
  if (m_scanmanager_cls->push_button_needs_polling ())
  {
    return (m_set.enable_start_button);
  }
  else
  {
    return (m_set.enable_start_button && !m_scanmanager_cls->using_adf ());
  }
}

void
view_manager::set_resolution (long resolution)
{
  long adjust_res[2] = {resolution, resolution};

  m_set.resolution = resolution;

  /* set resolution to backend */
  m_scanmanager_cls->get_valid_resolution (&adjust_res[0], &adjust_res[1], true);
  m_scanmanager_cls->set_scan_resolution (adjust_res[0], adjust_res[1]);
}

void
view_manager::set_image_type (const imagetype *type)
{
  memcpy (&m_set.imgtype, type, sizeof (imagetype));
  m_scanmanager_cls->set_color_mode(type->pixeltype, type->bitdepth);
}

int
view_manager::microsleep (size_t usec)
{
  struct timespec ts;
  ts.tv_sec  =  usec / 1000000;
  ts.tv_nsec = (usec % 1000000) * 1000;

  return nanosleep (&ts, NULL);
}
