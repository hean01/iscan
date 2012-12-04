/* pisa_progress_window.cc
   Copyright (C) 2001, 2004  SEIKO EPSON CORPORATION

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

#include "pisa_progress_window.h"
#include "gettext.h"
#define  _(msg_id)	gettext (msg_id)

#include <cmath>
#include <iostream>

#include "pisa_main_window.h"
#include "pisa_view_manager.h"


// callbacks for use by the GTK+ toolkit
static gint _delete (GtkWidget *, progress_window *);
static void _cancel (GtkWidget *, progress_window *);


/*! Creates an instance as the child of a \a parent widget.  The \a
  parent widget, by default, is the root window.

  Initially, the message will be set to progress_window::WARMING_UP
  with the control button disabled.  The instance will \e not be made
  visible.  Use the show() and hide() members to control visibility.
*/
progress_window:: progress_window (GtkWidget *parent)
  : _msg (-1), _can (false), _last_step (0)
{
  // first, let's gtk_*_new all the main players
  _box = (GtkDialog   *) gtk_dialog_new ();
  _txt = (GtkLabel    *) gtk_label_new ("");
  _bar = (GtkProgressBar *) gtk_progress_bar_new ();
  _btn = (GtkButton   *) gtk_button_new ();

  _btn_cancel = gtk_label_new (_("  Cancel  "));
  _btn_finish = gtk_label_new (_("  Finish  "));

  if (!_box || !_txt || !_bar || !_btn
      || !_btn_cancel || !_btn_finish) {
    if (_btn_finish) gtk_widget_destroy (_btn_finish);
    if (_btn_cancel) gtk_widget_destroy (_btn_cancel);

    if (_btn) gtk_widget_destroy (GTK_WIDGET (_btn));
    if (_bar) gtk_widget_destroy (GTK_WIDGET (_bar));
    if (_txt) gtk_widget_destroy (GTK_WIDGET (_txt));
    if (_box) gtk_widget_destroy (GTK_WIDGET (_box));
    throw std::bad_alloc ();
  }

  // container
  gtk_window_set_policy (GTK_WINDOW (_box), false, false, true);
  gtk_window_set_title  (GTK_WINDOW (_box), PACKAGE);
  gtk_container_border_width (GTK_CONTAINER (_box->vbox), 5);
  gtk_widget_realize (GTK_WIDGET (_box));
  gtk_signal_connect (GTK_OBJECT (_box), "delete_event",
		      GTK_SIGNAL_FUNC (_delete), this);

  // message
  gtk_box_pack_start (GTK_BOX (_box->vbox), GTK_WIDGET(_txt),
		      false, false, 5);

  // progress
  gtk_widget_set_usize (GTK_WIDGET (_bar), 350, 20);
  gtk_box_pack_start (GTK_BOX (_box->vbox), GTK_WIDGET(_bar),
		      false, false, 5);

  // actions
  GTK_WIDGET_SET_FLAGS (GTK_WIDGET (_btn), GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX (_box->action_area), GTK_WIDGET (_btn),
		      false, false, 0);
  gtk_signal_connect (GTK_OBJECT (_btn), "clicked",
		      GTK_SIGNAL_FUNC (_cancel), this);
  gtk_widget_grab_default (GTK_WIDGET (_btn));

  set_text (WARMING_UP);

  gtk_window_set_modal (GTK_WINDOW (_box), true);
  gtk_window_set_transient_for (GTK_WINDOW (_box), GTK_WINDOW (parent));
}

progress_window::~progress_window (void)
{
  gtk_widget_destroy (GTK_WIDGET (_btn_cancel));
  gtk_widget_destroy (GTK_WIDGET (_btn_finish));

  gtk_widget_destroy (GTK_WIDGET (_box));
}

void
progress_window::show (void)
{
  gtk_widget_show_all (GTK_WIDGET (_box));
}

void
progress_window::hide (void)
{
  gtk_widget_hide (GTK_WIDGET (_box));
}

/*! This member function sets the progress indicator to show the
  percentage of \a progress that has been made in terms of an \a
  estimated_total.

  In many situations the estimated total can be known beforehand and
  will not vary, but in some circumstances this estimate may change,
  or become more precisely known, so that you may want to update the
  progress indicator even if there was \e no \a progress.
*/
void
progress_window::set_progress (double progress, double estimated_total)
{
  if (0 == estimated_total) return;

  int step = _step_count * (progress / estimated_total);

  if (_last_step != step)
    {
      gtk_progress_bar_set_fraction (_bar, progress / estimated_total);
      _last_step = step;
    }
}

/*! Changes the text in the message area to an arbitrary \a message.
 */
void
progress_window::set_text (const char *message)
{
  gtk_label_set_text (_txt, message);
}

/*! Changes the text in the message area to one of the "canned"
  messages and adjusts the control button's look and behaviour as
  specified in the progress_window::message documentation.
 */
void
progress_window::set_text (message id)
{
  _last_step = 0;
  if (id == _msg)		// nothing to do
    return;

  switch (id) {
  case WARMING_UP:
    set_text (_("Scanner is warming up. Please wait..."));
    break;
  case PREVIEWING:
    set_text (_("Pre-scanning in Progress"));
    break;
  case SCANNING:
    set_text (_("Scanning in Progress"));
    break;
  case WAITING:
    set_text (_("Starting a sequence of scans.\n"
		"Press to scanner's Start button to start each scan."));
    break;
  default:			// should have been caught by the compiler!
    throw;
  }

  if (-1 == _msg) {		// we were called by the constructor
    GtkWidget *lbl = (WAITING == id) ? _btn_finish : _btn_cancel;
    gtk_container_add (GTK_CONTAINER (_btn), lbl);
  }

  if (WAITING == id && WAITING != _msg) {
    flip_label (_btn_cancel, _btn_finish);
  }
  if (WAITING != id && WAITING == _msg) {
    flip_label (_btn_finish, _btn_cancel);
  }

  gtk_widget_set_sensitive (GTK_WIDGET (_btn), (WARMING_UP != id));

  _msg = id;
}

/*! Returns true when the user has requested cancellation of the
  action(s) the object is reporting progress on, false otherwise.
 */
bool
progress_window::is_cancelled (void) const
{
  return _can;
}

/*! Mainly intended for internal use (by the GTK+ callbacks, to be
  precise), this member function raises a flag indicating that the
  user requested cancellation of the action(s) that this object is
  reporting on.

  Cancellation requests can not be revoked.

  \note It is up to the application to check for and honour such
  requests.
 */
void
progress_window::cancel (void)
{
  if (WARMING_UP != _msg)
    _can = true;
}

/*! Changes the control button's label \a from one \a to another.

  \internal Just removing the label from its container would result in
  its destruction unless someone holds onto a reference for the label
  object.  Since we may want to reuse the label, we add a reference
  before its removal.
 */
void
progress_window::flip_label (GtkWidget *from, GtkWidget *to)
{
  gtk_object_ref (GTK_OBJECT (from));
  gtk_container_remove (GTK_CONTAINER (_btn), from);
  gtk_container_add (GTK_CONTAINER (_btn), to);
  gtk_widget_show (to);
}


static
gint
_delete (GtkWidget *, progress_window *p)
{
  if (p)
    p->cancel ();

  return true;
}

static
void
_cancel (GtkWidget *, progress_window *p)
{
  if (p)
    p->cancel ();
}
