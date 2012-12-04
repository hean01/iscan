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

#ifndef ___PISA_STRUCTS_H
#define ___PISA_STRUCTS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_GTK_GTK_H
#include <gtk/gtk.h>
#ifndef HAVE_GTK_2
#include <gdk_imlib.h>
#endif
#endif

// scanner
/*--------------------------------------------------------------*/
typedef struct _gamma_struct
{
  unsigned char gamma_r [ 256 ];
  unsigned char gamma_g [ 256 ];
  unsigned char gamma_b [ 256 ];
} gamma_struct;


/*--------------------------------------------------------------*/
typedef struct _imagetype
{
  long		id;
  char		pixeltype;
  char		bitdepth;
  char		de_screening;
  char		ae_option;
  char		dropout;
  char		monoopt;
  char		halftone;
} imagetype;

// ui
/*--------------------------------------------------------------*/
template <class Type>
struct _point
{
  Type	x, y;

  _point ( ) { x = y = 0; }
  _point ( Type u, Type v = 0 ) { x = u, y = v; }
  _point ( const _point & pt ) { x = pt.x; y = pt.y; }
  _point & operator= ( const _point & pt )
  {
    if ( & pt != this )
      {
	x = pt.x;
	y = pt.y;
      }
    return ( * this );
  }

  // == operator
  friend int operator== ( const _point & pt1, const _point & pt2 )
  {
    return ( pt1.x == pt2.x && pt1.y == pt2.y );
  }

  // - operator
  friend _point operator- ( const _point & pt1, const _point & pt2 )
  {
    return ( _point ( pt1.x - pt2.x, pt1.y - pt2.y ) );
  }

  // + operator
  friend _point operator+ ( const _point & pt1, const _point & pt2 )
  {
    return ( _point ( pt1.x + pt2.x, pt1.y + pt2.y ) );
  }
};

typedef _point <double>	_pointD;
typedef _point <long>	_pointL;

/*--------------------------------------------------------------*/
template <class Type>
struct _rect
{
  Type	top, bottom, right, left;

  _rect ( ) { top = bottom = right = left = Type ( 0 ); }
  _rect ( Type _top, Type _bottom, Type _right, Type _left ):
    top ( _top ), bottom ( _bottom ), right ( _right ), left ( _left ) { }
  _rect ( const _rect & x )
  {
    top = x.top, bottom = x.bottom;
    right = x.right, left = x.left;
  }
  _rect & operator= ( const _rect & x )
  {
    if ( & x != this )
      {
	top = x.top, bottom = x.bottom;
	right = x.right, left = x.left;
      }
    return ( * this );
  }

  // == operator
  friend int operator== ( const _rect & rc1, const _rect & rc2 )
  {
    return ( rc1.top == rc2.top && rc1.bottom == rc2.bottom &&
	     rc1.right == rc2.right && rc1.left == rc2.left );
  }

  // - operator
  friend _rect operator- ( const _rect & rc1, const _rect & rc2 )
  {
    return ( _rect ( rc1.top - rc2.top, rc1.bottom - rc2.bottom,
		     rc1.right - rc2.right, rc1.left - rc2.left ) );
  }

  // + operator
  friend _rect operator+ ( const _rect & rc1, const _rect & rc2 )
  {
    return ( _rect ( rc1.top + rc2.top, rc1.bottom + rc2.bottom,
		     rc1.right + rc2.right, rc1.left + rc2.left ) );
  }

  // empty?
  friend int is_rect_empty ( const _rect & rc )
  {
    return ( rc.top == 0 && rc.bottom == 0 && rc.right == 0 && rc.left == 0 );
  }
};

typedef _rect <_point <long> >	_rectPL;
typedef _rect <double>		_rectD;
typedef _rect <long>		_rectL;


#ifdef HAVE_GTK_GTK_H
/*--------------------------------------------------------------*/
typedef struct _toolbar_items
{
  GtkWidget		* widget;
  char			** xpm;
  GtkSignalFunc		* func;
} toolbar_items;

/*--------------------------------------------------------------*/
typedef struct _menu_items
{
  long			id;
  GtkWidget		* widget;
  const char		* name;
  GtkSignalFunc		* func;
} menu_items;


/*------------------------------------------------------------*/
typedef struct _scale_items
{
  long			id;
  GtkWidget		* widget;
  GtkAdjustment		* adjust;
  GtkSignalFunc		* func;
  float			val;
  float			min;
  float			max;
  float			step;
  float			page;
} scale_items;
#endif

typedef struct
{
  unsigned char		* m_img;
  long			m_width;
  long			m_height;
  long			m_rowbytes;
} _pisa_image_info;

#include <cstddef>

class pisa_image_info : public _pisa_image_info
{
public:
  size_t m_bits_per_pixel;

  pisa_image_info () : m_bits_per_pixel (24)
    {
      m_img = 0;
      m_width = 0;
      m_height = 0;
      m_rowbytes = 0;
    };
};

#endif // ___PISA_STRUCTS_H

