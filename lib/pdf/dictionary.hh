//  dictionary.hh -- PDF dictionaries
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


#ifndef iscan_pdf_dictionary_hh_included
#define iscan_pdf_dictionary_hh_included

#ifndef __cplusplus
#error "This is a C++ header file; use a C++ compiler to compile it."
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "object.hh"
#include "primitive.hh"

#include <map>

namespace iscan
{

namespace pdf
{

/*! Defines a pdf dictionary object [p 59]
 */
class dictionary : public object
{
private:
  typedef std::map<const char *, object *> store_type;
  typedef store_type::iterator             store_iter;
  typedef store_type::const_iterator       store_citer;

  store_type _store;
  store_type _mine;

public:
  virtual ~dictionary ();

  /*! Insert a key/value pair into the dictionary
   *
   *  If the key already exists, its value is replaced with the new one.
   *
   *  The key is written to the PDF file as a name object as defined in the
   *  PDF spec [p. 59]
   */
  void insert (const char *key, object *value);

  void insert (const char *key, primitive value);
  void insert (const char *key, object value);

  /*! Count the number of objects in the dictionary
   */
  size_t size () const;

  /*! Obtain a reference to an object with a given key
   */
  const object * operator[] (const char *key) const;

  virtual void print (FILE* fp) const;
};

}       // namespace pdf
}       // namespace iscan

#endif  // iscan_pdf_dictionary_hh_included
