//  jpegstream.cc -- image streams producing JPEG files
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

#include "jpegstream.hh"

#include <cstdlib>
#include <ios>

namespace iscan
{
  jpegstream::jpegstream (FILE *fp, const string& name)
    : _stream (fp), _header (false), _scanline (NULL)
  {
    if (!_stream) throw std::invalid_argument ("invalid file handle");
#if HAVE_JPEGLIB_H
    init ();
#endif
  }

  jpegstream::~jpegstream (void)
  {
    delete [] _scanline;
#if HAVE_JPEGLIB_H
    if (_header)
      {
        if (0 == _v_sz) lib->finish_compress (&_info);
        lib->destroy_compress (&_info);
      }
#endif
    fflush (_stream);
  }

  basic_imgstream&
  jpegstream::write (const byte_type *line, size_type n)
  {
    if (!line || 0 == n) return *this;
#if HAVE_JPEGLIB_H
    if (!_header)
      {
        write_header ();
      }
    if (!_scanline)
      {
        lib->write_scanlines (&_info, (JSAMPLE **) &line, 1);
        if (0 < _info.err->msg_code)
          throw std::ios_base::failure ("write error");
      }
    else
      {
        // FIXME: assumes that _bits == 1, whereas the condition for
        //        _scanline to be true, see write_init (), requires
        //        only that _bits != 8.
        for (unsigned int i = 0; i < _h_sz; ++i)
          {
            div_t index = div (i, 8 * sizeof (JSAMPLE));
            int offset = 8 * sizeof (JSAMPLE) - 1 - index.rem;
            _scanline[i] = ((line[index.quot] & (1 << offset))
                            ? 0 : ~0);
          }
        lib->write_scanlines (&_info, (JSAMPLE **) &_scanline, 1);
        if (0 < _info.err->msg_code)
          throw std::ios_base::failure ("write error");
      }
    --_v_sz;
#endif /* HAVE_JPEGLIB_H */
    return *this;
  }

  bool
  jpegstream::is_usable (void)
  {
    if (lib)
      {
        return lib->is_usable;
      }

    lib = new (std::nothrow) jpeg_lib_handle ();
    if (!lib)
      {
        return false;
      }

    lib->is_usable = false;
    lib->message   = string ();
    lib->lib       = NULL;
#if HAVE_JPEGLIB_H
    try
      {
        basic_imgstream::dlopen ("libjpeg", validate);
      }
    catch (std::runtime_error& e)
      {
        lib->message = e.what ();
        return lib->is_usable;
      }
#endif /* HAVE_JPEGLIB_H */

    return lib->is_usable;
  }

#if HAVE_JPEGLIB_H
#define funcsym(name)                                           \
  lib->name                                                     \
    = ((jpegstream::jpeg_lib_handle::name##_f)                  \
       basic_imgstream::dlsym (lib->lib, "jpeg_"#name));
#endif

  bool
  jpegstream::validate (lt_dlhandle h)
  {
    if (!h) return false;

#if HAVE_JPEGLIB_H
    lib->lib = h;

# ifndef jpeg_create_compress
    funcsym (create_compress);
# else
    funcsym (CreateCompress);
# endif
    funcsym (finish_compress);
    funcsym (destroy_compress);
    funcsym (destroy);
    funcsym (stdio_dest);
    funcsym (std_error);
    funcsym (write_scanlines);
    funcsym (set_defaults);
    funcsym (start_compress);
    funcsym (default_qtables);

    // restrict usage of libjpeg to the version range it was compiled against;
    // either before version 7.0 or 7.0 and later
    bool is_version_consistent =
        ((JPEG_LIB_VERSION <  70 && !lib->default_qtables) ||
         (JPEG_LIB_VERSION >= 70 &&  lib->default_qtables));

    lib->is_usable = (
# ifndef jpeg_create_compress
                      lib->create_compress
# else
                      lib->CreateCompress
# endif
                      && lib->finish_compress
                      && lib->destroy_compress
                      && lib->destroy
                      && lib->stdio_dest
                      && lib->std_error
                      && lib->write_scanlines
                      && lib->set_defaults
                      && lib->start_compress
                      && is_version_consistent);
#endif /* HAVE_JPEGLIB_H */

    return lib->is_usable;
  }

#if HAVE_JPEGLIB_H
#undef funcsym
#endif

  basic_imgstream&
  jpegstream::write_header (void)
  {
    check_consistency ();

#if HAVE_JPEGLIB_H

    _info.image_width  = _h_sz;
    _info.image_height = _v_sz;

    _info.in_color_space   = (RGB == _cspc ? JCS_RGB : JCS_GRAYSCALE);
    _info.input_components = (RGB == _cspc ? 3 : 1);

    lib->set_defaults (&_info);

    size_type density_max = (1 << sizeof (_info.X_density) * 8) - 1;
    _info.density_unit = 1;
    _info.X_density = (_hres <= density_max) ? _hres : density_max;
    _info.Y_density = (_vres <= density_max) ? _vres : density_max;

    lib->start_compress (&_info, true);

    if (mono == _cspc && 8 != _bits)
      {
        _scanline = new byte_type[_h_sz];
      }
    else
      {
        _scanline = NULL;
      }
#endif /* HAVE_JPEGLIB_H */

    _header = true;
    return *this;
  }

  void
  jpegstream::check_consistency (void) const
  {
    if (!(mono == _cspc || grey == _cspc || RGB == _cspc))
      {
        throw std::logic_error ("unsupported colour space");
      }
  }

  void
  jpegstream::init (void)
  {
    if (!is_usable ())
      {
        throw std::runtime_error (lib->message);
      }

#if HAVE_JPEGLIB_H
    // set up JPEG library default error handlers first, then override
    // error handling for fatal errors (because the default would just
    // end up calling exit())
    _info.err = lib->std_error (&_err);
    _err.error_exit = error_exit;
# ifndef jpeg_create_compress
    lib->create_compress (&_info);
# else
    lib->CreateCompress (&_info, JPEG_LIB_VERSION,
                         (size_t) sizeof (struct jpeg_compress_struct));
# endif
    lib->stdio_dest (&_info, _stream);
#endif /* HAVE_JPEGLIB_H */
  }

  jpegstream::jpeg_lib_handle *jpegstream::lib = NULL;

#if HAVE_JPEGLIB_H
  void
  jpegstream::error_exit (jpeg_common_struct *info)
  {
    jpegstream::lib->destroy (info);
  }
#endif

} // namespace iscan
