/* 
   SANE EPSON backend
   Copyright (C) 2001 SEIKO EPSON CORPORATION

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

#ifndef ___PISA_GIMP_H
#define ___PISA_GIMP_H

#include <config.h>
#include <gtk/gtk.h>

#ifdef HAVE_ANY_GIMP
#include <libgimp/gimp.h>
#include "gimp-plugin.h"

#ifndef HAVE_GIMP_2
#ifdef HAVE_LIBGIMP_GIMPFEATURES_H
#include <libgimp/gimpfeatures.h>
#else
#define GIMP_CHECK_VERSION(major, minor, micro) 0
#endif /* HAVE_LIBGIMP_GIMPFEATURES_H */

#ifdef GIMP_CHECK_VERSION
#if GIMP_CHECK_VERSION(1,1,25)
	/* ok, we have the new gimp interface */
#else
	/* we have the old gimp interface and need the compatibility header file */
#include "pisa_gimp_1_0_patch.h" 
#endif
#else
	/* we have the old gimp interface and need the compatibility header file */
#include "pisa_gimp_1_0_patch.h" 
#endif /*GIMP_CHECK_VERSION*/
#endif	/* HAVE_GIMP_2 */
#endif /* HAVE_ANY_GIMP */
	
gint pisa_gimp_main ( gint argc, gchar * argv [ ] );
void pisa_gimp_quit ( void );

#ifdef HAVE_ANY_GIMP

gchar * pisa_gimp_gtkrc ( void );
gboolean pisa_gimp_use_xshm ( void );

class gimp_scan
{
 public:

  // operation
  int	create_gimp_image ( int width,
							int height,
							int pixeltype,
							int depth );

  unsigned char *		get_next_buf ( void );
  int	set_image_rect ( void );
  int	finish_scan ( int cancel );

 private:

  // operation
  int bw2gray ( void );

  // attribute  
  int			m_rows;

  int			m_width;
  int			m_height;
  int			m_pixeltype;
  int			m_depth;
  int			m_rowbytes;

  int			m_image_id;
  int			m_layer_id;
  guchar		* m_tile;
  GimpDrawable	* m_drawable;
  GimpPixelRgn	m_region;

};

#endif // HAVE_ANY_GIMP


#endif // ___PISA_GIMP_H
