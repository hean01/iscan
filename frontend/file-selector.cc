/* file-selector.cc -- customized file selection dialog
   Copyright (C) 2003, 2005, 2008  SEIKO EPSON CORPORATION

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "file-selector.h"
#include "gettext.h"
#define  _(msg_id)	gettext (msg_id)

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "pisa_aleart_dialog.h"
#include "pisa_enums.h"
#include "pisa_error.h"

#include "../lib/pcxstream.hh"
#include "../lib/pnmstream.hh"
#include "../lib/pngstream.hh"
#include "../lib/jpegstream.hh"
#include "../lib/tiffstream.hh"
#include "../lib/pdfstream.hh"

#ifndef DEBUG
#define g_print(...)
#endif


#ifdef DONT_HAVE_GTK_2
#undef HAVE_GTK_2
#endif

#ifndef HAVE_GTK_2
#define G_CALLBACK  GTK_SIGNAL_FUNC
#define g_signal_stop_emission_by_name gtk_signal_emit_stop_by_name
#define g_signal_connect_swapped gtk_signal_connect_object
#define g_signal_connect gtk_signal_connect
#define GTK1_OBJ(obj) GTK_OBJECT (obj)
GtkWidget *			// "stolen" from GTK+2.0
gtk_widget_get_parent (GtkWidget *widget)
{
  g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

  return widget->parent;
}
#else
#define GTK1_OBJ(obj) obj
#endif

struct menu_info		// helper struct for callbacks
{
  file_selector *fs;
  GtkWidget     *widget[4];     // holding "ADF" spinboxes and labels
  int            index;
};


// callback implementations

static void
_destroy (file_selector *fs)
{
  g_print ("%s\n", __func__);

  fs->destroy (true);
}

#if 0
static void
_click_ng( GtkWidget *, file_selector * )
{
  DBG_FS fprintf( stderr, "_click_ng\n" );

  gtk_main_quit();
}
#endif

static void
_click_ok (file_selector *fs)
{
  g_print ("%s\n", __func__);

  if (fs->save_pathname ())
    {
      fs->destroy ();
      gtk_main_quit ();
    }
}

// FIXME: this really should be using image stream capability API to
//        determine what is and what is not supported.
static void
_change_format (GtkWidget *, struct menu_info *id)
{
  id->fs->change_format( id->index ); // fs->get_type() below, requires this

  // adjust "file with all pages" check box
  bool make_active = (iscan::PDF == id->fs->get_type () ||
                      iscan::TIF == id->fs->get_type ());
  gtk_widget_set_sensitive (id->widget[0], make_active);
  if (!make_active) // disable multi-page mode when changing to a format that
                    // does not support it, do not do the opposite!
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (id->widget[0]), false);
    }

  // adjust "binding edge" selector sensitivity
  // We only support this for file formats (i.e. `imgstream` classes)
  // that support "easy" rotation.  Yes, this is utterly flawed.  The
  // "binding edge" selector _should_ be in the main part of the GUI.
  // It has no dependency on saving to file whatsoever, it is an issue
  // that depends on device capabilities and document characteristics
  // only.
  make_active = (iscan::PDF == id->fs->get_type ());
  gtk_widget_set_sensitive (id->widget[1], make_active);
  gtk_widget_set_sensitive (id->widget[2], make_active);
  gtk_widget_set_sensitive (id->widget[3], make_active);
}

static void
_delete_text (GtkEditable *ed, gint start, gint end, file_selector *fs)
{
  g_print ("%s\n", __func__);

#ifndef HAVE_GTK_2
  gtk_signal_handler_block_by_func (GTK_OBJECT (ed), 
				    GTK_SIGNAL_FUNC (_delete_text), fs);
  fs->delete_text (start, end);
  gtk_signal_handler_unblock_by_func (GTK_OBJECT (ed),
				      GTK_SIGNAL_FUNC (_delete_text), fs);
#else
  g_signal_handlers_block_by_func (ed, (gpointer) _delete_text, fs);
  fs->delete_text (start, end);
  g_signal_handlers_unblock_by_func (ed, (gpointer) _delete_text, fs);
#endif

  g_signal_stop_emission_by_name (GTK1_OBJ(ed), "delete-text"); 
}

static void
_insert_text (GtkEditable *ed, gchar *text, gint len, gint *pos,
	      file_selector *fs)
{
  g_print ("%s\n", __func__);

#ifndef HAVE_GTK_2
  gtk_signal_handler_block_by_func (GTK_OBJECT (ed),
				    GTK_SIGNAL_FUNC ( _insert_text), fs);
  fs->insert_text (text, len, pos);
  gtk_signal_handler_unblock_by_func (GTK_OBJECT (ed),
				      GTK_SIGNAL_FUNC (_insert_text), fs);
#else
  g_signal_handlers_block_by_func (ed, (gpointer) _insert_text, fs);
  fs->insert_text (text, len, pos);
  g_signal_handlers_unblock_by_func (ed, (gpointer) _insert_text, fs);
#endif

  g_signal_stop_emission_by_name (GTK1_OBJ(ed), "insert-text"); 
}

static void
_change_seqnum (GtkAdjustment *, file_selector *fs)
{
  fs->change_seqnum ();
}

static void
_change_digits (GtkAdjustment *, file_selector *fs)
{
  fs->change_digits ();
}

static void
_toggled_multi_page_mode_fs (GtkWidget *widget, file_selector *fs)
{
  fs->change_page_mode (gtk_toggle_button_get_active (
                        GTK_TOGGLE_BUTTON (widget)));
}

static void
_changed_binding_edge (GtkWidget *widget, file_selector *fs)
{
  fs->change_binding_edge (gtk_toggle_button_get_active (
                           GTK_TOGGLE_BUTTON (widget)));
}
static void
_toggled_multi_page_mode_widget (GtkWidget *widget, GtkWidget *p)
{
  bool on = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
  gtk_widget_set_sensitive (p, !on);
}

// 	class implementation

using std::string;
using std::stringstream;
using iscan::file_opener;

			 	// define static variables
const file_selector::format
file_selector::_format[]  = {
  { "PCX", 0, "pcx", "pcx", iscan::PCX },
  { "PNM", 0, "pnm", "pnm", iscan::PNM },
  { "PNG", 0, "png", "png", iscan::PNG },
  { "JPEG", 0, "jpg", "jpg|jpeg", iscan::JPG },
  { "TIFF", 0, "tiff", "tif|tiff", iscan::TIF },
  { "PDF", 0, "pdf", "pdf", iscan::PDF },
  { 0, 0, 0, 0, iscan::PNM }    // array terminator
};

void
file_selector::init ()
{
  g_print ("%s\n", __func__);

  _widget   = NULL;
  _filename = NULL;
  _pathname = NULL;

  _using_adf = false;
  _format_index = 0;
  _deleted_all = false;
  _multi_page_mode = false;
  _has_left_edge_binding = true;

  _number = -1;
  _seqnum = NULL;
  _digits = NULL;

  int size = 1;			// terminating { 0 } element
  for (int i = 0; 0 != _format[i].name; ++i)
    ++size;
  _fmt = new const format * [size];
				// okay, so we may waste a few

  // Looks like the idea is to only add supported formats to _fmt and
  // leave unsupported formats out.
  int cnt = 0;
  int len = 0;
  for (int i = 0; i < size - 1; ++i)
    {
      _fmt[cnt] = &_format[i];	// FIXME: check support
      len += strlen (_fmt[cnt]->rgx);
      ++len;			// for the '|'
      ++cnt;
    }
  _fmt[cnt] = 0;		// array terminator

  _ext_regex = new char[len];	// overwrite last '|' with '\0'
  char *pos = _ext_regex;
  for (int i = 0; _fmt[i]; ++i)
    {
      strcpy (pos, _fmt[i]->rgx);
      pos += strlen (_fmt[i]->rgx);
      *pos++ = '|';		// overwrite '\0', then advance
    }
  *--pos = '\0';		// go back one, then terminate

  g_print ("%s: ext_regex =`%s'\n", __func__, _ext_regex);
}

void
file_selector::create_window (GtkWindow *window, GtkWidget *parent,
			      bool do_consecutive_scanning,
			      bool do_duplex,
			      const char* default_format)
{
  g_print ("%s: enter\n", __func__);

  _parent    = parent;
  _using_adf = do_consecutive_scanning;
  _do_duplex = do_duplex;

  _widget = GTK_FILE_SELECTION (gtk_file_selection_new (PACKAGE));

  if (!_widget)
    throw pisa_error( PISA_STATUS_NO_MEM );

  gtk_window_set_transient_for (GTK_WINDOW (_widget), window);
  gtk_file_selection_hide_fileop_buttons (_widget);

  gint index = 0;
  if (is_supported (default_format))
    index = get_index (default_format);
  else if (is_supported ("PNG"))
    index = get_index ("PNG");

  add_dialog_extensions (index);
				// needs to be called before we canonize

  if (!_filename)
    _filename = canonize ("default");  

  free (_pathname);
  _pathname = 0;
				// FIXME? 0 == _filename
  gtk_file_selection_set_filename (_widget, _filename);

  g_signal_connect_swapped (GTK1_OBJ (_widget->ok_button), "clicked",
			    G_CALLBACK (_click_ok), GTK1_OBJ (this));
  g_signal_connect_swapped (GTK1_OBJ (_widget->cancel_button), "clicked",
			    G_CALLBACK (_destroy), GTK1_OBJ (this));
  g_signal_connect_swapped (GTK1_OBJ (_widget), "delete_event",
			    G_CALLBACK (_destroy), GTK1_OBJ (this));
  g_signal_connect_swapped (GTK1_OBJ (_widget), "destroy",
			    G_CALLBACK (gtk_main_quit), GTK1_OBJ (_widget));

  // We need to interfere with users selecting and editing filenames
  // to guarantee a correct extension and the appropriate templating
  // bit when _using_adf.
  g_signal_connect (GTK1_OBJ (_widget->selection_entry), "delete-text",
		    G_CALLBACK (_delete_text), this);
  g_signal_connect (GTK1_OBJ (_widget->selection_entry), "insert-text",
		    G_CALLBACK (_insert_text), this);

  gtk_widget_show (GTK_WIDGET (_widget));
  gtk_grab_add (GTK_WIDGET (_widget));
  gtk_main ();
  if (_widget)
    {
      gtk_grab_remove (GTK_WIDGET (_widget));
    }
  g_print ("%s: exit\n", __func__);
}

void
file_selector::destroy ()
{
  g_print ("%s\n", __func__);

  if (_using_adf)
    {
      hide ();
    }
  else
    {
      destroy (true);
    }
}

void
file_selector::destroy (bool really)
{
  g_print ("%s (%i)\n", __func__, really);

  free (_filename);
  _filename = NULL;

  if (_widget)
    {
      gtk_widget_destroy (GTK_WIDGET (_widget));
    }
  _widget = NULL;

  if (_parent)
    {
      gtk_widget_set_sensitive (_parent, true);
    }
  _parent = NULL;

  _number = -1;
  _seqnum =  NULL;		// gtk_widget_destroy cleans these up
  _digits =  NULL;

  _using_adf = false;
  _do_duplex = false;
  _deleted_all = false;

  delete [] _fmt;
  _fmt = NULL;
  delete [] _ext_regex;
  _ext_regex = NULL;
}

char *
file_selector::get_pathname () const
{
  return (_pathname ? strdup (_pathname) : NULL);
}

int
file_selector::get_sequence_number () const
{
  return _number;
}

bool
file_selector::valid_pathname (const string& path) const
{
  string::size_type slash = path.rfind (file_opener::dir_sep);
  string name = path.substr (slash + 1);
  string::size_type dot = name.rfind (file_opener::ext_sep);

  string::size_type cmp_pos = dot;

  if (_using_adf && !_multi_page_mode)
    {
      string::size_type dash = name.find_last_of ("-", dot - 1);
      cmp_pos = dash;
    }

  if (128 < path.size ()	// blame the spec for this one
      || !permission (path.c_str ())
      || 0 >= cmp_pos) // 0 length filename
    {
      return false;
    }

  return true;
}

bool
file_selector::save_pathname()
{
  g_print ("%s\n", __func__);

  if (!_widget)
    {
      g_print ("%s: widget's gone!\n", __func__);
      free (_pathname);
      _pathname = NULL;
      return true;
    }

  const char *current = gtk_file_selection_get_filename (_widget);

  if (!valid_pathname (current))
    {
      show_message (pisa_error (PISA_ERR_FILENAME));
      return false;
    }
  if (possible_overwrite (current))
    {
      bool ok = show_message (pisa_error (PISA_ERR_OVERWRITE),
                              _("Overwrite"), _("Cancel"));
      if (!ok)
	return false;
    }

  char *new_name = (char *) malloc ((strlen (current) + 1)
				    * sizeof (char));
  if (!new_name)
    throw pisa_error (PISA_STATUS_NO_MEM);

  strcpy (new_name, current);
  free (_pathname);
  _pathname = new_name;
  free (_filename);
  _filename = 0;

  g_print ("%s: pathname = `%s'\n", __func__, _pathname);

  return true;
}

void
file_selector::hide () const
{
  g_print ("%s\n", __func__);

  if (_using_adf)
    {
      g_print ("%s: calling gtk_widget_hide\n", __func__);
      if (_widget)
	{
	  gtk_widget_hide (GTK_WIDGET (_widget));
	}
      if (_parent)
	{
	  gtk_widget_set_sensitive (_parent, true);
	}
    }
}

void
file_selector::set_entry (const char *text)
{
  g_print ("%s (%s)\n", __func__, text);

  if (!_filename)
    return;			// logical error but return for now

  if (!text
      || !*text			// empty string
      || text == _filename
      || 0 == strcmp (text, _filename))
    return;			// nothing to change

  char *new_name = (canonize (text));
  if (new_name)
    {
      free (_filename);
      _filename = new_name;
#ifndef HAVE_GTK_2
      gtk_signal_handler_block_by_func (GTK_OBJECT (_widget->selection_entry),
					GTK_SIGNAL_FUNC (_delete_text), this);
      gtk_signal_handler_block_by_func (GTK_OBJECT (_widget->selection_entry),
					GTK_SIGNAL_FUNC (_insert_text), this);
      gtk_entry_set_text (GTK_ENTRY (_widget->selection_entry), _filename);
      gtk_signal_handler_unblock_by_func (GTK_OBJECT (_widget->selection_entry),
					  GTK_SIGNAL_FUNC (_insert_text), this);
      gtk_signal_handler_unblock_by_func (GTK_OBJECT (_widget->selection_entry),
					  GTK_SIGNAL_FUNC (_delete_text), this);
#else
      g_signal_handlers_block_by_func (GTK_EDITABLE (_widget->selection_entry),
				       (gpointer) _delete_text, this);
      g_signal_handlers_block_by_func (GTK_EDITABLE (_widget->selection_entry),
				       (gpointer) _insert_text, this);
      gtk_entry_set_text (GTK_ENTRY (_widget->selection_entry), _filename);
      g_signal_handlers_unblock_by_func (GTK_EDITABLE (_widget->selection_entry),
					 (gpointer) _insert_text, this);
      g_signal_handlers_unblock_by_func (GTK_EDITABLE (_widget->selection_entry),
					 (gpointer) _delete_text, this);
#endif
    }
}

void
file_selector::delete_text (gint start, gint end)
{
  g_print ("%s (%d,%d)\n", __func__, start, end);
  g_print ("%s orig _filename '%s'\n", __func__, _filename);

  _deleted_all = ((end - start) == (gint) strlen (_filename));

  if (0 > end) end = strlen (_filename) + 1;
  int dot = strrchr (_filename, ((_using_adf && !_multi_page_mode)
                                 ? '-' : '.')) - _filename;
  if (end > dot) end = dot;

  if (start < end)
    {
      GtkWidget *ed = _widget->selection_entry;
      gtk_editable_delete_text (GTK_EDITABLE (ed), start, end);
      _filename = strdup (gtk_entry_get_text (GTK_ENTRY (ed)));

      g_print ("%s new  _filename '%s'\n", __func__, _filename);
    }
}

void
file_selector::insert_text (gchar *text, gint len, gint *pos)
{
  g_print ("%s (%s,%d,%d)\n", __func__, text, len, (pos ? *pos : -1));
  g_print ("%s orig _filename '%s'\n", __func__, _filename);

  if (_deleted_all && pos && (0 == *pos))
    {
      _deleted_all = false;
      text = canonize (text);
      if (!text) return;
      len  = strlen (text);

      GtkEditable *ed = GTK_EDITABLE (_widget->selection_entry);
#ifndef HAVE_GTK_2
      gtk_signal_handler_block_by_func (GTK_OBJECT (ed),
					GTK_SIGNAL_FUNC (_delete_text), this);
      gtk_editable_delete_text (ed, 0, -1);
      gtk_signal_handler_unblock_by_func (GTK_OBJECT (ed),
					GTK_SIGNAL_FUNC (_delete_text), this);
#else
      g_signal_handlers_block_by_func (ed, (gpointer) _delete_text, this);
      gtk_editable_delete_text (ed, 0, -1);
      g_signal_handlers_unblock_by_func (ed, (gpointer) _delete_text, this);
#endif
    }

  int dot = strrchr (_filename, ((_using_adf && !_multi_page_mode)
                                 ? '-' : '.')) - _filename;

  if (pos && (*pos <= dot))
    {
      GtkWidget *ed = _widget->selection_entry;
      gtk_editable_insert_text (GTK_EDITABLE (ed), text, len, pos);
#ifndef HAVE_GTK_2
      {				// not getting delete-event's for some
				// reason
	char *name = canonize (gtk_entry_get_text (GTK_ENTRY (ed)));
	gtk_entry_set_text (GTK_ENTRY (ed), name);
	free (name);
      }
#endif
      free (_filename);
      _filename = strdup (gtk_entry_get_text (GTK_ENTRY (ed)));

      g_print ("%s new  _filename '%s'\n", __func__, _filename);
    }
}

void
file_selector::change_format (int index)
{
  g_print ("%s (%d)\n", __func__, index);

  _format_index = index;
				// reflect changes in filename
  char *canon = canonize (_filename);
  set_entry (canon);
  free (canon);
}

void
file_selector::change_seqnum ()
{
  g_print ("%s\n", __func__);

  _number = int (_seqnum->value);
}

void
file_selector::change_digits ()
{
  g_print ("%s\n", __func__);

  int max = 9;			// there's at least one digit
  for (int i = 2; i <= int (_digits->value); ++i)
    {
      max *= 10;
      max +=  9;
    }

  if (_seqnum)
    {
      _seqnum->upper = max;
      if (max < _seqnum->value)	// also updates the GUI
	gtk_adjustment_set_value (_seqnum, max);

      gtk_adjustment_changed (_seqnum);
    }

  if (_filename)		// reflect changes in filename
    {
      char *canon = canonize (_filename);
      set_entry (canon);
      free (canon);
    }
}

#include <sstream>
#include <iostream>
void
file_selector::change_page_mode (bool mode)
{
  _multi_page_mode = mode;
  
  string name = _filename;
  stringstream ss;
  
  string::size_type dot  = name.rfind (file_opener::ext_sep);
  string::size_type dash = name.find_last_of ("-", dot - 1);

  if (mode)
    {
      ss << name.substr (0, dash)
         << name.substr (dot);
    }
  else
    {
      ss << name.substr (0, dot) << "-";
      for (int i = 0; i < int (_digits->value); ++i)
        ss << file_opener::hash_mark;
      ss << name.substr (dot);
    }
      
  set_entry (ss.str ().c_str ());
}

void
file_selector::change_binding_edge (bool is_left_edge)
{
  g_print ("%s(%d)\n", __func__, is_left_edge);

  _has_left_edge_binding = is_left_edge;
}

// caller needs to free returned char *
char *
file_selector::canonize (const char *text) const
{
  if (!text || 0 == *text)
    return 0;

  g_print ("%s (%s)\n", __func__, text);

  string name = text;
  string ext = _fmt[_format_index]->ext;
  stringstream ss;

  string::size_type dot = name.rfind (file_opener::ext_sep);

  if (_using_adf && !_multi_page_mode)
    {
      string::size_type dash = name.find_last_of ("-", dot - 1);
      ss << name.substr (0, dash)
         << "-";
      
      for (int i = 0; i < int (_digits->value); ++i)
        ss << file_opener::hash_mark;
    }
  else
    {
      ss << name.substr (0, dot);
    }

  ss << file_opener::ext_sep
     << ext;

  return strdup (ss.str ().c_str ());
}

void
file_selector::add_dialog_extensions (gint index)
{
  g_print ("%s: enter\n", __func__);

  GtkWidget *frame = gtk_frame_new (_("Save Options"));
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_IN);

  GtkWidget *align = gtk_alignment_new (1.0, 0.5, 0.0, 0.0);
  gtk_alignment_set_padding (GTK_ALIGNMENT (align), 5, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), GTK_WIDGET (align));
  gtk_widget_show (align);

  unsigned short rows = 1;
  if (_using_adf) rows = 2;
  if (_do_duplex) rows = 3;
  GtkTable *opts
    = (GtkTable *) gtk_table_new (rows + (_using_adf ? 2 : 0), 3, FALSE);
  gtk_table_set_col_spacings (opts, 5);
  gtk_container_add (GTK_CONTAINER (align), GTK_WIDGET (opts));
  gtk_widget_show (GTK_WIDGET (opts));

  GtkWidget *mi = NULL;
  GtkWidget *multi =
    gtk_check_button_new_with_label ( _("Create file with all pages"));

  GtkWidget *bind_edge = gtk_label_new
    (_("Binding Position (for Double-Sided Scanning)"));
  GtkWidget *left_edge = gtk_radio_button_new_with_label (NULL, _("Left"));
  GtkWidget *top_edge  = gtk_radio_button_new_with_label
    (gtk_radio_button_get_group (GTK_RADIO_BUTTON (left_edge)), _("Top"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (left_edge), true);

  GtkWidget *w = 0;
  {				// file type selector
    w = gtk_label_new (_("Determine File Type:"));
    gtk_table_attach_defaults (opts, w, 0, 1, 0, 1);
    gtk_misc_set_alignment (GTK_MISC (w), 1.0, 0.5);
    gtk_widget_show (w);

    w = gtk_option_menu_new ();
    {
      GtkWidget *m = gtk_menu_new ();

      for (int i = 0; _fmt[i]; ++i)
	{
	  mi = gtk_menu_item_new_with_label (_fmt[i]->name);
	  gtk_menu_append (GTK_MENU (m), mi);
	  gtk_widget_show (mi);

	  struct menu_info *id = new struct menu_info;
	  id->fs = this;
	  id->widget[0] = multi;
	  id->widget[1] = bind_edge;
	  id->widget[2] = left_edge;
	  id->widget[3] = top_edge;
	  id->index = i;

	  g_signal_connect (GTK1_OBJ (mi), "activate",
			    G_CALLBACK (_change_format), (void *) id);

          gtk_widget_set_sensitive (mi, is_supported (_fmt[i]->name));
	}
      gtk_option_menu_set_menu( GTK_OPTION_MENU( w ), m );
    }
    gtk_table_attach_defaults (opts, w, 1, 3, 0, 1);
    gtk_option_menu_set_history (GTK_OPTION_MENU (w), index);
    change_format (index);
    gtk_widget_show (w);
  }

  if (_using_adf)
    {
      g_print ("%s: using adf\n", __func__);

      gtk_signal_connect (GTK_OBJECT (multi), "toggled",
      	            GTK_SIGNAL_FUNC (_toggled_multi_page_mode_fs), this);
      gtk_table_attach_defaults (opts, multi, 1, 3, 1, 2);
      gtk_widget_show (multi);

      if (_do_duplex)
        {
          gtk_signal_connect (GTK_OBJECT (left_edge), "toggled",
                              GTK_SIGNAL_FUNC (_changed_binding_edge), this);
          gtk_table_attach_defaults (opts, bind_edge, 0, 1, 2, 3);
          gtk_misc_set_alignment (GTK_MISC (bind_edge), 1.0, 0.5);
          gtk_table_attach_defaults (opts, left_edge, 1, 2, 2, 3);
          gtk_table_attach_defaults (opts, top_edge, 2, 3, 2, 3);
          gtk_widget_show (bind_edge);
          gtk_widget_show (left_edge);
          gtk_widget_show (top_edge);
        }

 				// sequence number selector
      w = gtk_label_new (_("Start filing at:"));
      gtk_table_attach_defaults (opts, w, 0, 1, rows + 0, rows + 1);
      gtk_misc_set_alignment (GTK_MISC (w), 1.0, 0.5);
      gtk_signal_connect (GTK_OBJECT (multi), "toggled",
              GTK_SIGNAL_FUNC (_toggled_multi_page_mode_widget), w);
      gtk_widget_show (w);

      if (!_seqnum)
	_seqnum = (GtkAdjustment *) gtk_adjustment_new (1, 0, 9, 1, 10, 0);
      if (_seqnum)
	_number = int (_seqnum->value);
      else			// play it safe and make sure
	_number = -1;

      w = gtk_spin_button_new (_seqnum, 0, 0);
      gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (w), true);
      gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (w), true);
      gtk_table_attach_defaults (opts, w, 1, 3, rows + 0, rows + 1);
      gtk_signal_connect (GTK_OBJECT (multi), "toggled",
              GTK_SIGNAL_FUNC (_toggled_multi_page_mode_widget), w);
      gtk_widget_show (w);

				// number of digits selector
      w = gtk_label_new (_("Number of digits:"));
      gtk_table_attach_defaults (opts, w, 0, 1, rows + 1, rows + 2);
      gtk_misc_set_alignment (GTK_MISC (w), 1.0, 0.5);
      gtk_signal_connect (GTK_OBJECT (multi), "toggled",
              GTK_SIGNAL_FUNC (_toggled_multi_page_mode_widget), w);
      gtk_widget_show (w);

      if (!_digits)
	_digits = (GtkAdjustment *) gtk_adjustment_new (3, 1, 6, 1,  1, 0);
      if (_digits)
	change_digits();

      w = gtk_spin_button_new (_digits, 0, 0);
      gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (w), true);
      gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (w), true);
      gtk_table_attach_defaults (opts, w, 1, 3, rows + 1, rows + 2);
      gtk_signal_connect (GTK_OBJECT (multi), "toggled",
              GTK_SIGNAL_FUNC (_toggled_multi_page_mode_widget), w);
      gtk_widget_show (w);

      g_signal_connect (GTK1_OBJ (_seqnum), "value_changed",
			G_CALLBACK (_change_seqnum), this);
      g_signal_connect (GTK1_OBJ (_digits), "value_changed",
			G_CALLBACK (_change_digits), this);
   }

  gtk_box_pack_end (GTK_BOX (_widget->main_vbox), frame, false, false, 0);

  struct menu_info id;
  id.fs = this;
  id.widget[0] = multi;
  id.widget[1] = bind_edge;
  id.widget[2] = left_edge;
  id.widget[3] = top_edge;
  id.index = index;
  _change_format (mi, &id);
  gtk_widget_show (frame);
  gtk_widget_show (GTK_WIDGET (opts));  

  g_print ("%s: exit\n", __func__);
}

bool
file_selector::permission( const char *file_in ) const
{
  if (!file_in || 0 == strlen (file_in))
    return false;

  char* file = strdup (file_in);

  if (!file)
    return false;

  if (0 == access( file, F_OK ))	// file exists
    {
      bool rv = (0 == access( file, W_OK ));	// whether we can write to it
      free (file);
      return rv;
    }

  // check write access to the directory (note that we need execute
  // privileges as well)

  char *slash = strrchr( file, '/');
  *slash = '\0';		// temporarily truncate to dirname
  const char *dir = (file == slash
		     ? "/"	// whoops!, file in root directory
		     : file);

  bool w_ok = false;		// assume the worst
  if (0 == access( dir, F_OK ))
    w_ok = (0 == access( dir, W_OK | X_OK ));

  *slash = '/';			// restore filename

  free (file);
  return w_ok;
}

bool
file_selector::show_message( const pisa_error& oops,
                             const char *yes,
                             const char *no ) const
{
  aleart_dialog dlg;

  if (yes && no)		// binary question
    return (1 == dlg.message_box( GTK_WIDGET( _widget ),
				  oops.get_error_string(),
				  yes, no ));

  dlg.message_box( GTK_WIDGET( _widget ), oops.get_error_string() );
  return true;
}

void
file_selector::regerror (int code, regex_t *regex) const
{
  size_t length = ::regerror (code, regex, 0, 0);
  char *message = new char[length];

  ::regerror (code, regex, message, length);
  fprintf (stderr, "%s\n", message);

  delete[] message;
}

std::string
file_selector::get_format_name () const
{
  return _format[_format_index].name;
}

iscan::file_format
file_selector::get_type () const
{
  return _format[_format_index].type;
}

bool
file_selector::multi_page_mode ()
{
  if (iscan::PDF == get_type () || iscan::TIF == get_type ())
    return _multi_page_mode;

  return false;
}

bool
file_selector::has_left_edge_binding ()
{
  return _has_left_edge_binding;
}

gint
file_selector::get_index (const char* name) const
{
  if (!name) return 0;

  int i = 0;
  while (_format[i].name && (0 != strcmp (name, _format[i].name)))
    {
      ++i;
    }
  return (_format[i].name ? i : 0);
}

bool
file_selector::is_supported (const char* format) const
{
  // FIXME! ugly kludge to "check" for support
  if (!format)
    {
      return false;
    }
  if (0 == strcmp ("PCX", format))
    {
      return iscan::pcxstream::is_usable ();
    }
  if (0 == strcmp ("PNM", format))
    {
      return iscan::pnmstream::is_usable ();
    }
  if (0 == strcmp ("PNG", format))
    {
      return iscan::pngstream::is_usable ();
    }
  if (0 == strcmp ("JPEG", format))
    {
      return iscan::jpegstream::is_usable ();
    }
  if (0 == strcmp ("PDF", format))
    {
      return iscan::pdfstream::is_usable ();
    }
  if (0 == strcmp ("TIFF", format))
    {
      return iscan::tiffstream::is_usable ();
    }

  return false;
}

#include <sstream>
#include <iostream>
bool
file_selector::possible_overwrite (const std::string& current) const
{
  if (!_using_adf || _multi_page_mode)
    return (0 == access (current.c_str (), F_OK));

  string::size_type dot  = current.rfind (file_opener::ext_sep);
  string::size_type hash = current.find_last_not_of (file_opener::hash_mark,
                                                     dot - 1);
  int digits  = dot - ++hash;

  stringstream ss;
  ss << current.substr (0, hash)
     << "([1-9][0-9]{" << digits << ",}|[0-9]{" << digits << "})\\"
     << current.substr (dot);

  return check_overwrite (ss.str ().c_str ());
}

int
file_selector::check_overwrite (const char *regexp_in) const
{
  bool found = false;

  if (!regexp_in)
    return false;

  char* regexp = strdup (regexp_in);
  if (!regexp)
    return true;

  char *slash = strrchr( regexp, '/' );

  if (!slash)
    {
      free (regexp);
      return true;
    }

  *slash = '\0';                // regexp now holds the directory name
  char dirname[ strlen( regexp )];
  strcpy( dirname, regexp );

  *slash = '^';                 // re-anchor the regexp

  regex_t *comp_regex = new regex_t;
  int comp = regcomp( comp_regex, slash, REG_EXTENDED );

  if (0 == comp)
    {
      size_t     nsub = comp_regex->re_nsub + 1;
      regmatch_t match[nsub];

      DIR *dir = opendir( dirname );
      if (!dir)
        {
          regfree( comp_regex );
          delete comp_regex;
          free (regexp);
          return 0;               // file creation failure handles this
        }

      struct dirent *file = 0;
      while (!found && (file = readdir( dir )))
        {
          int result = regexec( comp_regex, file->d_name, nsub, match, 0 );
          if (0 == result)
            {
              size_t digits = match[1].rm_eo - match[1].rm_so;
              char num[digits + 1];
              char *c = num;
              {
                char *p = file->d_name + match[1].rm_so;
                while (0 < digits--)
                  *c++ = *p++;
              }
              int seq_num = atoi( num );

              found = (seq_num >= get_sequence_number());
            }
          else
            if (REG_NOMATCH != result)
              regerror( comp, comp_regex );
        }
      closedir( dir );
    }
  else
    regerror( comp, comp_regex );

  regfree( comp_regex );
  delete comp_regex;
  free (regexp);

  return found;
}
