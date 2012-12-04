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

/*------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>

/*------------------------------------------------------------*/
#include "pisa_scan_tool.h"
#include "pisa_error.h"
#include "pisa_default_val.h"

/*------------------------------------------------------------*/
static int build_lineart ( pisa_image_info * info,
			   unsigned char do_clr,
			   long threshold );
static int build_halftone ( pisa_image_info * info,
			    unsigned char do_clr,
			    unsigned char halftone,
			    long threshold );
static int build_dither ( pisa_image_info * info,
			  unsigned char do_clr,
			  unsigned char halftone,
			  long threshold );
static unsigned char dropout ( unsigned char red,
			       unsigned char grn,
			       unsigned char blu,
			       unsigned char do_clr,
			       long thdiff = 0 );

/*------------------------------------------------------------*/
int calc_rowbytes ( int width, pisa_pixel_type type )
{
  int rowbytes;

  rowbytes = width;

  switch ( type )
    {
    case PISA_PT_BW:
      rowbytes = ( width + 7 ) / 8;
      break;

    case PISA_PT_GRAY:
      rowbytes = width;
      break;

    case PISA_PT_RGB:
      rowbytes = width * 3;
      break;
    }

  return rowbytes;
}

/*------------------------------------------------------------*/
int calc_bitperpix ( pisa_pixel_type pixel, pisa_bitdepth_type bitdepth )
{
  int bitperpix;

  bitperpix = 8;
  bitdepth = bitdepth;

  switch ( pixel )
    {
    case PISA_PT_BW:
      bitperpix = 1;
      break;

    case PISA_PT_GRAY:
      bitperpix = 8;
      break;

    case PISA_PT_RGB:
      bitperpix = 24;
      break;
    }

  return bitperpix;
}

/*------------------------------------------------------------*/
int tool_lut ( pisa_image_info * info,
	       const gamma_struct * lut )
{
  int i, j;
  unsigned char * img;

  for ( i = 0; i < info->m_height; i++ )
    {
      img = info->m_img + i * info->m_rowbytes;
  
      for ( j = 0; j < info->m_width; j++ )
	{
	  // red
	  * img = lut->gamma_r [ * img ];
	  img++;
	  // green
	  * img = lut->gamma_g [ * img ];
	  img++;
	  // blue
	  * img = lut->gamma_b [ * img ];
	  img++;
	}
    }
  
  return PISA_ERR_SUCCESS;
}


/*------------------------------------------------------------*/
int tool_matrix ( pisa_image_info * info,
		  int saturation,
		  const double * color_profile )
{
  int		i, j;
  int		color [ 9 ];
  double	color_corr [ 9 ];
  int		red, grn, blu;
  int		result_red, result_grn, result_blu;
  unsigned char	* img;
  int		max;

  max = 0xff << 10;

  generate_color_coef ( color_corr, color_profile, saturation );

  for ( i = 0; i < 9; i++ )
    color [ i ] = ( int ) ( color_corr [ i ] * ( 1 << 10 ) );
  
  for ( i = 0; i < info->m_height; i++ )
    {
      img = info->m_img + i * info->m_rowbytes;

      for ( j = 0; j < info->m_width; j++ )
	{
	  red = * ( img + 0 );
	  grn = * ( img + 1 );
	  blu = * ( img + 2 );
	  
	  result_red = ( int ) ( color [ 0 ] * red + color [ 1 ] * grn + color [ 2 ] * blu );
	  result_grn = ( int ) ( color [ 3 ] * red + color [ 4 ] * grn + color [ 5 ] * blu );
	  result_blu = ( int ) ( color [ 6 ] * red + color [ 7 ] * grn + color [ 8 ] * blu );
	  
	  if ( result_red < 0   ) result_red = 0;
	  if ( result_red > max ) result_red = max;
	  if ( result_grn < 0   ) result_grn = 0;
	  if ( result_grn > max ) result_grn = max;
	  if ( result_blu < 0   ) result_blu = 0;
	  if ( result_blu > max ) result_blu = max;
	  
	  * ( img + 0 ) = result_red >> 10;
	  * ( img + 1 ) = result_grn >> 10;
	  * ( img + 2 ) = result_blu >> 10;
	  
	  img += 3;
	}
    }
  
  return PISA_ERR_SUCCESS;
}

/*------------------------------------------------------------*/
int generate_color_coef ( double * dst,
			  const double * src,
			  int saturation )
{
  int		i, j;
  int		color_corr [ 3 ] [ 3 ];
  double	tmp_color [ 3 ] [ 3 ];
  int		val;

  // build saturate
  val = 300 + 2 * saturation;

  for ( i = 0; i < 3; i++ )
    for ( j = 0; j < 3; j++ )
      color_corr [ i ] [ j ] = -saturation;

  color_corr [ 0 ] [ 0 ] =
  color_corr [ 1 ] [ 1 ] =
  color_corr [ 2 ] [ 2 ] = val;

  // build color filter
  for ( i = 0; i < 3; i++ )
    for ( j = 0; j < 3; j++ )
      {
	tmp_color [ i ] [ j ]  = color_corr [ 0 ] [ j ] * src [ i * 3 + 0 ];
	tmp_color [ i ] [ j ] += color_corr [ 1 ] [ j ] * src [ i * 3 + 1 ];
	tmp_color [ i ] [ j ] += color_corr [ 2 ] [ j ] * src [ i * 3 + 2 ];
      }
  
  for ( i = 0; i < 3; i++ )
    for ( j = 0; j < 3; j++ )
      dst [ i * 3 + j ] = tmp_color [ i ] [ j ] / 300.0;
  
  return PISA_ERR_SUCCESS;
}


/*------------------------------------------------------------*/
int build_gray ( pisa_image_info * info,
		 unsigned char do_clr )
{
  long		i, j;
  unsigned char val;
  unsigned char	* img;

  for ( i = 0; i < info->m_height; i++ )
    {
      img = info->m_img + i * info->m_rowbytes;
      for ( j = 0; j < info->m_width; j++ )
	{
	  val = dropout ( * ( img + 0 ), * ( img + 1 ), * ( img + 2 ),
			  do_clr );
	  * ( img + 0 ) = val;
	  * ( img + 1 ) = val;
	  * ( img + 2 ) = val;
	  img += 3;
	}
    }

  return PISA_ERR_SUCCESS;
}


/*------------------------------------------------------------*/
int build_bw ( pisa_image_info * info,
	       unsigned char do_clr,
	       unsigned char halftone,
	       long threshold )
{
  int	ret;

  ret = PISA_ERR_PARAMETER;

  switch ( halftone )
    {
    case PISA_HT_NONE:
      ret = build_lineart ( info, do_clr, threshold );
      break;

    case PISA_HT_TONEA:
    case PISA_HT_TONEB:
    case PISA_HT_TONEC:
      ret = build_halftone ( info, do_clr, halftone, threshold );
      break;

    case PISA_HT_DITHERA:
    case PISA_HT_DITHERB:
    case PISA_HT_DITHERC:
    case PISA_HT_DITHERD:
      ret = build_dither ( info, do_clr, halftone, threshold );
      break;
    }

  return ret;
}



/*------------------------------------------------------------*/
static int build_lineart ( pisa_image_info * info,
			   unsigned char do_clr,
			   long threshold )
{
  int		i, j;
  unsigned char	val;
  unsigned char	* img;
  long		thdiff;

  thdiff = 128 - threshold;

  for ( i = 0; i < info->m_height; i++ )
    {
      img = info->m_img + i * info->m_rowbytes;
      for ( j = 0; j < info->m_width; j++ )
	{
	  val = dropout ( * img, * ( img + 1 ), * ( img + 2 ),
			  do_clr, thdiff );
	  if ( val > 128 )
	    val = 255;
	  else
	    val = 0;
	  
	  * ( img + 0 ) = val;
	  * ( img + 1 ) = val;
	  * ( img + 2 ) = val;
	  img += 3;
	}
    }

  return PISA_ERR_SUCCESS;
}


/*------------------------------------------------------------*/
static long Hard [ 8 ] [ 8 ] =
{
  { 128, 128, 128, 128, 128, 128, 128, 128 },
  { 128, 128, 128, 128, 128, 128, 128, 128 },
  { 128, 128, 128, 128, 128, 128, 128, 128 },
  { 128, 128, 128, 128, 128, 128, 128, 128 },
  { 128, 128, 128, 128, 128, 128, 128, 128 },
  { 128, 128, 128, 128, 128, 128, 128, 128 },
  { 128, 128, 128, 128, 128, 128, 128, 128 },
  { 128, 128, 128, 128, 128, 128, 128, 128 }
};

static long Soft [ 8 ] [ 8 ] =
{
  { 131, 136, 136, 131, 124, 119, 119, 124 },
  { 136, 149, 149, 136, 119, 106, 106, 119 },
  { 136, 149, 149, 136, 119, 106, 106, 119 },
  { 128, 128, 128, 128, 128, 128, 128, 128 },
  { 131, 136, 136, 131, 124, 119, 119, 124 },
  { 124, 119, 119, 124, 131, 136, 136, 131 },
  { 119, 106, 106, 119, 136, 149, 149, 136 },
  { 124, 119, 119, 124, 131, 136, 136, 131 }
};

static long Net [ 8 ] [ 8 ] =
{
  { 146, 173, 173, 146, 109,  82,  82, 109 },
  { 173, 236, 236, 173,  82,  19,  19,  82 },
  { 173, 236, 236, 173,  82,  19,  19,  82 },
  { 146, 173, 173, 146, 109,  82,  82, 109 },
  { 109,  82,  82, 109, 148, 173, 173, 146 },
  {  82,  19,  19,  82, 173, 236, 236, 173 },
  {  82,  19,  19,  82, 173, 236, 236, 173 },
  { 109,  89,  82, 109, 146, 173, 173, 146 }
};

static long Bayer [ 4 ] [ 4 ] =
{
  { 248, 120, 216,  88 },
  {  56, 184,  24, 152 },
  { 200,  72, 232, 104 },
  {   8, 136,  40, 168 }
};

static long Spiral [ 4 ] [ 4 ] =
{
  {  40, 152, 136,  24 },
  { 168, 248, 232, 120 },
  { 184, 200, 216, 104 },
  {  56, 72,   88,   8 }
};

static long Net4x4 [ 4 ] [ 4 ] =
{
  {  24,  40, 152, 104 },
  {  56, 248, 232, 136 },
  { 168, 200, 216,  88 },
  { 120, 184,  72,   8 }
};

static long Net8x8 [ 8 ] [ 8 ] =
{
  { 236, 188,  52,   4,  68, 100, 164, 228 },
  { 180,  44,  12, 140, 132,  92, 108, 172 },
  {  36,  20, 148, 212, 204, 124,  84,  76 },
  {  28, 156, 220, 252, 244, 196, 116,  60 },
  {  68, 100, 164, 228, 236, 188,  52,   4 },
  { 132,  92, 108, 172, 180,  44,  12, 140 },
  { 204, 124,  84,  76,  36,  20, 148, 212 },
  { 244, 196, 116,  60,  28, 156, 220, 252 }
};

static struct _halftone
{
  long	* matrix;
  long	size;
} proc_halftone [ 8 ] =
{
  { 0,            8 },
  { Hard [ 0 ],   8 },
  { Soft [ 0 ],   8 },
  { Net [ 0 ],    8 },
  { Bayer [ 0 ],  4 },
  { Spiral [ 0 ], 4 },
  { Net4x4 [ 0 ], 4 },
  { Net8x8 [ 0 ], 8 }
};

static int build_halftone ( pisa_image_info * info,
			    unsigned char do_clr,
			    unsigned char halftone,
			    long threshold )
{
  int		i, j, k;
  long		sc1, sc2, sc3, sc4, sc5, sc6, long_val;
  unsigned char	val;
  unsigned char	* img;
  long		thdiff, thre;
  long		* pattern;
  long		matrix_size;
  long		* rowa, * rowb, * rowc;

  thdiff = 128 - threshold;

  pattern = proc_halftone [ halftone ].matrix;
  if ( pattern == 0 )
    return PISA_ERR_PARAMETER;

  matrix_size = proc_halftone [ halftone ].size;

  rowa = new long [ info->m_width + 10 ];
  rowb = new long [ info->m_width + 10 ];
  rowc = new long [ info->m_width + 10 ];

  ::memset ( rowa, 0, ( info->m_width + 10 ) * sizeof ( long ) );
  ::memset ( rowb, 0, ( info->m_width + 10 ) * sizeof ( long ) );
  ::memset ( rowc, 0, ( info->m_width + 10 ) * sizeof ( long ) );
  
  for ( i = 0; i < info->m_height; i++ )
    {
      img = info->m_img + i * info->m_rowbytes;

      for ( j = 0; j < info->m_width; j++ )
	{
	  k = j + 2;
	  sc1 = * ( rowa + k     );
	  sc2 = * ( rowb + k - 1 );
	  sc3 = * ( rowb + k     );
	  sc4 = * ( rowb + k + 1 );
	  sc5 = * ( rowc + k - 2 );
	  sc6 = * ( rowc + k - 1 );
	  
	  val = dropout ( * img, * ( img + 1 ), *  ( img + 2 ),
			  do_clr, thdiff );
	  long_val = val + ( sc3 + sc6 ) / 4 + ( sc1 + sc2 + sc4 + sc5 ) / 8;
	  thre = * ( pattern + ( i & ( matrix_size - 1 ) ) * matrix_size +
		     ( j & ( matrix_size - 1 ) ) );
	  if ( long_val >= thre )
	    {
	      val = 255;
	      * ( rowc + k ) = long_val - 255;
	    }
	  else
	    {
	      val = 0;
	      * ( rowc + k ) = long_val;
	    }
	  
	  * ( img + 0 ) = val;
	  * ( img + 1 ) = val;
	  * ( img + 2 ) = val;
	  img += 3;
	}
      ::memcpy ( rowa, rowb, ( info->m_width + 10 ) * sizeof ( long ) );
      ::memcpy ( rowb, rowc, ( info->m_width + 10 ) * sizeof ( long ) );
      ::memset ( rowc,    0, ( info->m_width + 10 ) * sizeof ( long ) );
    }
  
  delete [ ] rowa;
  delete [ ] rowb;
  delete [ ] rowc;

  return PISA_ERR_SUCCESS;
}


/*------------------------------------------------------------*/
static int build_dither ( pisa_image_info * info,
			  unsigned char do_clr,
			  unsigned char halftone,
			  long threshold )
{
  int		i, j;
  unsigned char	val;
  unsigned char	* img;
  long		thdiff, thre;
  long		* pattern;
  long		matrix_size;

  thdiff = 128 - threshold;

  pattern = proc_halftone [ halftone ].matrix;
  if ( pattern == 0 )
    return PISA_ERR_PARAMETER;

  matrix_size = proc_halftone [ halftone ].size;

  for ( i = 0; i < info->m_height; i++ )
    {
      img = info->m_img + i * info->m_rowbytes;
      for ( j = 0; j < info->m_width; j++ )
	{
	  val = dropout ( * img, * ( img + 1 ), * ( img + 2 ),
			  do_clr, thdiff );
	  thre = * ( pattern + ( i & ( matrix_size - 1 ) ) * matrix_size +
		     ( j & ( matrix_size - 1 ) ) );
	  if ( ( long ) val >= thre )
	    val = 255;
	  else
	    val = 0;
	  * ( img + 0 ) = val;
	  * ( img + 1 ) = val;
	  * ( img + 2 ) = val;
	}
    }
  
  return PISA_ERR_SUCCESS;
}

/*------------------------------------------------------------*/
static unsigned char dropout ( unsigned char red,
			       unsigned char grn,
			       unsigned char blu,
			       unsigned char do_clr,
			       long thdiff )
{
  unsigned char ret;
  long val;

  ret = 0;

  if ( thdiff != 0 )
    {
      // red
      val = red + thdiff;
      if ( val < 0   ) val = 0;
      if ( val > 255 ) val = 255;
      red = val;

      // green
      val = grn + thdiff;
      if ( val < 0   ) val = 0;
      if ( val > 255 ) val = 255;
      grn = val;

      // blue
      val = blu + thdiff;
      if ( val < 0   ) val = 0;
      if ( val > 255 ) val = 255;
      blu = val;
    }

  switch ( do_clr )
    {
    case PISA_DO_NONE:
      val = red * 2 + grn * 6 + blu * 2;
      if ( val == 0 )
	ret = 0;
      else
	ret = ( unsigned char ) ( val / 10 );
      break;

    case PISA_DO_RED:
      ret = red;
      break;

    case PISA_DO_GREEN:
      ret = grn;
      break;

    case PISA_DO_BLUE:
      ret = blu;
      break;
    }
      
  return ret;
}
