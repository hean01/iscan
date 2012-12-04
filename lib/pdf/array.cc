//  array.cc -- PDF array objects
//  Copyright (C) 2008  SEIKO EPSON CORPORATION
//
//  This file is part of the 'iscan' program.
//
//  The 'iscan' program is free-ish software.
//  You can redistribute it and/or modify it under the terms of the GNU
//  General Public License as published by the Free Software Foundation;
//  either version 2 of the License or at your option any later version.
//
//  This program is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY;  without even the implied warranty of FITNESS
//  FOR A PARTICULAR PURPOSE or MERCHANTABILITY.
//  See the GNU General Public License for more details.
//
//  You should have received a verbatim copy of the GNU General Public
//  License along with this program; if not, write to:
//
//      Free Software Foundation, Inc.
//      59 Temple Place, Suite 330
//      Boston, MA  02111-1307  USA
//
//  As a special exception, the copyright holders give permission
//  to link the code of this program with the esmod library and
//  distribute linked combinations including the two.  You must obey
//  the GNU General Public License in all respects for all of the
//  code used other than esmod.


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "array.hh"

namespace iscan
{

namespace pdf
{

array::~array ()
{
  store_citer it;

  for (it = _mine.begin (); _mine.end () != it; ++it)
    {
      object* obj = *it;
      delete obj;
      obj = NULL;
    }
}

void
array::insert (object *value)
{
  _store.push_back (value);
}

void
array::insert (primitive value)
{
  primitive *copy = new primitive ();

  *copy = value;
  _mine.push_back (copy);
  insert (copy);
}

void
array::insert (object value)
{
  object *copy = new object ();

  *copy = value;
  _mine.push_back (copy);
  insert (copy);
}

size_t
array::size () const
{
  return _store.size ();
}

const object *
array::operator[] (size_t index) const
{
  return _store[index];
}

void
array::print (FILE* fp) const
{
  store_citer it;

  fprintf (fp, "[ ");
  if (4 < _store.size ())
    {
      fprintf (fp, "\n");
    }
  for (it = _store.begin (); _store.end () != it; ++it)
    {
      (*it)->print (fp);
      fprintf (fp, " ");
      if (4 < _store.size ())
        {
          fprintf (fp, "\n");
        }
    }
  fprintf (fp, "]");
}

}       // namespace pdf
}       // namespace iscan
