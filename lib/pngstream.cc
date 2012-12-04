//  pngstream.cc -- image streams producing PNG files
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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pngstream.hh"

#include <iostream>

namespace iscan
{
  pngstream::pngstream (FILE *fp, const string& name)
    : _stream (fp), _header (false), _footer (false)
  {
    if (!_stream) throw std::invalid_argument ("invalid file handle");
#if HAVE_PNG_H
    init ();
#endif
  }

  pngstream::~pngstream (void)
  {
    try
      {
        flush ();
      }
    catch (const std::exception& oops)
      {
        std::cerr << oops.what ();
      }

#if HAVE_PNG_H
    lib->destroy_write_struct (&_png, &_info);
#endif
  }

  basic_imgstream&
  pngstream::write (const byte_type *line, size_type n)
  {
    if (!line || 0 == n) return *this;
#if HAVE_PNG_H
    if (!_header)
      {
        write_header ();
      }
    set_error_handler (_png, _info);
    lib->write_row (_png, (png_byte *) line);
#endif
    return *this;
  }

  basic_imgstream&
  pngstream::flush (void)
  {
#if HAVE_PNG_H
    set_error_handler (_png, _info);

    if (_header && !_footer && _png->num_rows == _png->flush_rows)
      {
        lib->write_end (_png, _info);
        _footer = true;
      }
#endif
    fflush (_stream);

    return *this;
  }

  bool
  pngstream::is_usable (void)
  {
    if (lib)
      {
        return lib->is_usable;
      }

    lib = new (std::nothrow) png_lib_handle ();
    if (!lib)
      {
        return false;
      }

    lib->is_usable = false;
    lib->message   = string ();
    lib->lib       = NULL;
#if HAVE_PNG_H
    try
      {
        // Some PNG libraries are *not* linked with libz which leads
        // to a "file not found" error when calling dlopen.  We just
        // dlopen the libz library explicitly to work around this.
        basic_imgstream::dlopen ("libz");
        basic_imgstream::dlopen ("libpng", validate);
      }
    catch (std::runtime_error& e)
      {
        try
          {
            // At least Debian has released packages with a different
            // name for the library.  Try this as a fallback to avoid
            // the need for a development package that has libpng.so.
            basic_imgstream::dlopen ("libpng12", validate);
          }
        catch (std::runtime_error& e)
          {
            lib->message = e.what ();
            return lib->is_usable;
          }
      }
#endif /* HAVE_PNG_H */

    return lib->is_usable;
  }

#if HAVE_PNG_H
#define funcsym(name)                                   \
  lib->name                                             \
    = ((pngstream::png_lib_handle::name##_f)            \
       basic_imgstream::dlsym (lib->lib, "png_"#name));
#endif

  bool
  pngstream::validate (lt_dlhandle h)
  {
    if (!h) return false;

#if HAVE_PNG_H
    lib->lib = h;

    funcsym (access_version_number);
    funcsym (create_write_struct);
    funcsym (create_info_struct);
    funcsym (destroy_write_struct);
    funcsym (init_io);
    funcsym (set_IHDR);
    funcsym (set_pHYs);
    funcsym (set_invert_mono);
    funcsym (write_info);
    funcsym (write_row);
    funcsym (write_flush);
    funcsym (write_end);

    if (lib->access_version_number
        && lib->create_write_struct
        && lib->create_info_struct
        && lib->destroy_write_struct
        && lib->init_io
        && lib->set_IHDR
        && lib->set_pHYs
        && lib->set_invert_mono
        && lib->write_info
        && lib->write_row
        && lib->write_flush
        && lib->write_end)
      {
        lib->is_usable = (PNG_LIBPNG_VER <= lib->access_version_number ());
      }
#endif /* HAVE_PNG_H */

    return lib->is_usable;
  }

#if HAVE_PNG_H
#undef funcsym
#endif

  basic_imgstream&
  pngstream::write_header (void)
  {
    check_consistency ();

#if HAVE_PNG_H
    set_error_handler (_png, _info);

    if (mono == _cspc)
      {
        _bits = 1;
        lib->set_invert_mono (_png);
      }

    lib->init_io (_png, _stream);

    lib->set_IHDR (_png, _info, _h_sz, _v_sz, _bits,
                   (RGB == _cspc
                    ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_GRAY),
                   PNG_INTERLACE_NONE,
                   PNG_COMPRESSION_TYPE_DEFAULT,
                   PNG_FILTER_TYPE_DEFAULT);

    size_type hres = size_type (100 / 2.54 * _hres + 0.5);
    size_type vres = size_type (100 / 2.54 * _vres + 0.5);
    lib->set_pHYs (_png, _info, hres, vres, PNG_RESOLUTION_METER);

    lib->write_info (_png, _info);
#endif /* HAVE_PNG_H */

    _header = true;
    return *this;
  }

  void
  pngstream::check_consistency (void) const
  {
    if (!(mono == _cspc || grey == _cspc || RGB == _cspc))
      {
        throw std::logic_error ("unsupported colour space");
      }
  }

  void
  pngstream::init (void)
  {
    if (!is_usable ())
      {
        throw std::runtime_error (lib->message);
      }

#if HAVE_PNG_H
    _png  = lib->create_write_struct (version_string,
                                      NULL, NULL, NULL);
    _info = NULL;
    if (!_png) throw std::bad_alloc ();

    _info = lib->create_info_struct (_png);
    if (!_info)
      {
        lib->destroy_write_struct (&_png, NULL);
        throw std::bad_alloc ();
      }
#endif /* HAVE_PNG_H */
  }

  pngstream::png_lib_handle *pngstream::lib = NULL;

#if HAVE_PNG_H
  png_const_charp pngstream::version_string = PNG_LIBPNG_VER_STRING;
#endif

} // namespace iscan
