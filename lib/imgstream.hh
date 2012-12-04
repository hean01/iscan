//  imgstream.hh -- provides an interface to write multiple images
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

#ifndef iscan_imgstream_hh_included
#define iscan_imgstream_hh_included

#ifndef __cplusplus
#error "This is a C++ header file; use a C++ compiler to compile it."
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "basic-imgstream.hh"
#include "file-opener.hh"


namespace iscan
{
  enum file_format
    {
      PCX,
      PNM,
      PNG,
      JPG, JPEG = JPG,
      PDF,
      TIF,                      // libtiff uses TIFF as a type already!
      NO_FORMAT,
    };

  class imgstream : public basic_imgstream
  {
  public:
    typedef basic_imgstream::byte_type byte_type;
    typedef basic_imgstream::size_type size_type;

    imgstream (file_opener& opener, file_format format,
               bool match_direction = false);
    virtual ~imgstream (void);

    virtual imgstream& write (const byte_type *data, size_type n);
    virtual imgstream& flush (void);

    virtual void next (void);

    static bool is_usable (void);

  protected:
    imgstream (void);

    bool is_back (unsigned long); // indicates whether a page is the
                                  // back page of a duplex scan

    unsigned long _page;
    bool _match_direction; // when true, match front and back
                           // orientation for duplex scans

  private:
    basic_imgstream * create_stream (void);

    file_opener* _opener;
    file_format  _format;

    basic_imgstream *_stream;
    bool _configured;
  };

  imgstream *
  create_imgstream (file_opener& opener, file_format format,
                    bool match_direction = false);

} // namespace iscan

#endif /* iscan_imgstream_hh_included */
