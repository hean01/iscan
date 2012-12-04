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

/*------------------------------------------------------------*/
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <new>
using std::bad_alloc;


/*------------------------------------------------------------*/
#include "pisa_scan_manager.h"
#include "pisa_error.h"
#include "pisa_scan_tool.h"
#include "pisa_change_unit.h"

/*------------------------------------------------------------*/
#define SCALING_INPUT_WIDTH_LIMIT 21840

/*------------------------------------------------------------*/
void scan_manager::open_device ( char * name )
{
  sane_scan::init ( );

  m_resize_cls	= 0;
  m_moire_cls	= 0;
  m_sharp_cls	= 0;
  m_resize_img_info = 0;
  m_moire_img_info  = 0;
  m_sharp_img_info  = 0;

  _has_prev_img = false;

  sane_scan::open_device (name);
  _max_descreen_resolution = get_max_resolution (has_zoom () ? 800 : 600);
}

void
scan_manager::set_scan_parameters (bool is_preview)
{
  for (int i = 0; i < 2; ++i)
    {
      adjust_scan_param (&_resolution[i], &_zoom[i]);
    }
  init_img_process_info ();
  update_settings (is_preview);
}

void
scan_manager::start_scan (int *width, int *height)
{
  sane_scan::start_scan (width, height);
  modify_img_info (width, height);
  create_img_cls ();
}

void
scan_manager::init_preview (int *width, int *height)
{
  set_scan_parameters (true);
  start_scan (width, height);
}

void
scan_manager::init_scan (int *width, int *height, bool reset_params)
{
  if (reset_params) set_scan_parameters ();

  if (area_is_too_large ())
    {
      if (getenv ("ISCAN_DEBUG"))
	{
	  fprintf (stderr, "Backend will try to scan with these settings\n");
	  fprintf (stderr, "Resolution (main,sub): %ld,%ld\n",
		   _resolution[0], _resolution[1]);
	  fprintf (stderr, "Zoom       (main,sub): %ld,%ld\n",
		   _zoom[0], _zoom[1]);
	  fprintf (stderr, "Offset     (main,sub): %ld,%ld\n",
		   inch2pixel (_offset[0], _resolution[0], _zoom[0]),
		   inch2pixel (_offset[1], _resolution[1], _zoom[1]));
	  fprintf (stderr, "Area       (main,sub): %ld,%ld\n",
		   inch2pixel (true , _area[0], _resolution[0], _zoom[0]),
		   inch2pixel (false, _area[1], _resolution[1], _zoom[1]));
	}

      throw pisa_error (PISA_ERR_AREALARGE);
    }

  start_scan (width, height);
}

/*------------------------------------------------------------*/
void scan_manager::acquire_image ( unsigned char * img,
				   int row_bytes,
				   int height,
				   int cancel )
{
  if ( m_resize || m_moire || m_sharp )
    {
      int in_rowbytes = 0, in_line;
      unsigned char * in_img = 0;

      if ( m_sharp )
	in_rowbytes = m_sharp_info.in_rowbytes;
      if ( m_moire )
	in_rowbytes = m_moire_info.in_rowbytes;
      if ( m_resize )
	in_rowbytes = m_resize_info.in_rowbytes;

      if ( cancel || img == 0 )
        {
          if (0 < row_bytes)
            {
              // Run kludge to trigger a SANE_STATUS_EOF return value
              // on sane_read() in sane_scan::acquire_image(), as per
              // SANE specification requirements.
              // Fixes problems when used with `net` backend.

              unsigned char b[512];
              SANE_Status s = SANE_STATUS_GOOD;
              while (SANE_STATUS_GOOD == s)
                {
                  s = sane_scan::acquire_image (b, sizeof (b), 1, cancel);
                }
              return;
            }
          else
            {
              sane_scan::acquire_image (0, 0, height, cancel);
            }
        }
      else
	{
	  in_line = get_send_in_line ( 1 );

	  if ( 0 < in_line )
	    {
	      int buf_size = in_line * in_rowbytes;
	      
	      in_img = new unsigned char [ buf_size + in_rowbytes ];
	      
	      if ( in_img == 0 )
		{
		  sane_scan::acquire_image ( 0, 0, 1, 1 );
		  
		  throw pisa_error ( PISA_ERR_OUTOFMEMORY );
		}
	      
	      sane_scan::acquire_image ( in_img,
					 in_rowbytes,
					 in_line,
					 cancel );
	    }
	}

      ::memset ( img, 128, row_bytes );
      if ( cancel == 0 && PISA_ERR_SUCCESS != image_process ( in_img, img ) )
	{
	  delete [ ] in_img;
	  sane_scan::acquire_image ( 0, 0, 1, 1 );
	  throw pisa_error ( PISA_ERR_OUTOFMEMORY );
	}

      if ( in_img )
	delete [ ] in_img;
    }
  else
    sane_scan::acquire_image ( img, row_bytes, height, cancel );
}

int
scan_manager::init_img_process_info ()
{
  // set flag
  if (!has_zoom ())
    m_resize = 1;
  else
    m_resize = 0;

  if (_de_screening && !get_size_check()) // color or B/W document
    {
      m_moire = 1;
      m_resize = 0;
    }
  else
    m_moire = 0;

  m_sharp = _usm;

  if ( m_resize )
    init_zoom (&m_resize_info);

  if ( m_moire )
    init_moire (&m_moire_info);

  if ( m_sharp )
    init_sharp (&m_sharp_info);

  return PISA_ERR_PARAMETER;
}

int
scan_manager::init_zoom (resize_img_info *info)
{
  long act_res[2];

  info->resolution = _resolution[0];

  for (int i = 0; i < 2; ++i)
    {
      act_res[i] = _resolution[i] * _zoom[i] / 100;
    }

  info->out_width	= ::inch2width  (_area[0], act_res[0]);
  info->out_height	= ::inch2height (_area[1], act_res[1]);
  info->out_rowbytes	= calc_rowbytes (info->out_width,
					 static_cast <pisa_pixel_type>
					 (_pixeltype));

  for (int i = 0; i < 2; ++i)
    {
      _resolution[i] = act_res[i];
      _zoom[i] = 100;
    }

  try
    {
      get_valid_resolution (&_resolution[0], &_resolution[1], true);
    }
  catch (const pisa_error& e)
    {
      if (pisa_error (PISA_STATUS_UNSUPPORTED) != e)
	throw (e);
    }

  if (   act_res[0] == _resolution[0]
      && act_res[1] == _resolution[1])
    {
      m_resize = 0;
      return PISA_ERR_SUCCESS;
    }

  info->in_width	= ::inch2width  (_area[0], _resolution[0]);
  info->in_height	= ::inch2height (_area[1], _resolution[1]);
  info->in_rowbytes	= calc_rowbytes (info->in_width,
					 static_cast <pisa_pixel_type>
					 (_pixeltype));
  
  info->bits_per_pixel	= calc_bitperpix (static_cast <pisa_pixel_type>
					  (_pixeltype),
					  static_cast <pisa_bitdepth_type>
					  (_bitdepth));

  if (_pixeltype == PISA_PT_BW)
    info->resize_flag	= PISA_RS_NN;
  else
    info->resize_flag	= PISA_RS_BC;
  
  return PISA_ERR_SUCCESS;
}

int
scan_manager::init_moire (moire_img_info *info)
{
  info->resolution = _resolution[0] * _zoom[0] / 100;

  _resolution[0] =
  _resolution[1] =
    iscan::moire::get_res_quote (info->resolution, !has_zoom ());

  if (!has_zoom ())
    {
      try
	{
	  get_valid_resolution (&_resolution[0], &_resolution[1]);
	}
      catch (const pisa_error& e)
	{
	  if (pisa_error (PISA_STATUS_UNSUPPORTED) != e)
	    throw (e);
	  // else do nothing
	}
    }

  info->in_resolution = _resolution[0];

  _zoom [ 0 ] = 100;
  _zoom [ 1 ] = 100;

  info->in_width	= ::inch2width  (_area[0], _resolution[0]);
  info->in_height	= ::inch2height (_area[1], _resolution[1]);
  info->in_rowbytes	= calc_rowbytes (info->in_width,
					 static_cast <pisa_pixel_type>
					 (_pixeltype));

  info->out_width	= ::inch2width  (_area[0], info->resolution);
  info->out_height	= ::inch2height (_area[1], info->resolution);
  info->out_rowbytes	= calc_rowbytes (info->out_width,
					 static_cast <pisa_pixel_type>
					 (_pixeltype));

  info->bits_per_pixel	= calc_bitperpix (static_cast <pisa_pixel_type>
					  (_pixeltype),
					  static_cast <pisa_bitdepth_type>
					  (_bitdepth));

  return PISA_ERR_SUCCESS;
}

/*------------------------------------------------------------*/
int
scan_manager::init_sharp (sharp_img_info *info)
{
  long resolution;

  if ( m_resize )
    {
      info->in_width	= m_resize_info.out_width;
      info->in_height	= m_resize_info.out_height;

      resolution	= m_resize_info.resolution;
    }
  else if ( m_moire )
    {
      info->in_width	= m_moire_info.out_width;
      info->in_height	= m_moire_info.out_height;

      resolution	= m_moire_info.resolution;
    }
  else
    {
      info->in_width	= ::inch2width  (_area[0], _resolution[0], _zoom[0]);
      info->in_height	= ::inch2height (_area[1], _resolution[1], _zoom[1]);

      resolution	= _resolution[0];
    }

  info->in_rowbytes	= calc_rowbytes (info->in_width,
					 static_cast <pisa_pixel_type>
					 (_pixeltype));
  
  info->bits_per_pixel	= calc_bitperpix (static_cast <pisa_pixel_type>
					  (_pixeltype),
					  static_cast <pisa_bitdepth_type>
					  (_bitdepth));

  info->out_width	= info->in_width;
  info->out_height	= info->in_height;
  info->out_rowbytes	= info->in_rowbytes;

  m_sharp_cls->set_parms (resolution,
                          using_tpu (),
			  !has_zoom (),
			  & info->strength,
			  & info->radius,
			  & info->clipping);

  info->sharp_flag	= PISA_SH_UMASK;
  
  return PISA_ERR_SUCCESS;
}

/*------------------------------------------------------------*/
int scan_manager::modify_img_info ( int * width, int * height )
{
  // resize
  if ( m_resize )
    {
      if ( m_resize_info.in_width == m_resize_info.out_width )
	{
	  m_resize_info.out_width = * width;
	}
      else if ( ( has_autocrop () && get_autocrop () )
                || ( has_size_check () && get_size_check () ) )
        {
          m_resize_info.out_width =
            (*width * m_resize_info.out_width + 0.5) / m_resize_info.in_width;
        }
	  m_resize_info.out_rowbytes = ( m_resize_info.out_width *
					 m_resize_info.bits_per_pixel +
					 7 ) / 8;

      m_resize_info.in_width	= * width;
      m_resize_info.in_rowbytes	= ( m_resize_info.in_width *
				    m_resize_info.bits_per_pixel +
				    7 ) / 8;

      if ( m_resize_info.in_height == m_resize_info.out_height )
	{
	  m_resize_info.out_height = * height;
	}
      else if ( ( has_autocrop () && get_autocrop () )
                || ( has_size_check () && get_size_check () ) )
        {
          m_resize_info.out_height =
            (*height * m_resize_info.out_height + 0.5) / m_resize_info.in_height;
        }

      m_resize_info.in_height	= * height;
    }
 
  if ( m_moire )
    {
      if ( ( has_autocrop () && get_autocrop () )
           || ( has_size_check () && get_size_check () ) )
        {
          if ( m_moire_info.in_width == m_moire_info.out_width )
            {
              m_moire_info.out_width = * width;
            }
          else
            {
              m_moire_info.out_width =
                (*width * m_moire_info.resolution + 0.5) / _resolution[0];
            }
          if ( m_moire_info.in_height == m_moire_info.out_height )
            {
              m_moire_info.out_height = * height;
            }
          else
            {
              m_moire_info.out_height =
                (*height * m_moire_info.resolution + 0.5) / _resolution[1];
            }
          m_moire_info.in_width  = * width;
          m_moire_info.in_height = * height;
          m_moire_info.in_rowbytes =
            calc_rowbytes (m_moire_info.in_width,
                           static_cast <pisa_pixel_type> (_pixeltype));
          m_moire_info.out_rowbytes =
            calc_rowbytes (m_moire_info.out_width,
                           static_cast <pisa_pixel_type> (_pixeltype));
        }
    }

  if ( m_sharp )
    {
      m_sharp_info.in_width	= * width;
      m_sharp_info.in_height	= * height;
      m_sharp_info.in_rowbytes	= ( m_sharp_info.in_width *
				    m_sharp_info.bits_per_pixel +
				    7 ) / 8;

      if ( m_resize )
	{
	  m_sharp_info.in_width		= m_resize_info.out_width;
	  m_sharp_info.in_height	= m_resize_info.out_height;
	  m_sharp_info.in_rowbytes	= m_resize_info.out_rowbytes;
	}

      if ( m_moire )
	{
	  m_sharp_info.in_width		= m_moire_info.out_width;
	  m_sharp_info.in_height	= m_moire_info.out_height;
	  m_sharp_info.in_rowbytes	= m_moire_info.out_rowbytes;
	}

      m_sharp_info.out_width	= m_sharp_info.in_width;
      m_sharp_info.out_height	= m_sharp_info.in_height;
      m_sharp_info.out_rowbytes	= m_sharp_info.in_rowbytes;
    }

  // update width and height
  if ( m_resize )
    {
      * width	= m_resize_info.out_width;
      * height	= m_resize_info.out_height;
    }

  if ( m_moire )
    {
      * width	= m_moire_info.out_width;
      * height	= m_moire_info.out_height;
    }

  if ( m_sharp )
    {
      * width	= m_sharp_info.out_width;
      * height	= m_sharp_info.out_height;
    }


  return PISA_ERR_SUCCESS;
}

/*------------------------------------------------------------*/
int scan_manager::create_img_cls ( void )
{
  release_memory ();

  if (m_sharp)
    {
      m_sharp_img_info = new IMAGE_INFO [ 2 ];

      set_img_info ( & m_sharp_img_info [ _IN ],
		     & m_sharp_img_info [ _OUT ],
		     m_sharp_info );

      m_sharp_cls = new iscan::focus (m_sharp_info);
    }

  if ( m_moire )
    {
      m_moire_img_info = new IMAGE_INFO [ 2 ];

      set_img_info ( & m_moire_img_info [ _IN ],
		     & m_moire_img_info [ _OUT ],
		     m_moire_info );

      m_moire_cls = new iscan::moire (m_moire_info, !has_zoom ());
    }
  
  if ( m_resize )
    {
      m_resize_img_info = new IMAGE_INFO [ 2 ];

      set_img_info ( & m_resize_img_info [ _IN ],
		     & m_resize_img_info [ _OUT ],
		     m_resize_info );

      m_resize_cls = new iscan::scale (m_resize_info);
    }

  return PISA_ERR_SUCCESS;
}

/*------------------------------------------------------------*/
int scan_manager::release_memory ( void )
{
  if ( m_resize_cls )
    delete m_resize_cls;
  m_resize_cls = 0;

  if ( m_moire_cls )
    delete m_moire_cls;
  m_moire_cls = 0;

  if ( m_sharp_cls )
    delete m_sharp_cls;
  m_sharp_cls = 0;

  if ( m_resize_img_info )
    delete [] m_resize_img_info;
  m_resize_img_info = 0;

  if ( m_moire_img_info )
    delete [] m_moire_img_info;
  m_moire_img_info = 0;

  if ( m_sharp_img_info )
    delete [] m_sharp_img_info;
  m_sharp_img_info = 0;

  return PISA_ERR_SUCCESS;
}

/*------------------------------------------------------------*/
void scan_manager::set_img_info ( LPIMAGE_INFO in_img_info,
				  LPIMAGE_INFO out_img_info,
				  const img_size & size )
{
  in_img_info->pImg_Buf		= 0;
  in_img_info->Img_Width	= size.in_width;
  in_img_info->Img_Height	= size.in_height;
  in_img_info->Img_RowBytes	= size.in_rowbytes;
  in_img_info->BitsPerPixel	= size.bits_per_pixel;

  out_img_info->pImg_Buf	= 0;
  out_img_info->Img_Width	= size.out_width;
  out_img_info->Img_Height	= size.out_height;
  out_img_info->Img_RowBytes	= size.out_rowbytes;
  out_img_info->BitsPerPixel	= size.bits_per_pixel;
}

/*------------------------------------------------------------*/
int scan_manager::get_send_in_line ( int out_line )
{
  size_t quote = out_line;

  if (m_sharp)
    {
      m_sharp_img_info [ _OUT ].Img_Height = quote;
      quote = m_sharp_cls->get_line_quote (quote);
      m_sharp_img_info [ _IN ].Img_Height  = quote;
    }
  if (m_moire)
    {
      m_moire_img_info [ _OUT ].Img_Height = quote;
      quote = m_moire_cls->get_line_quote (quote);
      m_moire_img_info [ _IN ].Img_Height  = quote;
    }
  if (m_resize)
    {
      m_resize_img_info [ _OUT ].Img_Height = quote;
      quote = m_resize_cls->get_line_quote (quote);
      m_resize_img_info [ _IN ].Img_Height  = quote;
    }

  return quote;
}

/*------------------------------------------------------------*/
int scan_manager::image_process ( unsigned char * in_img,
				  unsigned char * out_img )
{
  unsigned char *inbuf, *outbuf;
  size_t         in_sz,  out_sz;

  unsigned char *tmpbuf = NULL;
  size_t         tmp_sz = 0;
  bool           zapbuf = false;

  try
    {
      if (m_resize)
	{
	  in_sz = m_resize_img_info [ _IN ].Img_Height * m_resize_img_info [ _IN].Img_RowBytes;
	  inbuf = in_img;

	  if (m_sharp)
	    {
	      out_sz = tmp_sz = (m_resize_img_info [ _OUT ].Img_Height
				 * m_resize_img_info [ _OUT ].Img_RowBytes);
	      outbuf = tmpbuf = new unsigned char [out_sz];
	      zapbuf = true;
	    }
	  else
	    {
	      out_sz = m_resize_img_info [ _OUT ].Img_Height * m_resize_img_info [ _OUT ].Img_RowBytes;
	      outbuf = out_img;
	    }
	  m_resize_cls->exec (inbuf, in_sz, outbuf, out_sz);
	}

      if (m_moire)
	{
	  in_sz = m_moire_img_info [ _IN ].Img_Height * m_moire_img_info [ _IN ].Img_RowBytes;
	  inbuf = in_img;

	  if (m_sharp)
	    {
	      out_sz = tmp_sz = (m_moire_img_info [ _OUT ].Img_Height
				 * m_moire_img_info [ _OUT ].Img_RowBytes);
	      outbuf = tmpbuf = new unsigned char [out_sz];
	      zapbuf = true;
	    }
	  else
	    {
	      out_sz = m_moire_img_info [ _OUT ].Img_Height * m_moire_img_info [ _OUT ].Img_RowBytes;
	      outbuf = out_img;
	    }
	  m_moire_cls->exec (inbuf, in_sz, outbuf, out_sz);
	}

      if (m_sharp)
	{
	  out_sz = m_sharp_img_info [ _OUT ].Img_Height * m_sharp_img_info [ _OUT ].Img_RowBytes;
	  outbuf = out_img;

	  if (m_resize || m_moire)
	    {
	      in_sz = tmp_sz;
	      inbuf = tmpbuf;
	    }
	  else
	    {
	      in_sz = m_sharp_img_info [ _IN ].Img_Height * m_sharp_img_info [ _IN ].Img_RowBytes;
	      inbuf = in_img;
	    }
	  m_sharp_cls->exec (inbuf, in_sz, outbuf, out_sz);
	}

      if (zapbuf)
	{
	  delete [] tmpbuf;
	}
    }
  catch (bad_alloc& oops)
    {
      if (zapbuf)
	{
	  delete [] tmpbuf;
	}
      return PISA_ERR_OUTOFMEMORY;
    }

  return PISA_ERR_SUCCESS;
}

void
scan_manager::adjust_scan_param (long *resolution, long *scale) const
{
  int min_res   = 50;
  int max_res   = get_max_resolution ();
  int adj_res   = *resolution;

  int min_scale =  50;
  int max_scale = 200;
  int adj_scale = 100;		// assume no scaling is needed

  if (adj_res < min_res)
    {
      adj_scale = adj_res * 100 / min_res;

      if (adj_scale < min_scale)
	{
	  adj_scale = min_scale;
	}
      adj_res = min_res;
    }

  if (max_res < adj_res)
    {
      adj_scale = adj_res * 100 / max_res;

      if (adj_scale > max_scale)
	{
	  adj_scale = max_scale;
	}
      adj_res = max_res;
    }

  *resolution = adj_res;
  *scale      = adj_scale;
}

/*!  Sets main and sub resolutions to the best available value.

     The best available value is the first resolution not smaller than
     the value passed.  If no such value is available, the largest
     available resolution will be used unless \a use_max is \c false.

     In the latter case an exception will be thrown instead.
 */
void
scan_manager::get_valid_resolution (long int *x_res, long int *y_res,
				    bool use_max) const
{
  if (!x_res || !y_res)
    throw pisa_error (PISA_ERR_PARAMETER);

  SANE_Int res_x = get_resolution (SANE_NAME_SCAN_X_RESOLUTION, *x_res);
  SANE_Int res_y = get_resolution (SANE_NAME_SCAN_Y_RESOLUTION, *y_res);

  if (0 == res_x || 0 == res_y)
    {
      if (!use_max)
	throw pisa_error (PISA_STATUS_UNSUPPORTED);
      if (0 == res_x)
	res_x = get_max_resolution (SANE_NAME_SCAN_X_RESOLUTION);
      if (0 == res_y)
	res_y = get_max_resolution (SANE_NAME_SCAN_Y_RESOLUTION);
    }

  *x_res = res_x;
  *y_res = res_y;
}

static bool
scaling_too_large (const img_size &info)
{
  return    info.in_width > SCALING_INPUT_WIDTH_LIMIT
         && info.in_width != info.out_width;
}

bool
scan_manager::area_is_too_large (void) const
{
  int area_is_valid;

  // due to a bug in esmod, need to limit the input width when scaling
  if (m_resize && scaling_too_large (m_resize_info)) return true;
  if (m_moire  && scaling_too_large (m_moire_info))  return true;

  get_value ("scan-area-is-valid", &area_is_valid);
  return (0 == area_is_valid);
}

bool
scan_manager::adf_duplex_direction_matches (void) const
{
  int rv;

  // if option is inactive, assume direction does not match
  if (!is_activate ("adf-duplex-direction-matches"))
    return false;

  try
    {
      get_value ("adf-duplex-direction-matches", &rv);
    }
  catch (pisa_error& oops)
    { // if no option available, assume direction does not match
      return false;
    }
  return (SANE_TRUE == rv);
}

void
scan_manager::has_prev_img ( int has_prev )
{
  _has_prev_img = has_prev;
}

bool
scan_manager::push_button_needs_polling (void) const
{
  SANE_Bool result = false;

  if (is_activate ("needs-polling"))
    get_value ("needs-polling", &result);

  return result;
}

void
scan_manager::update_settings (bool is_preview)
{
  SANE_Bool auto_scan = !is_preview;

  set_scan_source ();

  if (is_activate ("adf-auto-scan"))
    set_value ("adf-auto-scan", &auto_scan);

  _max_descreen_resolution = get_max_resolution (has_zoom () ? 800 : 600);

  set_film_type ();
  set_focus (_focus);
  set_speed (_speed);
  set_deskew (_deskew);
  set_color_mode (_pixeltype, _bitdepth);

  if (is_preview || _has_prev_img)
    {
      set_gamma_table (_gamma.gamma_r);
      if (_pixeltype == PISA_PT_BW)
        set_threshold (_threshold);
    }
  if (_pixeltype == PISA_PT_RGB)
    set_color_profile (_coef);
  set_brightness_method (br_iscan);
  if (has_brightness())
    {
      set_brightness (_brightness);
    }
  if (has_contrast())
    {
      set_contrast (_contrast);
    }
  set_scan_resolution (_resolution[0], _resolution[1]);
  set_scan_zoom (_zoom[0], _zoom[1]);
  set_scan_area (_offset[0], _offset[1],
                 _area[0], _area[1]);
}

pisa_error_id
scan_manager::set_scan_parameters (const settings& set, const marquee& marq)
{
  _pixeltype	= set.imgtype.pixeltype;
  _bitdepth	= set.imgtype.bitdepth;
  _dropout	= set.imgtype.dropout;
  _monoopt	= set.imgtype.monoopt;
  _halftone	= set.imgtype.halftone;

  _offset[0]	= marq.offset.x;
  _offset[1]	= marq.offset.y;
  _area[0]	= marq.area.x;
  _area[1]	= marq.area.y;
  _resolution[0]= (set.resolution * marq.scale + 50) / 100;
  _resolution[1]= (set.resolution * marq.scale + 50) / 100;
  _zoom[0]	= 100;
  _zoom[1]	= 100;

  _brightness	= marq.brightness;
  _contrast	= marq.contrast;
  _deskew	= get_deskew();

  for (int i = 0; i < 256; ++i)
    {
      _gamma.gamma_r[i] = marq.lut.gamma_r[i];
      _gamma.gamma_g[i] = marq.lut.gamma_g[i];
      _gamma.gamma_b[i] = marq.lut.gamma_b[i];
    }

  if (set.imgtype.pixeltype == PISA_PT_RGB)
    {
      generate_color_coef (_coef, set.coef, marq.saturation);
    }
  else			// use identity matrix
    {			// FIXME: fold into generate_color_coef()
      for (int i = 0; i < 9; ++i)
	_coef[i] = 0.0;

      _coef[0] = 1.0;
      _coef[4] = 1.0;
      _coef[8] = 1.0;
    }

  _threshold = marq.threshold;
  _speed = set.enable_draft_mode;
  _focus = marq.focus;

  if (PISA_PT_BW == set.imgtype.pixeltype)
    _usm = 0;
  else
    _usm = set.usm;

  if (PISA_DESCREEN_ON == set.imgtype.de_screening)
    _de_screening = 1;
  else
    _de_screening = 0;


  // check for error conditions
  pisa_error_id err = PISA_ERR_SUCCESS;

  if (set.imgtype.de_screening == PISA_DESCREEN_ON)
    {
      if (   _resolution[0] > _max_descreen_resolution
	  || _resolution[1] > _max_descreen_resolution)
	err = PISA_ERR_MRRESTOOHIGH;
    }
  return err;
}

#include "pisa_view_manager.h"
pisa_error_id
scan_manager::set_scan_parameters (const settings& set, const marquee& marq,
				   int resolution)
{
  _pixeltype	= PISA_PT_RGB;
  _bitdepth	= PISA_BD_8;
  _dropout	= PISA_DO_NONE;
  _monoopt	= PISA_MO_NONE;
  _halftone	= PISA_HT_NONE;

  _offset[0]	= marq.offset.x;
  _offset[1]	= marq.offset.y;
  _area[0]	= marq.area.x;
  _area[1]	= marq.area.y;
  _resolution[0]= resolution;
  _resolution[1]= resolution;
  _zoom[0]	= 100;
  _zoom[1]	= 100;

  _brightness	= marq.brightness;
  _contrast	= marq.contrast;
  _deskew	= get_deskew();

  // gamma table
  for (int i = 0; i < 256; ++i)
    {
      _gamma.gamma_r[i] = i;
      _gamma.gamma_g[i] = i;
      _gamma.gamma_b[i] = i;
    }

  // profile matrix
  for (int i = 0; i < 9; ++i)
    _coef[i] = 0.0;

  _coef[0] = 1.0;
  _coef[4] = 1.0;
  _coef[8] = 1.0;

  _threshold	= 0;
  _speed	= 1;
  _focus	= marq.focus;
  _usm		= 0;
  _de_screening	= 0;

  return PISA_ERR_SUCCESS;
}
