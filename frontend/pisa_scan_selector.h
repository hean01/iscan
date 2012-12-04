/* 						-*- C++ -*-
   SANE EPSON backend
   Copyright (C) 2003 SEIKO EPSON CORPORATION

   Date         Author      Reason
   2003-03-13   S.Wong      New

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

#ifndef ___PISA_SCAN_SELECTOR_H
#define ___PISA_SCAN_SELECTOR_H

#include <gtk/gtk.h>
#include <regex.h>

class scan_selector
{
public:
  ~scan_selector();
   scan_selector( bool is_dialog = false );

  void show() const;

  GtkWidget * widget() const;

  void update();
  void cancel();
  void select();
  char * get_device( bool rewrite = false );

private:

  char * rewrite_name( const SANE_Device *device );
  void regerror( int code, regex_t *regex );

  bool m_local_only;

  GtkWidget *m_dbox;
  GtkWidget *m_hbox;
  GtkWidget *m_opts;
  GtkWidget *m_menu;

  GSList *_items;
  GSList *_dev_names;
  GSList *_gui_names;

  char *_current_dev;
  char *_current_gui;
};

#endif // ___PISA_SCAN_SELECTOR_H
