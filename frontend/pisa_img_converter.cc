/* 
   SANE EPSON backend
   Copyright (C) 2001 SEIKO EPSON CORPORATION

   Date         Author      Reason
   07/10/2001   Peter       New

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

/*--------------------------------------------------------------*/
#include "pisa_img_converter.h"
#include "pisa_error.h"

/*--------------------------------------------------------------*/


/*Convert 8bpp grayscale pixels to 24bpp rgb pixels*/
int convert_grayscale_to_rgb(BYTE* in_buf, long width, long height, BYTE* out_rgb_buf)
{
  long i,j,ofs;	
  ofs = 0;
  for( i = 0; i<height; i++)
    for( j = 0; j<width; j++, ofs+=3)
      {
	/*just copy the gray value 3 times to make rgb*/
	out_rgb_buf[ofs] = in_buf[i*width+j];
	out_rgb_buf[ofs+1] = out_rgb_buf[ofs];
	out_rgb_buf[ofs+2] = out_rgb_buf[ofs];
      }
  return PISA_ERR_SUCCESS;
}

/*Convert 1bpp b&w pixels to 24bpp rgb pixels*/
int convert_binary_to_rgb(BYTE* in_buf, long width, long height, BYTE* out_rgb_buf)
{
  long i,j,in_ofs, out_ofs;
  int rgb_val;
  int bitshift;
  unsigned int temp_byte = 0;

  //offset into input image
  in_ofs = 0;
  //offset into output image	
  out_ofs = 0;
	
  for( i = 0; i<height; i++)
    {
      j = 0;
      bitshift = -1;
      in_ofs  = i*(int)((width+7)/8);
      while( j<width)
	{
	  //get a new byte every 8 columns
	  if(j%8 == 0)
	    {
	      temp_byte = in_buf[in_ofs+(int)(j/8)];
	      bitshift = 7; 
	    }

	  //black or white pixel
	  if((temp_byte >> bitshift) & 1)
	    rgb_val = IMG_CONVERTER_RGB_WHITE;
	  else
	    rgb_val = IMG_CONVERTER_RGB_BLACK;
				
	  //write the rgb values
	  out_rgb_buf[out_ofs] = (unsigned int) rgb_val;
	  out_rgb_buf[out_ofs+1] = (unsigned int)rgb_val;
	  out_rgb_buf[out_ofs+2] = (unsigned int)rgb_val;
	  j++;
	  out_ofs+=3;
	  bitshift--;
	}
    }
  return PISA_ERR_SUCCESS;
}


