//  pngstream.hh -- image streams producing PNG files
//  Copyright (C) 2008, 2009  SEIKO EPSON CORPORATION
//
//  This file is part of 'iscan' program.
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
//  code used other then esmod.


#ifndef iscan_pngstream_hh_included
#define iscan_pngstream_hh_included

#ifndef __cplusplus
#error "This is a C++ header file; use a C++ compiler to compile it."
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "basic-imgstream.hh"

#include <cstdio>
#include <string>
#include <ios>

#if HAVE_PNG_H
#include <png.h>
#endif

namespace iscan
{
  using std::string;

  class pngstream : public basic_imgstream
  {
  public:
    typedef basic_imgstream::byte_type byte_type;
    typedef basic_imgstream::size_type size_type;

    explicit pngstream (FILE *fp, const string& name = string ());
    virtual ~pngstream (void);

    virtual basic_imgstream& write (const byte_type *line, size_type n);
    virtual basic_imgstream& flush (void);

    static bool is_usable (void);

  private:
    basic_imgstream& write_header (void);
    void check_consistency (void) const;

    void init (void);

    FILE *_stream;
    bool  _header;
    bool  _footer;

    static bool validate (lt_dlhandle h);
    struct png_lib_handle
    {
      bool        is_usable;
      string      message;
      lt_dlhandle lib;

#if HAVE_PNG_H
      fundecl (png_uint_32, access_version_number,
               void);
      fundecl (png_structp, create_write_struct,
               png_const_charp, png_voidp, png_error_ptr, png_error_ptr);
      fundecl (png_infop  , create_info_struct ,
               png_structp);
      fundecl (void, destroy_write_struct,
               png_structpp, png_infopp);
      fundecl (void, init_io,
               png_structp, FILE *);
      fundecl (void, set_IHDR,
               png_structp, png_infop, png_uint_32, png_uint_32, int,
               int, int, int, int);
      fundecl (void, set_pHYs,
               png_structp, png_infop, png_uint_32, png_uint_32, int);
      fundecl (void, set_invert_mono, png_structp);
      fundecl (void, write_info,
               png_structp, png_infop);
      fundecl (void, write_row,
               png_structp, png_bytep);
      fundecl (void, write_flush,
               png_structp);
      fundecl (void, write_end,
               png_structp, png_infop);
#endif /* HAVE_PNG_H */
    };
    static png_lib_handle *lib;

#if HAVE_PNG_H
    friend void set_error_handler (png_structp, png_infop);

    png_structp _png;
    png_infop   _info;

    static png_const_charp version_string;
#endif
  };


//    implementation

// This function needs to be expanded verbatim at the location it's
// invoked for the setjmp call to work as intended.
#if HAVE_PNG_H
#define set_error_handler(png, info)                            \
  {                                                             \
    if (!png || !info || setjmp (png_jmpbuf (png)))             \
      {                                                         \
        pngstream::lib->destroy_write_struct (&png, &info);     \
        png = NULL;                                             \
        info = NULL;                                            \
        throw std::ios_base::failure ("write error");           \
      }                                                         \
  }
#endif

} // namespace iscan

#endif /* !defined (iscan_pngstream_hh_included) */
