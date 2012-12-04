//  object.hh -- PDF objects
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

#ifndef iscan_pdf_object_hh_included
#define iscan_pdf_object_hh_included

#ifndef __cplusplus
#error "This is a C++ header file; use a C++ compiler to compile it."
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cstdlib>
#include <cstdio>

namespace iscan
{

namespace pdf
{

using namespace std;

/*! A base class for all pdf objects [p 51].
 *
 * A pdf::object is also used to pass around object numbers in a transparent
 * fashion so that object references can be output correctly as elements of
 * arrays and dictionaries.
 */
class object
{
private:

  size_t _obj_num;
  static size_t next_obj_num; // the next free object number

public:

  object ();

  /*! Creates a new pdf::object with its object number set to \a num.
   *
   *  Constructs an indirect object.
   */
  object (size_t num);

  virtual ~object (void);

  /*! Obtain the pdf::object's object number.
   *
   * If the object has not been allocated an object number yet, a new one is
   * allocated and returned.
   */
  size_t obj_num ();

  /*! Determine whether the object is direct or indirect [p 63].
   *
   *  Probably don't need this method.
   *
   *  @return True if the object is direct, False if it is indirect
   */
  bool is_direct () const;

  /*! Output the object contents.
   *
   *  In the case of pdf::object, this only outputs an indirect reference to
   *  itself [p 64].
   *
   *  Each subclass re-implements this in order to print its own content.
   *  It should only ever output the object contents, ommitting the object
   *  definition header and footer [p 64]
   */
  virtual void print (FILE* fp) const;

  /*! Compare the contents of two pdf::objects.
   *
   * Only the object contents are compared, the object number of the two
   * objects can be different.
   *
   * In the case of pdf::object, the object numbers are compared.
   */
  virtual bool operator== (object& that) const;
  
  /*! Reset the current object number to recycle them for new documents
   */
  static void reset_object_numbers ();

};

}       // namespace pdf
}       // namespace iscan

#endif  // iscan_pdf_object_hh_included
