//  object.cc -- PDF objects
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

#include "object.hh"

#include <stdexcept>

namespace iscan
{

namespace pdf
{


object::object ()
{
  _obj_num = 0;
}

object::object (size_t num)
{
  // FIXME: what if num has already been used?
  _obj_num = num;
}

object::~object (void)
{
}

size_t
object::obj_num ()
{
  if (65535 == next_obj_num)
    {
      throw std::runtime_error ("PDF object number overflow");
    }

  if (is_direct ())
    {
      _obj_num = ++next_obj_num;
    }
  return _obj_num;
}

bool
object::operator== (object& that) const
{
  // FIXME: what if one or both instances have not gotten an object
  //        number yet?
  return _obj_num == that._obj_num;
}

void
object::print (FILE* fp) const
{
  fprintf (fp, "%zu 0 R", _obj_num);
}

bool
object::is_direct () const
{
  return (0 == _obj_num);
}

size_t object::next_obj_num = 0;

void object::reset_object_numbers ()
{
  next_obj_num = 0;
}

}       // namespace pdf
}       // namespace iscan
