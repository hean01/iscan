//  primitive.hh -- PDF primitives
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

#ifndef iscan_pdf_primitive_hh_included
#define iscan_pdf_primitive_hh_included

#ifndef __cplusplus
#error "This is a C++ header file; use a C++ compiler to compile it."
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "object.hh"

#include <string>

namespace iscan
{

namespace pdf
{
  using std::string;

/*! Defines a primitive pdf object: one of string, name, integer, or real.
 */
class primitive : public object
{
private:
  string _str;

public:
  primitive ();

  /*! Create a new pdf string/name object with the given value.
   */
  primitive (string value);

  /*! Create a new pdf integer object
   */
  primitive (int value);

  primitive (size_t value);

  /*! Create a new pdf real object
   */
  primitive (double value);

  virtual ~primitive (void);

  /*! Compare the contents of two pdf::primitives.
   *
   * Only the object contents are compared, the object number of the
   * two objects may differ.
   */
  virtual bool operator== (const primitive& other) const;

  void operator= (const primitive& that);

  virtual void print (FILE* fp) const;
};

}       // namespace pdf
}       // namespace iscan

#endif  // iscan_pdf_primitive_hh_included
