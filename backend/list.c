/*  list.c -- A poor implementation of a linked list
 *  Copyright (C) 2008  SEIKO EPSON CORPORATION
 *
 *  License: GPLv2+|iscan
 *  Authors: AVASYS CORPORATION
 *
 *  This file is part of the SANE backend distributed with Image Scan!
 *
 *  Image Scan!'s SANE backend is free software.
 *  You can redistribute it and/or modify it under the terms of the GNU
 *  General Public License as published by the Free Software Foundation;
 *  either version 2 of the License or at your option any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *  You ought to have received a copy of the GNU General Public License
 *  along with this package.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 *  Linking Image Scan!'s SANE backend statically or dynamically with
 *  other modules is making a combined work based on this SANE backend.
 *  Thus, the terms and conditions of the GNU General Public License
 *  cover the whole combination.
 *
 *  As a special exception, the copyright holders of Image Scan!'s SANE
 *  backend give you permission to link Image Scan!'s SANE backend with
 *  SANE frontends that communicate with Image Scan!'s SANE backend
 *  solely through the SANE Application Programming Interface,
 *  regardless of the license terms of these SANE frontends, and to
 *  copy and distribute the resulting combined work under terms of your
 *  choice, provided that every copy of the combined work is
 *  accompanied by a complete copy of the source code of Image Scan!'s
 *  SANE backend (the version of Image Scan!'s SANE backend used to
 *  produce the combined work), being distributed under the terms of
 *  the GNU General Public License plus this exception.  An independent
 *  module is a module which is not derived from or based on Image
 *  Scan!'s SANE backend.
 *
 *  As a special exception, the copyright holders of Image Scan!'s SANE
 *  backend give you permission to link Image Scan!'s SANE backend with
 *  independent modules that communicate with Image Scan!'s SANE
 *  backend solely through the "Interpreter" interface, regardless of
 *  the license terms of these independent modules, and to copy and
 *  distribute the resulting combined work under terms of your choice,
 *  provided that every copy of the combined work is accompanied by a
 *  complete copy of the source code of Image Scan!'s SANE backend (the
 *  version of Image Scan!'s SANE backend used to produce the combined
 *  work), being distributed under the terms of the GNU General Public
 *  License plus this exception.  An independent module is a module
 *  which is not derived from or based on Image Scan!'s SANE backend.
 *
 *  Note that people who make modified versions of Image Scan!'s SANE
 *  backend are not obligated to grant special exceptions for their
 *  modified versions; it is their choice whether to do so.  The GNU
 *  General Public License gives permission to release a modified
 *  version without this exception; this exception also makes it
 *  possible to release a modified version which carries forward this
 *  exception.
 */


/*! \file
    \brief  Implements a linked list.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "list.h"

#include <string.h>

/*! Create a new list.
 */
list* list_create ()
{
  list* lst = 0;
  
  lst = t_calloc (1, list);
  return lst;
}

/*! Destroy a list.
 *  Deletes the list related structures, and the contained data with the
 *  given destructor function. If a destructor is not provided the data is
 *  left as is.
 */
void list_destroy (list* lst, void (*dtor)(void*))
{
  if (!lst) return;

  list_entry* entry = lst->head;
  list_entry* tmp;

  while (entry != NULL)
  {
    if (dtor && entry->data)
      (*dtor) (entry->data);
    
    tmp = entry->next;
    delete (entry);
    entry = tmp;
  }

  delete (lst);
}

/*! Adds a new element to the end of the list.
 *  Does *not* make a copy.
 */
bool list_append (list* lst, void* new_data)
{
  if (!lst) return false;

  list_entry* entry = 0;

  entry = t_calloc (1, list_entry);
  if (!entry) return false;

  entry->data = new_data;

  if (0 == lst->num_entries)
    {
      lst->head = entry;
      lst->tail = entry;
      lst->cur = entry;
    }
  else
    {
      lst->tail->next = entry;
      lst->tail = entry;
    }

  lst->num_entries += 1;
  return true;
}

/*! Creates a NULL terminated array of pointers to list entries.
 */
void** list_normalize (const list *lst)
{
  void **nlst;

  if (!lst) return NULL;

  nlst = t_malloc (lst->num_entries + 1, void *);
  if (nlst)
    {
      list *p = (list *) lst;
      list_entry *cur = p->cur;

      void *entry;
      int i = 0;

      list_reset (p);
      while ((entry = list_next (p)))
        {
          nlst[i++] = entry;
        }
      nlst[i++] = NULL;
      p->cur = cur;
    }
  return nlst;
}

/*! Obtain the number of elements in the list
 */
size_t list_size (list* lst)
{
  if (!lst) return 0;
  return lst->num_entries;
}

/*! Reset iteration to start from the beginning of the list
 */
void list_reset (list* lst)
{
  if (!lst) return;

  lst->cur = lst->head;
}

/*! Proceed to the next element in the list.
 *  Used to iterate over the list items.
 */
void* list_next (list* lst)
{
  if (!lst || !lst->cur) return 0;

  void* rv = lst->cur->data;
  lst->cur = lst->cur->next;

  return rv;
}
