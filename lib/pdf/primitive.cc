//  primitive.cc -- PDF primitives
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

#include "primitive.hh"

#include <sstream>

namespace iscan
{

namespace pdf
{

primitive::primitive ()
  : _str (string ())
{
}

primitive::primitive (string value)
  : _str (value)
{
}

primitive::primitive (int value)
{
  stringstream ss;
  ss << value;
  ss >> _str;
}

primitive::primitive (size_t value)
{
  stringstream ss;
  ss << value;
  ss >> _str;
}

primitive::primitive (double value)
{
  stringstream ss;
  ss << value;
  ss >> _str;
}

primitive::~primitive (void)
{
}

bool
primitive::operator== (const primitive& that) const
{
  return _str == that._str;
}

void
primitive::print (FILE* fp) const
{
  fprintf (fp, "%s", _str.c_str ());
}

// FIXME: doesn't do the default assignment just what we want?
void
primitive::operator= (const primitive& that)
{
  _str = that._str;
}


}       // namespace pdf
}       // namespace iscan
