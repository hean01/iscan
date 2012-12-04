/* 
   SANE EPSON backend
   Copyright (C) 2001, 2005, 2008, 2009  SEIKO EPSON CORPORATION

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

#ifndef ___PISA_SCAN_MANAGER_H
#define ___PISA_SCAN_MANAGER_H

#include "esmod-wrapper.hh"

#include "pisa_sane_scan.h"
#include "pisa_error.h"

class scan_manager : public sane_scan
{
 public:

  // operator
  void	open_device ( char * name = 0 );

  void	init_preview (int *width, int *height);
  void	init_scan (int *width, int *height, bool reset_params = true);
  void	adjust_scan_param (long *resolution, long *scale) const;
  void	get_valid_resolution (long int *x_res, long int *y_res,
			      bool use_max = false) const;

  void	acquire_image ( unsigned char * img,
			int row_bytes,
			int height,
			int cancel );
  int	release_memory ( void );

  pisa_error_id set_scan_parameters (const settings& set,
				     const marquee& marq);
  pisa_error_id set_scan_parameters (const settings& set,
				     const marquee& marq,
				     int resolution);

  bool adf_duplex_direction_matches (void) const;

  void has_prev_img ( int has_prev );

  bool push_button_needs_polling (void) const;

 protected:
  void set_scan_parameters (bool is_preview = false);
  void start_scan (int *width, int *height);

 private:

  typedef struct _IMAGE_INFO {
    unsigned char* pImg_Buf;
    long	   Img_Width;
    long	   Img_Height;
    unsigned long  Img_RowBytes;
    short	   BitsPerPixel;
  } IMAGE_INFO, *LPIMAGE_INFO;

  // operation
  int	init_img_process_info (void);
  int	init_zoom (resize_img_info *info);
  int	init_moire (moire_img_info *info);
  int	init_sharp (sharp_img_info *info);

  int	modify_img_info ( int * width, int * height );

  int	create_img_cls ( void );

  void	set_img_info ( LPIMAGE_INFO in_img_info,
		       LPIMAGE_INFO out_img_info,
		       const img_size & size );

  void	update_settings (bool is_preview);
  bool	area_is_too_large (void) const;

  // FIXME: the following block of member functions should not be the
  //        responsibility of the scan_manager.  They can be removed
  //        once the filter class hierarchy has support for chaining
  //        of filters and this class no longer needs to baby sit the
  //        data transfers.
  int	get_send_in_line ( int out_line );
  int	image_process ( unsigned char * in_img, unsigned char * out_img );

  // for image module
  long			m_resize;
  long			m_moire;
  long			m_sharp;

  resize_img_info	m_resize_info;
  moire_img_info	m_moire_info;
  sharp_img_info	m_sharp_info;

  iscan::scale		* m_resize_cls;
  iscan::moire		* m_moire_cls;
  iscan::focus		* m_sharp_cls;

  enum { _IN, _OUT };
  IMAGE_INFO            * m_resize_img_info;
  IMAGE_INFO            * m_moire_img_info;
  IMAGE_INFO            * m_sharp_img_info;

  char          _pixeltype;
  char          _bitdepth;
  char          _dropout;
  char          _monoopt;
  char          _halftone;
  double        _offset[2];
  double        _area[2];
  long          _resolution[2];
  long          _zoom[2];
  gamma_struct  _gamma;
  double        _coef[9];
  long          _threshold;
  long          _speed;
  long          _focus;
  long          _usm;
  long          _brightness;
  long          _contrast;
  long          _deskew;
  long          _de_screening;
  long          _max_descreen_resolution;
  int           _has_prev_img;
};

#endif // ___PISA_SCAN_MANAGER_H

