//  tiffstream.cc -- image streams producing TIFF files
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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tiffstream.hh"

#include <cstdlib>
#include <ios>
#include <stdexcept>

namespace iscan
{
  // Forward declaration of handlers and support functions.
  static void handle_error (const char *module, const char *fmt, va_list ap);
  static void handle_warning (const char *module, const char *fmt, va_list ap);


  tiffstream::tiffstream (FILE *fp, const string& name)
    : _stream (fp)
  {
    if (!_stream) throw std::invalid_argument ("invalid file handle");
    init (name);                // handles HAVE_TIFFIO_H only stuff
  }

  tiffstream::~tiffstream (void)
  {
#if HAVE_TIFFIO_H
    lib->Close (_tiff);
#endif
    fflush (_stream);
  }

  imgstream&
  tiffstream::write (const byte_type *line, size_type n)
  {
    if (!line || 0 == n) return *this;

    if (0 == _page)
      {
        set_tags ();
      }

#if HAVE_TIFFIO_H
    if (1 != lib->WriteScanline (_tiff, const_cast<char *> (line), _row, 1))
      {
        throw std::ios_base::failure ("failure writing TIFF scanline");
      }
    ++_row;
#endif /* HAVE_TIFFIO_H */
    return *this;
  }

  void
  tiffstream::next (void)
  {
#if HAVE_TIFFIO_H
    if (0 == _row) return;
#endif

    if (0 < _page)
      {
#if HAVE_TIFFIO_H
        if (1 != lib->WriteDirectory (_tiff))
          {
            throw std::runtime_error ("failure writing TIFF directory");
          }
#endif
      }
    set_tags ();
  }

  bool
  tiffstream::is_usable (void)
  {
    if (lib)
      {
        return lib->is_usable;
      }

    lib = new (std::nothrow) tiff_lib_handle ();
    if (!lib)
      {
        return false;
      }

    lib->is_usable = false;
    lib->message   = string ();
    lib->lib       = NULL;
#if HAVE_TIFFIO_H
    try
      {
        imgstream::dlopen ("libtiff", validate);
      }
    catch (std::runtime_error& e)
      {
        lib->message = e.what ();
        return lib->is_usable;
      }
#endif /* HAVE_TIFFIO_H */

    return lib->is_usable;
  }

#if HAVE_TIFFIO_H
#define funcsym(name)                                           \
  lib->name = ((tiffstream::tiff_lib_handle::name##_f)          \
               imgstream::dlsym (lib->lib, "TIFF"#name));
#endif

  bool
  tiffstream::validate (lt_dlhandle h)
  {
    if (!h) return false;

#if HAVE_TIFFIO_H
    lib->lib = h;

    funcsym (Open);
    funcsym (FdOpen);
    funcsym (Close);
    funcsym (WriteDirectory);
    funcsym (WriteScanline);
    funcsym (Flush);
    funcsym (SetField);
    funcsym (SetErrorHandler);
    funcsym (SetWarningHandler);

    lib->is_usable = (lib->Open
                      && lib->FdOpen
                      && lib->Close
                      && lib->WriteDirectory
                      && lib->WriteScanline
                      && lib->Flush
                      && lib->SetField
                      && lib->SetErrorHandler
                      && lib->SetWarningHandler);

    if (lib->is_usable)
      {
        lib->SetErrorHandler (handle_error);
        lib->SetWarningHandler (handle_warning);
      }
#endif /* HAVE_TIFFIO_H */

    return lib->is_usable;
  }

#if HAVE_TIFFIO_H
#undef funcsym
#endif

  void
  tiffstream::set_tags (void)
  {
    check_consistency ();

#if HAVE_TIFFIO_H
    lib->SetField (_tiff, TIFFTAG_SAMPLESPERPIXEL, (RGB == _cspc ? 3 : 1));

    uint16 pm;
    if (mono == _cspc) pm = PHOTOMETRIC_MINISWHITE;
    if (grey == _cspc) pm = PHOTOMETRIC_MINISBLACK;
    if (RGB  == _cspc) pm = PHOTOMETRIC_RGB;
    lib->SetField (_tiff, TIFFTAG_PHOTOMETRIC, pm);

    if (RGB == _cspc)
      lib->SetField (_tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

    lib->SetField (_tiff, TIFFTAG_BITSPERSAMPLE, _bits);

    lib->SetField (_tiff, TIFFTAG_IMAGEWIDTH , _h_sz);
    lib->SetField (_tiff, TIFFTAG_IMAGELENGTH, _v_sz);
    lib->SetField (_tiff, TIFFTAG_ROWSPERSTRIP, 1);

    if (0 != _hres && 0 != _vres)
      {
        lib->SetField (_tiff, TIFFTAG_XRESOLUTION, float (_hres));
        lib->SetField (_tiff, TIFFTAG_YRESOLUTION, float (_vres));
        lib->SetField (_tiff, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
      }

    lib->SetField (_tiff, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

    _row = 0;
#endif /* HAVE_TIFFIO_H */
    ++_page;

    return;
  }

  void
  tiffstream::check_consistency (void) const
  {
    if (!(mono == _cspc || grey == _cspc || RGB == _cspc))
      {
        throw std::logic_error ("unsupported colour space");
      }
    if (!(1 == _bits || 8 == _bits))
      {
        throw std::logic_error ("unsupported bit depth");
      }
  }

  void
  tiffstream::init (const string& name)
  {
    if (!is_usable ())
      {
        throw std::runtime_error (lib->message);
      }

#if HAVE_TIFFIO_H
    _row = 0;
    // libtiff uses 'b' to signal big-endian, not binary as fopen()!
    _tiff = lib->Open (name.c_str (), "w");
    if (!_tiff) throw std::bad_alloc ();
#endif
  }

  tiffstream::tiff_lib_handle *tiffstream::lib = NULL;


  // Definition of handlers and support functions.

  /*! \todo  Implement when debugging framework has been worked out
   */
  static void
  handle_error (const char *module, const char *fmt, va_list ap)
  {
  }

  /*! \todo  Implement when debugging framework has been worked out
   */
  static void
  handle_warning (const char *module, const char *fmt, va_list ap)
  {
  }

} // namespace iscan
