//  dictionary.cc -- PDF dictionaries
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

#include "dictionary.hh"

namespace iscan
{

namespace pdf
{

dictionary::~dictionary ()
{
  store_citer it;

  for (it = _mine.begin (); _mine.end () != it; ++it)
    {
      object* obj = it->second;
      delete obj;
      obj = NULL;
    }
}

void
dictionary::insert (const char *key, object *value)
{
  if (_mine.end () != _mine.find (key))
    {
      delete _mine[key];
    }
  _store[key] = value;
}

void
dictionary::insert (const char *key, primitive value)
{
  primitive *copy = new primitive ();

  *copy = value;
  insert (key, copy);
  _mine[key] = copy;
}

void
dictionary::insert (const char *key, object value)
{
  object *copy = new object ();

  *copy = value;
  insert (key, copy);
  _mine[key] = copy;
}

size_t
dictionary::size () const
{
  return _store.size ();
}

const object *
dictionary::operator[] (const char* key) const
{
  store_citer it = _store.find (key);

  return (_store.end () != it ? it->second : NULL);
}

void
dictionary::print (FILE* fp) const
{
  store_citer it;

  if (1 >= _store.size ())
    {
      it = _store.begin ();
      fprintf (fp, "<< /%s ", it->first);
      it->second->print (fp);
      fprintf (fp, " >>");
      return;
    }

  fprintf (fp, "<<\n");
  for (it = _store.begin (); _store.end () != it; ++it)
    {
      fprintf (fp, "/%s ", it->first);
      it->second->print (fp);
      fprintf (fp, "\n");
    }
  fprintf (fp, ">>");
}

}       // namespace pdf
}       // namespace iscan
