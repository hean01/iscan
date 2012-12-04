//  fax-encoder.hh -- convert scanlines to fascimile format
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


#ifndef iscan_fax_encoder_hh_included
#define iscan_fax_encoder_hh_included

#ifndef __cplusplus
#error "This is a C++ header file; use a C++ compiler to compile it."
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "basic-imgstream.hh"

namespace iscan
{
  using std::string;

  class fax_encoder
  {
  public:
    typedef basic_imgstream::byte_type byte_type;
    typedef basic_imgstream::size_type size_type;

    string operator() (const byte_type* line, size_type n);
  };

} // namespace iscan

#endif /* !defined (iscan_fax_encoder_hh_included) */
