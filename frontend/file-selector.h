/* file-selector.h -- customized file selection dialog	-*- C++ -*-
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

#ifndef ISCAN_FILE_SELECTOR_H
#define ISCAN_FILE_SELECTOR_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <regex.h>
#include <gtk/gtk.h>

#include "pisa_error.h"

#include "imgstream.hh"         // for iscan::file_format

#define G_CALLBACK_API	public		/* EEK!  */

class file_selector
{
public:
  void init ();
  void create_window (GtkWindow *window, GtkWidget *parent,
		      bool do_consecutive_scanning = false,
		      bool show_match_direction = false,
		      const char* default_format = NULL);

  void destroy ();

  char *get_pathname () const;

  int  get_sequence_number () const;

  bool valid_pathname (const std::string& path) const;
  bool save_pathname ();	// really G_CALLBACK_API only

  void hide () const;

  std::string get_format_name () const;
  iscan::file_format get_type () const;

  bool multi_page_mode ();
  bool has_left_edge_binding ();

G_CALLBACK_API:

  void destroy (bool really);
  void delete_text (gint start, gint end);
  void insert_text (gchar *text, gint len, gint *pos);
  void change_format (int index);
  void change_seqnum ();
  void change_digits ();
  void change_page_mode (bool mode);
  void change_binding_edge (bool match);

private:
  char *canonize (const char *text) const;

  void add_dialog_extensions (gint index = 0);

  bool permission (const char *file) const;
  bool show_message (const pisa_error& oops,
                     const char *yes = NULL,
                     const char *no = NULL) const;

  void set_entry (const char *text);

  void regerror (int code, regex_t *regex) const;
  
  gint get_index (const char* name) const;
  bool is_supported (const char* format) const;
  bool possible_overwrite (const std::string& current) const;
  int check_overwrite (const char *regexp) const;

  GtkWidget        *_parent;
  GtkFileSelection *_widget;
  char *_filename;		// relative filename
  char *_pathname;		// absolute filename

  bool _using_adf;
  bool _do_duplex;		// indicates whether to display a binding
				// location selector (default: left edge)

  int  _format_index;		// index into file format menu indicating
				// the currently selected item
  bool _deleted_all;
  bool _multi_page_mode;
  bool _has_left_edge_binding;	// indicates whether the document is bound
				// on the left hand side

  int _number;			// for consecutive scans
  GtkAdjustment *_seqnum;
  GtkAdjustment *_digits;

  typedef struct		// graphic format info
  {
    const char *name;		// used in option menu
    const char *lib;		// library providing support
    const char *ext;		// default filename extension
    const char *rgx;		// filename extension regex
    const iscan::file_format type;
  } format;

  static const format   _format[];
         const format **_fmt;

  char *_ext_regex;
};

#endif /* ISCAN_FILE_SELECTOR_H */
