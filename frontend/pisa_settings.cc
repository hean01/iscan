/* 
   SANE EPSON backend
   Copyright (C) 2001, 2008 SEIKO EPSON CORPORATION

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

/*------------------------------------------------------------*/
#include "pisa_structs.h"
#include "pisa_settings.h"

/*------------------------------------------------------------*/
void settings::init ( marquee ** marq )
{
  marquee_node	 * new_node;

  new_node = new marquee_node;
  
  new_node->next = 0;
  new_node->data = * marq;

  top = new_node;
}

long
settings::get_marquee_size (void) const
{
  long		size;
  marquee_node	* tmp;

  size = 1;
  tmp = top;

  while ( tmp->next )
    {
      tmp = tmp->next;
      size++;
    }

  return size;
}

const marquee&
settings::get_marquee (long index) const
{
  return const_cast <settings *> (this)->get_marquee (index);
}

marquee&
settings::get_marquee (long index)
{
  if (0 > index)
    index = get_marquee_size () - 1;

  marquee_node *current = top;
  long		counter = 0;

  while (index != counter++)
    current = current->next;

  return *current->data;
}

/*------------------------------------------------------------*/
void settings::add_marquee ( marquee ** marq )
{
  marquee_node	* tmp;
  marquee_node	* new_node;

  new_node = new marquee_node;

  tmp = top;

  while ( tmp->next )
    tmp = tmp->next;

  new_node->next	= 0;
  new_node->data	= * marq;

  tmp->next = new_node;
}


void
settings::del_marquee (long index)
{
  if (!top) return;

  if (0 > index)
    index = get_marquee_size () - 1;

  int cnt = 0;
  marquee_node *del_node = top;
  marquee_node *pre_node = top;

  while (cnt != index - 1)
    {
      if (pre_node->next)
	{
	  pre_node = pre_node->next;
	  ++index;
	}
      else
	return;	// error
    }

  del_node = pre_node->next;
  pre_node->next = del_node->next;

  delete del_node->data;
  delete del_node;
}

/*------------------------------------------------------------*/
void settings::delete_all ( void )
{
  while ( top )
    {
      delete top->data;
      top = top->next;
    }
}

