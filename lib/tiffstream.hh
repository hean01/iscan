//  tiffstream.hh -- image streams producing TIFF files
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
//  code used other than esmod.


#ifndef iscan_tiffstream_hh_included
#define iscan_tiffstream_hh_included

#ifndef __cplusplus
#error "This is a C++ header file; use a C++ compiler to compile it."
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "imgstream.hh"

#if HAVE_TIFFIO_H
#include <tiffio.h>
#include <cstdarg>              // for error and warning handlers
#endif

namespace iscan
{
  using std::string;

  class tiffstream : public imgstream
  {
  public:
    typedef basic_imgstream::byte_type byte_type;
    typedef basic_imgstream::size_type size_type;

    tiffstream (FILE *fp, const string& name);
    virtual ~tiffstream (void);

    virtual imgstream& write (const byte_type *line, size_type n);

    virtual void next (void);

    static bool is_usable (void);

  private:
    void set_tags (void);
    void check_consistency (void) const;

    void init (const string& name);

    FILE *_stream;

    static bool validate (lt_dlhandle h);
    struct tiff_lib_handle
    {
      bool        is_usable;
      string      message;
      lt_dlhandle lib;

#if HAVE_TIFFIO_H
      fundecl (TIFF *, Open, const char *, const char *);
      fundecl (TIFF *, FdOpen, const int, const char *, const char *);
      fundecl (void, Close, TIFF *);
      fundecl (int, WriteDirectory, TIFF *);
      fundecl (int, WriteScanline, TIFF *, tdata_t, uint32, tsample_t);
      fundecl (int, Flush, TIFF *);
      fundecl (int, SetField, TIFF *, ttag_t, ...);
      fundecl (TIFFErrorHandler, SetErrorHandler, TIFFErrorHandler);
      fundecl (TIFFErrorHandler, SetWarningHandler, TIFFErrorHandler);
#endif /* HAVE_TIFFIO_H */
    };

    static tiff_lib_handle *lib;

#if HAVE_TIFFIO_H
    TIFF   *_tiff;
    uint32  _row;
#endif
  };

} // namespace iscan

#endif // iscan_tiffstream_hh_included
