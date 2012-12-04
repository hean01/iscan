//  pnmstream.cc -- image streams producing PNM files
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
//  code used other then esmod.


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pnmstream.hh"

#include <ios>

namespace iscan
{
  pnmstream::pnmstream (FILE *fp, const string& name)
    : _stream (fp), _header (false)
  {
    if (!_stream) throw std::invalid_argument ("invalid file handle");
  }

  pnmstream::~pnmstream (void)
  {
    fflush (_stream);
  }

  basic_imgstream&
  pnmstream::write (const byte_type *line, size_type n)
  {
    if (!line || 0 == n) return *this;
    if (!_header)
      {
        write_header ();
      }
    size_t rv = fwrite (line, sizeof (byte_type), n, _stream);
    if (n != rv) throw std::ios_base::failure ("write error");
    return *this;
  }

  bool
  pnmstream::is_usable (void)
  {
    return true;
  }

  basic_imgstream&
  pnmstream::write_header (void)
  {
    check_consistency ();

    string magic;
    if (mono == _cspc) magic = "P4";
    if (grey == _cspc) magic = "P5";
    if (RGB  == _cspc) magic = "P6";

    int rv;
    rv = fprintf (_stream, "%s\n%zd %zd\n", magic.c_str (), _h_sz, _v_sz);
    if (0 > rv) throw std::ios_base::failure ("write error");
    if ("P4" != magic)
      {
        rv = fprintf (_stream, "%d\n", (1 << _bits) - 1);
        if (0 > rv) throw std::ios_base::failure ("write error");
      }
    _header = true;
    return *this;
  }

  void
  pnmstream::check_consistency (void) const
  {
    if (!(mono == _cspc || grey == _cspc || RGB == _cspc))
      {
        throw std::logic_error ("unsupported colour space");
      }
    if (_bits > 16)
      {
        throw std::logic_error ("maximum bit depth exceeded");
      }
    if (grey == _cspc || RGB == _cspc)
      {
        if (8 != _bits && 16 != _bits)
          {
            throw std::invalid_argument ("bit depth/colour space mismatch");
          }
      }
  }

} // namespace iscan
