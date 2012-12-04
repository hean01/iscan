/* pisa_sane_scan.h					-*- C++ -*-
   Copyright (C) 2001, 2004, 2008, 2009  SEIKO EPSON CORPORATION

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

#ifndef ___PISA_SANE_SCAN_H
#define ___PISA_SANE_SCAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sane/sane.h>
#include <sane/saneopts.h>

#undef  SANE_NAME_SCAN_X_RESOLUTION
#define SANE_NAME_SCAN_X_RESOLUTION "x-resolution"

#ifdef __cplusplus
}
#endif

enum br_method_val
{
  br_iscan,
  br_gimp
};

class sane_scan
{
public:
  bool has_flatbed (void) const;
  void select_flatbed (void);
  bool using_flatbed (void) const;

  bool has_adf (void) const;
  void select_adf (bool duplex);
  bool using_adf (void) const;

  bool has_duplex (void) const;
  bool using_duplex (void) const;

  bool has_tpu (void) const;
  void select_tpu (bool positive);
  bool using_tpu (void) const;

  bool has_dfd (void) const;
  void set_dfd (long idx);

  char get_scan_source (void) const;
  char get_film_type (void) const;

  void clear_button_status (void);


  void	close_device (void);

  // queries
  void	get_current_max_size (double *width, double *height);
  long	get_max_resolution (long cutoff = -1) const;
  void	set_scan_resolution (int resolution_x, int resolution_y);
  void	get_color_profile (double *coef) const;
  bool	has_zoom (void) const;
  bool	has_focus (void) const;
  bool  has_draft_mode (void) const;
  bool  get_size_check (void) const;
  void  set_size_check (bool value);
  bool  has_size_check (void) const;
  bool	get_autocrop (void) const;
  void	set_autocrop (bool value);
  bool	has_autocrop (void) const;
  bool	has_start_button (void) const;
  bool	get_deskew (void) const;
  void	set_deskew (bool value);
  bool	has_deskew (void) const;
  void	set_brightness (long brightness);
  bool	has_brightness (void) const;
  void	set_contrast (long contrast);
  bool	has_contrast (void) const;

  void	set_brightness_method (br_method_val val);
  void	set_color_mode (char pixeltype, char bitdepth);

  bool	is_button_pressed (void) const;
  long	get_polling_time (void) const;

  void disable_wait_for_button (void);

  void	finish_acquire (bool eject = false);

  void	get_value (const char *option_name, void *value,
                   bool nothrow = false) const;

  void	get_range (const char *option_name,
                   float *max, float *min, float *step);

  void	set_preview (bool value);
  bool	has_preview (void) const;

  bool	has_clean (void) const;
  bool	has_calibrate (void) const;
  void	clean (void);
  void	calibrate (void);

protected:

  // mutators
  void	set_scan_source (void);
  void	set_film_type (void);

  void	init (void);
  void	open_device (char *name = 0);

  void	start_scan (int *width, int *height);
  SANE_Status	acquire_image (unsigned char *img, int row_bytes, int height,
		       int cancel);

  int	is_activate (const char *option_name) const;
  long	get_resolution (SANE_String_Const direction, int min_res) const;
  long	get_max_resolution (const char *option_name, long cutoff = -1) const;
  int	get_option_id (const char *option_name) const;

  void	set_value (const char *option_name, const void *value);

  void	query_device (char *device_name) const;
  void	get_scanner_info (const char *name = 0);

  void	set_color_profile (const double *coef);
  void	set_focus (long position);
  void	set_gamma_table (const unsigned char *gamma_table);
  void	set_scan_area (double offset_x, double offset_y,
		       double width, double height);
  void	set_scan_zoom (int zoom_x, int zoom_y);
  void	set_speed (long speed);
  void	set_threshold (long threshold);

private:
  SANE_Handle		m_hdevice;
  SANE_Parameters	m_sane_para;
  long			m_rows;

  char *		name;
  char			support_option;
  long			max_resolution;

  char _source;
  char _film;
};


#endif // ___PISA_SANE_SCAN_H
