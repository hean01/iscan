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

#ifndef ___PISA_TOOL_H
#define ___PISA_TOOL_H

#include <gtk/gtk.h>
#include <config.h>
#include "pisa_structs.h"

template <class Type>
Type similarity ( const Type & A,
		  const Type & a,
		  const Type & b )
{
  Type B;

  if ( a == 0 )
    return 0;

  if ( 0 < A )
    B = ( b * A + a / 2 ) / a;
  else
    B = ( b * A - a / 2 ) / a;

  return B;
}


template <class Type>
int pt_in_rect ( const _rect <Type> & rect, const _point <Type> & pt )
{
  if ( rect.left < pt.x && pt.x < rect.right &&
       rect.top  < pt.y && pt.y < rect.bottom )
    return 1;
  else
    return 0;
}

GtkWidget * xpm2widget ( GtkWidget * widget, char ** xpm_data );
GtkWidget * xpmlabelbox ( GtkWidget * parent, char ** xpm_data,
                          const char * text );

GtkWidget * pisa_create_toolbar ( const GtkWidget * parent,
				  toolbar_items * list );
GtkWidget * pisa_create_option_menu ( menu_items * list );
GtkWidget * pisa_create_scale ( scale_items * items );



#endif // ___PISA_TOOL_H

