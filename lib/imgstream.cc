//  imgstream.cc -- provides an interface to write multiple images
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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "imgstream.hh"

#include "jpegstream.hh"
#include "pcxstream.hh"
#include "pngstream.hh"
#include "pnmstream.hh"
#include "pdfstream.hh"
#include "tiffstream.hh"

namespace iscan
{
  imgstream::imgstream (file_opener& opener, file_format format,
                        bool match_direction)
    : _page (0), _match_direction (match_direction),
      _opener (&opener), _format (format), _configured (false)
  {
    _stream = create_stream ();
  }

  imgstream::imgstream (void)
    : _page (0), _match_direction (false), _opener (NULL), _format (NO_FORMAT),
      _stream (NULL), _configured (false)
  {
  }

  imgstream::~imgstream (void)
  {
    delete _stream;
  }

  imgstream&
  imgstream::write (const byte_type *data, size_type n)
  {
    if (!_stream) return *this;

    if (!_configured)
      {
        _stream->size (_h_sz, _v_sz);
        _stream->resolution (_hres, _vres);
        _stream->colour (_cspc);
        _stream->depth (_bits);
        _configured = true;
      }

    _stream->write (data, n);

    return *this;
  }

  imgstream&
  imgstream::flush (void)
  {
    if (_stream) _stream->flush ();
    return *this;
  }

  void
  imgstream::next (void)
  {
    if (!_configured) return;

    delete _stream;
    _configured = false;

    ++_page;
    ++(*_opener);
    _stream = create_stream ();
    if (_match_direction) _stream->rotate_180 (is_back (_page));
  }

  bool
  imgstream::is_back (unsigned long page)
  {
    return 0 == (page+1)%2;
  }

  bool
  imgstream::is_usable (void)
  {
    return true;
  }

  basic_imgstream *
  imgstream::create_stream (void)
  {
    if (PCX == _format) return new pcxstream (*_opener);
    if (PNM == _format) return new pnmstream (*_opener);
    if (PNG == _format) return new pngstream (*_opener);
    if (JPG == _format) return new jpegstream (*_opener);
    if (PDF == _format) return new pdfstream (*_opener);
    if (TIF == _format) return new tiffstream (*_opener, _opener->temp ());

    throw std::invalid_argument ("unsupported file format");
  }

  imgstream *
  create_imgstream (file_opener& opener, file_format format,
                    bool match_direction)
  {
    if (opener.is_collating ())
      {
        if (PDF == format) return new pdfstream (opener, match_direction);
        if (TIF == format) return new tiffstream (opener, opener.name ());
      }
    
    return new imgstream (opener, format, match_direction);
  }

}       // namespace iscan
