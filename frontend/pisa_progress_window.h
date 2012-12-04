/* pisa_progress_window.h				-*- C++ -*-
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

#ifndef ___PISA_PROGRESS_WINDOW_H
#define ___PISA_PROGRESS_WINDOW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

//! A progress feedback dialog
/*! A progress_window provides the user visual feedback about what is
  going on.  It has space for a short to medium length message at the
  top, a progress indicator in the middle and a control button at the
  the bottom of the window.

  The message may be changed to anything at any convenient time, but
  there are also several "canned" messages available.  These messages
  carry some additional logic with them as well.

  The control button can be used to flag cancellation of the action(s)
  that the dialog is reporting on.  However, it is the responsibility
  of the application to check for and honour such requests.
 */
class progress_window
{
public:
   progress_window (GtkWidget *parent = 0);
  ~progress_window (void);

  //! Makes the progress window visible.
  void show (void);
  //! Makes the progress window invisible.
  void hide (void);

  //! Updates the progress indicator.
  void set_progress (double progress, double estimated_total);

  //! IDs for "canned" message texts.
  enum message {
    /*! This message can be shown whenever it takes a while before the
      device is ready to carry out an action.  Setting this message
      will disable the control button because things like warming up
      typically should not be interrupted.
     */
    WARMING_UP,
    /*! This message can be shown during a preview scan.  The control
      button is enabled during a preview.
     */
    PREVIEWING,
    /*! This message can be shown during a normal scan.  Just as with
      a preview scan, the control button is enabled.
     */
    SCANNING,
    /*! This message can be shown when the application is waiting for
      the user to press a scanner button.  The control button will be
      enabled.  Note that it may labelled differently from the other
      cases.
     */
    WAITING
  };

  //! Updates the message indicating the stage of progress.
  void set_text (const char *message);
  //! Sets a "canned" message indicating the stage of progress.
  void set_text (message id);

  //! Indicates whether the user requested cancellation.
  bool is_cancelled (void) const;

  //! Flags a cancellation request.
  void cancel (void);

private:
  //! Changes the text on the control button.
  void flip_label (GtkWidget *from, GtkWidget *to);

private:
  GtkDialog   *_box;
  GtkLabel    *_txt;		//!< message area
  GtkProgressBar *_bar;		//!< progress indicator
  GtkButton   *_btn;		//!< control button
  GtkWidget   *_btn_cancel;
  GtkWidget   *_btn_finish;

  int  _msg;
  bool _can;

  static const int _step_count = 50;
  int _last_step;
};

#endif // ___PISA_PROGRESS_WINDOW_H
