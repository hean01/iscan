//  jpegstream.hh -- image streams producing JPEG files
//  Copyright (C) 2008, 2009  SEIKO EPSON CORPORATION
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
//  code used other then esmod.


#ifndef iscan_jpegstream_hh_included
#define iscan_jpegstream_hh_included

#ifndef __cplusplus
#error "This is a C++ header file; use a C++ compiler to compile it."
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "basic-imgstream.hh"

#include <cstdio>
#include <string>

#if HAVE_JPEGLIB_H
#include <jpeglib.h>
#endif

namespace iscan
{
  using std::string;

  class jpegstream : public basic_imgstream
  {
  public:
    typedef basic_imgstream::byte_type byte_type;
    typedef basic_imgstream::size_type size_type;

    explicit jpegstream (FILE *fp, const string& pathname = string ());
    virtual ~jpegstream (void);

    virtual basic_imgstream& write (const byte_type *line, size_type n);

    static bool is_usable (void);

  private:
    basic_imgstream& write_header (void);
    void check_consistency (void) const;

    void init (void);

    FILE *_stream;
    bool  _header;

    byte_type *_scanline;

    static bool validate (lt_dlhandle h);
    struct jpeg_lib_handle
    {
      bool        is_usable;
      string      message;
      lt_dlhandle lib;

#if HAVE_JPEGLIB_H
# ifndef jpeg_create_compress
      fundecl (void, create_compress, jpeg_compress_struct *);
# else
      fundecl (void, CreateCompress, jpeg_compress_struct *, int, size_t);
# endif
      fundecl (void, finish_compress, jpeg_compress_struct *);
      fundecl (void, destroy_compress, jpeg_compress_struct *);
      fundecl (void, destroy, jpeg_common_struct *);

      fundecl (void, stdio_dest, jpeg_compress_struct *, FILE *);
      fundecl (struct jpeg_error_mgr *, std_error, jpeg_error_mgr *);

      fundecl (void, write_scanlines, jpeg_compress_struct *, JSAMPLE **, int);
      fundecl (void, set_defaults, jpeg_compress_struct *);
      fundecl (void, start_compress, jpeg_compress_struct *, bool);

      // only used for version detection purposes; available since libjpeg 7.0
      fundecl (void, default_qtables, jpeg_compress_struct *, bool);
#endif /* HAVE_JPEGLIB_H */
    };
    static jpeg_lib_handle *lib;

#if HAVE_JPEGLIB_H
    static void error_exit (j_common_ptr info);

    struct jpeg_compress_struct _info;
    struct jpeg_error_mgr       _err;
#endif
  };

} // namespace iscan

#endif  /* !defined (iscan_jpegstream_hh_included) */
