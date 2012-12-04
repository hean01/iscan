/*
   SANE EPSON backend
   Copyright (C) 2003, 2008, 2009  SEIKO EPSON CORPORATION

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
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <regex.h>
#ifdef __cplusplus
extern "C" {
#endif
#include <sane/sane.h>
#include <sane/saneopts.h>
#ifdef __cplusplus
}
#endif
#include "pisa_error.h"
#include "pisa_main.h"
#include "pisa_scan_selector.h"
#include "pisa_view_manager.h"
#include "pisa_gimp.h"
/*------------------------------------------------------------*/

static void
_delete( GtkWidget *widget, scan_selector *ss )
{
  ss->cancel();			// same process as cancel
}

static void
_select( GtkEditable *, scan_selector *ss )
{
  ss->select();			// just forward the call
}

static void
_update( GtkButton *, scan_selector *ss )
{
  ss->update();			// just forward the call
}

static void
_cancel( GtkButton *, scan_selector *ss )
{
  ss->cancel();			// just forward the call
}


scan_selector::~scan_selector()
{
  free( _current_gui );
  free( _current_dev );
  g_slist_free( _gui_names );
  g_slist_free( _dev_names );
  g_slist_free( _items );
  ::gtk_widget_destroy( m_dbox );
}

scan_selector::scan_selector( bool is_dialog )
  : m_local_only( SANE_FALSE ),
    m_dbox( 0 ), m_hbox( 0 ), m_menu( 0 ), _items( 0 ),
    _dev_names( 0 ), _gui_names( 0 ),
    _current_dev( 0 ), _current_gui( 0 )
{
  GtkWidget *vbox = 0;		// dialog container

  if (is_dialog)
    {
      {				// dialog box
	GtkWidget *w = ::gtk_dialog_new ();
	::gtk_object_set_data( GTK_OBJECT( w ), "m_dbox", w );
	::gtk_window_set_title( GTK_WINDOW( w ),
				_("Scan Selector Dialog") );

#ifdef HAVE_GTK_2
	GTK_WINDOW( w )->type = GTK_WINDOW_TOPLEVEL;
#else
	GTK_WINDOW( w )->type = GTK_WINDOW_DIALOG;
#endif
	::gtk_window_set_modal( GTK_WINDOW( w ), TRUE );
	::gtk_window_set_policy( GTK_WINDOW( w ), FALSE, TRUE, FALSE );

	::gtk_signal_connect( GTK_OBJECT( w ), "delete_event",
			      GTK_SIGNAL_FUNC( ::_delete ), this );

	m_dbox = w;
      }

      {
	GtkWidget *w = GTK_DIALOG( m_dbox )->vbox;
	::gtk_object_set_data( GTK_OBJECT( m_dbox ), "vbox", w );
	::gtk_container_set_border_width( GTK_CONTAINER( w ), 5 );
	::gtk_widget_show( w );

	vbox = w;
      }
    }

  {				// selector container
    GtkWidget *w = ::gtk_hbox_new( FALSE, 5 );
    ::gtk_widget_ref( w );
    ::gtk_object_set_data_full( GTK_OBJECT( w ), "m_hbox", w,
				(GtkDestroyNotify) ::gtk_widget_unref );
    if (vbox)			// only when in a dialog
      ::gtk_box_pack_start( GTK_BOX( vbox ), w, TRUE, TRUE, 0 );
    ::gtk_container_set_border_width( GTK_CONTAINER( w ), 5 );

    m_hbox = w;
  }
  
  {				// scanner label
    GtkWidget *w = ::gtk_label_new (_("Scanner:"));
    ::gtk_box_pack_start( GTK_BOX( m_hbox ), w, FALSE, FALSE, 0 );
    ::gtk_widget_ref( w );
    ::gtk_object_set_data_full( GTK_OBJECT (m_hbox), "label", w,
				(GtkDestroyNotify) ::gtk_widget_unref );
    ::gtk_widget_show( w );
  }
  
  {				// option menu
    GtkWidget *w = ::gtk_option_menu_new ();
    ::gtk_box_pack_start( GTK_BOX( m_hbox ), w, FALSE, FALSE, 0 );
    ::gtk_widget_ref( w );
    ::gtk_object_set_data_full( GTK_OBJECT( m_hbox ), "option", w,
				(GtkDestroyNotify) ::gtk_widget_unref );
    ::gtk_widget_show( w );

    m_opts = w;
  }
  
  if (!is_dialog)
    {				// update button
      GtkWidget *w = ::gtk_button_new_with_label( _("  Update  ") );
      ::gtk_container_set_border_width( GTK_CONTAINER( w ), 5 );
      ::gtk_box_pack_start( GTK_BOX( m_hbox ), w, FALSE, FALSE, 0 );
      ::gtk_widget_ref( w );
      ::gtk_object_set_data_full( GTK_OBJECT( m_hbox ), "update", w,
				  (GtkDestroyNotify) ::gtk_widget_unref );
      ::gtk_widget_show( w );

      ::gtk_signal_connect( GTK_OBJECT( w ), "clicked",
			    GTK_SIGNAL_FUNC( ::_update ), this );
    }
  else
    {
      GtkWidget *action_area;
      {				// dialog action button area
	GtkWidget *w = GTK_DIALOG (m_dbox)->action_area;
	::gtk_object_set_data( GTK_OBJECT( m_dbox ), "action_area", w );
	::gtk_container_set_border_width( GTK_CONTAINER( w ), 5 );
	::gtk_widget_show( w );

	action_area = w;
      }

      GtkWidget *button_box;
      {
	GtkWidget *w = gtk_hbutton_box_new();
	gtk_widget_ref( w );
	::gtk_object_set_data_full( GTK_OBJECT( m_dbox ), "button_box", w,
				    (GtkDestroyNotify) gtk_widget_unref );
	::gtk_box_pack_start( GTK_BOX( action_area ), w, TRUE, FALSE, 0 );
	::gtk_widget_show( w );

	button_box = w;
      }
      
      {				// okay button
	GtkWidget *w = ::gtk_button_new_with_label( _("  OK  ") );
	::gtk_widget_ref( w );
	::gtk_object_set_data_full( GTK_OBJECT( m_dbox ), "okay_button", w,
				    (GtkDestroyNotify) gtk_widget_unref );
	::gtk_box_pack_start( GTK_BOX( button_box ), w, TRUE, FALSE, 0 );
	GTK_WIDGET_SET_FLAGS( w, GTK_CAN_DEFAULT );
	::gtk_widget_grab_default( w );
	::gtk_widget_show( w );

	::gtk_signal_connect( GTK_OBJECT( w ), "clicked",
			      GTK_SIGNAL_FUNC( ::_select ), this );
      }
  
      {				// cancel button
	GtkWidget *w = ::gtk_button_new_with_label( _("  Cancel  ") );
	::gtk_widget_ref( w );
	::gtk_object_set_data_full( GTK_OBJECT( m_dbox ), "cancel_button", w,
				    (GtkDestroyNotify) gtk_widget_unref );
	::gtk_box_pack_start( GTK_BOX( button_box ), w, TRUE, FALSE, 0 );
	::gtk_widget_show( w );

	::gtk_signal_connect( GTK_OBJECT( w ), "clicked",
			      GTK_SIGNAL_FUNC( ::_cancel ), this );
      }
    }
}

void
scan_selector::show() const
{
  gtk_widget_show( m_hbox );
  if (m_dbox)
    gtk_widget_show( m_dbox );
}

GtkWidget *
scan_selector::widget() const
{
  return (m_dbox ? m_dbox : m_hbox);
}

void
scan_selector::update()
{
  const SANE_Device **device;
  SANE_Status status;
  status = sane_get_devices ( &device, m_local_only );

  if (SANE_STATUS_GOOD != status)
    throw pisa_error( status );

  // prepare a new menu
  GtkWidget *menu = gtk_menu_new();
  GSList *items = 0;
  GSList *dev_names = 0;
  GSList *gui_names = 0;

  int cnt = 0;
  while (*device)
    {
      char *name = rewrite_name( *device );
      if (name)
	{
	  GtkWidget *item = gtk_menu_item_new ();
          GtkWidget *label = gtk_label_new( name );
          gtk_label_set_line_wrap( GTK_LABEL( label ), TRUE );
          gtk_container_add( GTK_CONTAINER(item), label );
	  ++cnt;

	  if (!m_dbox)
	    gtk_signal_connect( GTK_OBJECT( item ), "activate",
				GTK_SIGNAL_FUNC( _select ), this );

	  items     = g_slist_append( items, item );
	  dev_names = g_slist_append( dev_names,
				      (void *) ((*device)->name) );
	  gui_names = g_slist_append( gui_names, (void *) name );

	  gtk_menu_append( GTK_MENU( menu ), item );
          gtk_widget_show( label );
	  gtk_widget_show( item );
	}
      ++device;
    }

  if (!cnt)
    throw pisa_error( PISA_ERR_CONNECT );

  if (m_menu)			// get rid of the old menu
    gtk_option_menu_remove_menu( GTK_OPTION_MENU( m_opts ) );

  m_menu = menu;
  gtk_option_menu_set_menu( GTK_OPTION_MENU( m_opts ), m_menu );

  if (_items)
    {
      GSList *item;
      if (!m_dbox)
	for (item = _items; item; item = item->next )
	  gtk_signal_disconnect_by_func( GTK_OBJECT( item->data ),
					 GTK_SIGNAL_FUNC( _select ), this );
      g_slist_free( _items );
    }
  if (_dev_names)
    g_slist_free( _dev_names );
  if (_gui_names)
    g_slist_free( _gui_names );

  _items = items;
  _dev_names = dev_names;
  _gui_names = gui_names;

  if (!_current_dev)
    {
      char *name   = (char *) _dev_names->data;
      _current_dev = (char *) malloc( strlen( name ) + 1 );
      if (!_current_dev)
	throw pisa_error( PISA_STATUS_NO_MEM );
      strcpy( _current_dev, name );
    }
  if (!_current_gui)
    {
      char *name   = (char *) _gui_names->data;
      _current_gui = (char *) malloc( strlen( name ) + 1 );
      if (!_current_gui)
	throw pisa_error( PISA_STATUS_NO_MEM );
      strcpy( _current_gui, name );
    }
}

void
scan_selector::cancel()
{
  ::gtk_main_quit();
  sane_exit ();
  if( pisa_gimp_plugin() )
    pisa_gimp_quit();
  exit( 0 );
}

void
scan_selector::select()
{
  GtkWidget *item = gtk_menu_get_active( GTK_MENU( m_menu ) );

  gint pos = g_slist_position( _items, g_slist_find( _items, item ) );
  if (-1 == pos)
    {
      fprintf( stderr, "scan_selector: internal inconsistency\n" );
      abort();
    }

  char *dev_name = (char *) g_slist_nth_data( _dev_names, (guint) pos );
  char *gui_name = (char *) g_slist_nth_data( _gui_names, (guint) pos );

  if (0 != strcmp( _current_dev, dev_name ))
    {
      char *new_name = (char *) malloc( (strlen( dev_name ) + 1)
					* sizeof( char ) );
      if (!new_name)
	throw pisa_error( PISA_STATUS_NO_MEM );

      strcpy( new_name, dev_name );
      free( _current_dev );
      _current_dev = new_name;
    }
  if (0 != strcmp( _current_gui, gui_name ))
    {
      char *new_name = (char *) malloc( (strlen( gui_name ) + 1)
					* sizeof( char ) );
      if (!new_name)
	throw pisa_error( PISA_STATUS_NO_MEM );

      strcpy( new_name, gui_name );
      free( _current_gui );
      _current_gui = new_name;
    }

  if (m_dbox)
    {
      ::gtk_widget_hide( m_dbox );
      ::gtk_main_quit();
    }
  else
    g_view_manager->set_device( dev_name );
}

char *
scan_selector::get_device( bool rewrite )
{
  if (m_dbox && !rewrite)
    {
      update();
      if (1 < g_slist_length( _items ))
	{
	  show();
	  ::gtk_grab_add( m_dbox );
	  ::gtk_main();
	  ::gtk_grab_remove( m_dbox );
	}
    }
  return (rewrite
	  ? _current_gui	// GUI device name
	  : _current_dev);	// raw device name
}

char *
scan_selector::rewrite_name( const SANE_Device *device )
{
  if (!device)
    return 0;

  if (0 != strcmp( device->vendor, "Epson" ))
    return 0;			// sorry 'bout that, everyone ;-}

  char *name = const_cast< char * >( device->name );
  if (!name)
    return 0;

				// disable host-base scanners accessed
				// through the net backend (they crash
				// way too often)
  if (0 == strncmp( name, "net:", strlen( "net:" ) )
      && (   0 == strcmp( device->model, "GT-7200" )
	  || 0 == strcmp( device->model, "GT-7300" )
	  || 0 == strcmp( device->model, "Perfection1250" )
	  || 0 == strcmp( device->model, "Perfection1260" )
	  ))
    return 0;

  {				// dropping other backend EPSON devices
    regex_t *comp_regex = new regex_t;
    const char *regex = "^(net:([^:]+:))?(epson|epson2|plustek|snapscan):(.*)$";
    int comp = regcomp( comp_regex, regex, REG_EXTENDED );

    if (0 == comp)
      {
	int result = regexec( comp_regex, device->name, 0, 0, 0 );
	if (0 == result)
	  {
	    name = 0;		// other backend controlled device
	  }
	else
	  if (REG_NOMATCH != result)
	    regerror( comp, comp_regex );
      }
    else
      regerror( comp, comp_regex );

    regfree( comp_regex );
    delete comp_regex;

    if (!name)
      return 0;			// sorry 'bout that everyone ;-}
  }

  // if we are still here we have an epkowa backend supported EPSON
  // scanner OR, heaven forbid, an aliased scanner

  {
    // rewrite things into a string of the following form:
    //   "device->vendor device->model [device->name]"

    size_t length = (strlen (device->vendor) + 1       // " "
                     + strlen (device->model) + 2      // " ["
                     + strlen (device->name) + 1 + 1); // "]" + \0

    name = (char *) malloc( length * sizeof( char ) );
    if (!name)
      throw pisa_error( PISA_STATUS_NO_MEM );

    strncpy (name, device->vendor, strlen (device->vendor));

    char *c = name + strlen (device->vendor);
    *c++ = ' ';
    {
      const char *p = device->model;
      while (*p)
        *c++ = *p++;
    }
    *c++ = ' ';
    *c++ = '[';
    {			// copy device name
      const char *p = device->name;
      while (*p)
        *c++ = *p++;
    }
    *c++ = ']';
    *c = '\0';
  }

  return name;
}

void
scan_selector::regerror( int code, regex_t *regex )
{
  size_t length = ::regerror( code, regex, 0, 0 );
  char *message = new char[length];

  ::regerror( code, regex, message, length );
  fprintf( stderr, "%s\n", message );

  delete[] message;
}
