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

#ifndef ___PISA_VIEW_MANAGER_H
#define ___PISA_VIEW_MANAGER_H

#include <string>

#include "pisa_enums.h"

#include "pisa_settings.h"

#include "pisa_scan_manager.h"

#include "pisa_main_window.h"
#include "pisa_preview_window.h"
#include "pisa_progress_window.h"
#include "pisa_image_controls.h"
#include "pisa_gamma_correction.h"
#include "pisa_configuration.h"
#include "pisa_error.h"
#include "pisa_scan_selector.h"

#include <cstring>

#include "file-selector.h"
#include "imgstream.hh"

class view_manager
{
 public:
  
  // operation
  static int	create_view_manager ( int argc, char * argv [ ] );
  
  void	release_view_manager ( void );
  
  int	main ( void );

  void	init ( void );
  void	destroy ( void );

  GtkWidget *	create_window ( pisa_window_id id );
  int	close_window ( pisa_window_id id, int destroy_flag );

  image_controls *get_image_controls ();
  gamma_correction *get_gamma_correction ();
  void *	get_window_cls ( pisa_window_id id );

  scan_manager *	get_scan_manager ( void ) { return m_scanmanager_cls; }

  void	sensitive ( void );

  int	is_gimp ( void );

  void	start_scan ( void );

  int	update_lut (long index = -1);
  int	update_lut (marquee& m);

  bool	change_document_source (const imagetype *);
  void	set_device ( char * name );
  char * get_device_name() const;

  const settings& get_settings (void) const;
  const marquee&  get_marquee (long index = -1) const;
	marquee&  get_marquee (long index = -1);

  void add_marquee (marquee **m);
  void del_marquee (long index = -1);

  long     get_marquee_size () const;

  void set_destination (long destination);
  void set_resolution (long resolution);
  void set_image_type (const imagetype *type);
  void set_unit (long unit);
  void enable_start_button (bool value);
  void enable_draft_mode (bool value);
  void enable_unsharp_mask (bool value);

  int microsleep (size_t usec);

 private:

  // operation
  void	open_device ( void );
  void	close_device ( void );

  void	load_preference ( void );
  void	save_preference ( void );

  int init_img_info (void);

  int	init_scan_param (void);

  bool  do_scan (iscan::imgstream& is, const iscan::file_opener& fo,
                 bool first_time_around = true);
  int	do_scan_gimp (bool first_time_around = true);

  void	scan_file (iscan::imgstream& is, int *status,
		   bool first_time_around = true);
  bool  scan_gimp (int *cancel,
		   bool first_time_around = true);
  int	dialog_reply( const pisa_error& err ) const;

  void print (const std::string& filename) const;
  bool is_multi_image (void) const;
  bool wait_for_button (void) const;
  bool needs_duplex_rotation (void) const;

  // attribute
  settings	m_set;

  scan_manager		* m_scanmanager_cls;

  main_window		* m_main_cls;
  preview_window	* m_prev_cls;
  image_controls	* m_imgctrl_cls;
  gamma_correction	* m_gamma_cls;
  config_window		* m_config_cls;
  file_selector		* m_filsel_cls;
  scan_selector		* m_scansel_cls;

  progress_window *_feedback;

  std::string _image_format;
};

extern view_manager	* g_view_manager;

inline const settings&
view_manager::get_settings (void) const
{
  return m_set;
}

inline const marquee&
view_manager::get_marquee (long index) const
{
  return m_set.get_marquee (index);
}

inline marquee&
view_manager::get_marquee (long index)
{
  return m_set.get_marquee (index);
}

inline long
view_manager::get_marquee_size (void) const
{
  return m_set.get_marquee_size ();
}

inline void
view_manager::add_marquee (marquee **m)
{
  m_set.add_marquee (m);
}

inline void
view_manager::del_marquee (long index)
{
  m_set.del_marquee (index);
}

inline void
view_manager::set_destination (long destination)
{
  m_set.destination = destination;
}

inline void
view_manager::set_unit (long unit)
{
  m_set.unit = unit;
}

inline void
view_manager::enable_start_button (bool value)
{
  m_set.enable_start_button = value;
}

inline void
view_manager::enable_draft_mode (bool value)
{
  m_set.enable_draft_mode = value;
}

inline void
view_manager::enable_unsharp_mask (bool value)
{
  m_set.usm = (value ? 1 : 0);
}


#endif // ___PISA_VIEW_MANAGER_H

