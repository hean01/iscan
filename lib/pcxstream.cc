//  pcxstream.cc -- image streams producing PCX files
//  Copyright (C) 2011  SEIKO EPSON CORPORATION
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

#include "pcxstream.hh"

#include <ios>
#include <cstring>

namespace iscan
{
  pcxstream::pcxstream (FILE *fp, const string& name)
    : _stream (fp),
      _header (false),
      _footer (false),
      _bytesperline (0),
      _row_buf (NULL),
      _zbuf (NULL)
  {
    if (!_stream) throw std::invalid_argument ("invalid file handle");

    memset (_pcx_header, 0, sizeof (_pcx_header));
    _pcx_header [0] = 0x0a;     // Manufacturer
    _pcx_header [1] = 0x05;     // Version
    _pcx_header [2] = 1;        // Encoding
  }

  pcxstream::~pcxstream (void)
  {
    fflush (_stream);
  }

  basic_imgstream&
  pcxstream::flush (void)
  {
    if (!_footer)
      {
        if (mono == _cspc || grey == _cspc)
          {
            write_palette ();
          }
        delete [] _row_buf;
        delete [] _zbuf;
        _footer = true;
      }

    fflush (_stream);
    return *this;
  }

  void
  pcxstream::write_palette (void)
  {
    const size_type palette_size = 256 * 3 + 1;
    byte_type palette [palette_size];
    byte_type *ptr = palette;
    *ptr = 0x0c;                // set palette identifier
    ++ptr;

    for (int i=0; 256>i; ++i)
     {
       *ptr = *(ptr+1) = *(ptr+2) = i;
       ptr += 3;
     }
    size_t rv = fwrite (palette, sizeof (*palette), palette_size, _stream);
    if (palette_size != rv)
      {
        throw std::ios_base::failure ("write error");
      }
  }

  basic_imgstream&
  pcxstream::write (const byte_type *line, size_type n)
  {
    if (!line || 0 == n) return *this;
    if (!_header)
      {
        size_type sz = 0;
        write_header ();
        if (mono == _cspc)
          {
            sz = n * 8;
            pwrite = &pcxstream::write_mono;
          }
        else if (grey == _cspc)
          {
            sz = 1;             // dummy
            pwrite = &pcxstream::write_gray;
          }
        else if (RGB  == _cspc)
          {
            sz = _h_sz * 3;     // r + g + b
            pwrite = &pcxstream::write_color;
          }
        _row_buf = new byte_type [sz];
        _zbuf = new byte_type [_h_sz * 2 + 1]; // assume worst case
      }

    (this->*pwrite) (line, n);

    return *this;
  }

  void
  pcxstream::write_color (const byte_type *line, size_type n)
  {
    byte_type *r = _row_buf;
    byte_type *g = r + _h_sz;
    byte_type *b = g + _h_sz;

    for (size_type i=0; _h_sz>i; ++i)
      {
        r [i] = *line; ++line;
        g [i] = *line; ++line;
        b [i] = *line; ++line;
      }
    write_row (r, _h_sz);
    write_row (g, _h_sz);
    write_row (b, _h_sz);
  }

  void
  pcxstream::write_gray (const byte_type *line, size_type n)
  {
    write_row (line, n);
  }

  void
  pcxstream::write_mono (const byte_type *line, size_type n)
  {
    for (size_type i=0; n>i; ++i)
      {
        for (size_type j=0; 8>j; ++j)
          {
            _row_buf [i*8+7-j] = (line[i] & (0x1 << j)) ? 0x0 : 0xff;
          }
      }
    write_row (_row_buf, _h_sz);
  }

  void
  pcxstream::write_row (const byte_type *line, size_type n)
  {
    size_type zlen = compress_row (line, n, _zbuf);

    // 2 byte align
    if (n < _bytesperline)
      {
        _zbuf [zlen] = 0x00;     // add padding
        ++zlen;
      }

    write_bytes (_zbuf, zlen);
  }

  void
  pcxstream::write_bytes (const byte_type *bytes, size_type n)
  {
    size_type rv;
    rv = fwrite (bytes, sizeof (*bytes), n, _stream);
    if (n != rv)
      {
        throw std::ios_base::failure ("write error");
      }
  }

  basic_imgstream::size_type
  pcxstream::compress_row (const byte_type *line,
                           const size_type n,
                           byte_type *compressed)
  {
    byte_type cnt = 0;
    byte_type *ptr = compressed;
    byte_type saved = 0;

    for (size_type i=0; i<n; ++i)
      {
        if (0 == cnt)
          {
            saved = line[i];
            cnt = 1;
          }
        else if (63 > cnt && saved == line[i])
          {
            ++cnt;
          }
        else if (63 == cnt || saved != line[i])
          {
            if (1 < cnt || 0xc0 == (saved & 0xc0))
              {
                *ptr = 0xc0 | cnt;
                ++ptr;
              }
            *ptr = saved;
            ++ptr;
            saved = line[i];
            cnt = 1;
          }
      }
    // put the rest
    if (1 < cnt || 0xc0 == (saved & 0xc0))
      {
        *ptr = 0xc0 | cnt;
        ++ptr;
      }
    *ptr = saved;
    ++ptr;

    return (ptr - compressed);
  }

  bool
  pcxstream::is_usable (void)
  {
    return true;
  }

  basic_imgstream&
  pcxstream::write_header (void)
  {
    check_consistency ();

    // Even monochrome image, BitsPerPixel in PCX header is set to 8
    // in accordance with the implementation of GIMP.
    size_type bpp = 8;
    _pcx_header [3] = bpp;      // BitsPerPixel

    set_value_le ((_h_sz-1), &_pcx_header [8],  2); // Window.Xmax
    set_value_le ((_v_sz-1), &_pcx_header [10], 2); // Window.Ymax
    set_value_le (_hres,     &_pcx_header [12], 2); // HDpi
    set_value_le (_vres,     &_pcx_header [14], 2); // VDpi

    size_type nplanes = 1;
    size_type paletteinfo = 2;
    if (RGB  == _cspc)
      {
        nplanes = 3;
        paletteinfo = 1;
      }
    _pcx_header [65] = nplanes;                       // NPlanes
    set_value_le (paletteinfo, &_pcx_header [68], 2); // PaletteInfo

    _bytesperline = (_h_sz * bpp + 7) / 8;
    _bytesperline += (_bytesperline & 0x1); // 2 byte align
    set_value_le (_bytesperline, &_pcx_header [66], 2); // BytesPerLine

    write_bytes (_pcx_header, sizeof (_pcx_header));

    _header = true;
    return *this;
  }

  void
  pcxstream::check_consistency (void) const
  {
    if (!(mono == _cspc || grey == _cspc || RGB == _cspc))
      {
        throw std::logic_error ("unsupported colour space");
      }
    if (!(_bits == 1 || _bits == 8))
      {
        throw std::logic_error ("unsupported bit depth");
      }
    size_type max = (1 << 16) - 1;
    if (_h_sz > max || _v_sz > max)
      {
        throw std::logic_error ("maximum image size exceeded");
      }
  }

  // set value to array with little endian
  void
  pcxstream::set_value_le (const size_type value,
                           byte_type *array,
                           const size_type array_size)
  {
    if (!array || 0 >= array_size)
      {
        throw std::invalid_argument ("invalid argment");
      }

    size_type v = value;

    for (size_type i=0; array_size>i; ++i)
      {
        array[i] = v & 0xff;
        v >>= 8;
      }
  }

} // namespace iscan
