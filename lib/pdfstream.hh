//  pdfstream.hh -- image streams producing PDF files
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

#ifndef iscan_pdfstream_hh_included
#define iscan_pdfstream_hh_included

#ifndef __cplusplus
#error "This is a C++ header file; use a C++ compiler to compile it."
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "imgstream.hh"

#include "pdf/writer.hh"
#include "pdf/dictionary.hh"
#include "pdf/array.hh"
#include "pdf/primitive.hh"
#include "fax-encoder.hh"

#include <string>

namespace iscan
{
  using std::string;

class pdfstream : public imgstream
{

public:
  typedef basic_imgstream::byte_type byte_type;
  typedef basic_imgstream::size_type size_type;

private:
  bool _need_page_trailer;
  size_type _data_size;
  size_type _pdf_h_sz; // scaled size to fit on a page at 72 dpi
  size_type _pdf_v_sz; // but better to set the dpi in the PDF file,
                       // there is some way to do that, I read it in the spec!

  pdf::writer *_doc;
  pdf::dictionary *_pages;
  pdf::array *_page_list;
  pdf::dictionary *_trailer;

  size_type _row;
  pdf::primitive *_img_height_obj;

  basic_imgstream *_stream;
  FILE *_file;

  bool _do_jpeg;
  fax_encoder *_g3;
  bool _rotate_180;

public:
  static bool is_usable ();

  explicit pdfstream (FILE *fp, bool match_direction = false);

  virtual ~pdfstream ();

  virtual imgstream& write (const byte_type *line, size_type n);

  virtual void next ();
  virtual void rotate_180 (bool yes);

private:
  void init();
  void write_header ();
  void write_page_header ();
  void write_page_trailer ();
  void write_image_object (pdf::dictionary& image, std::string name);
};

}       // namespace iscan

#endif  /* iscan_pdfstream_hh_included */
