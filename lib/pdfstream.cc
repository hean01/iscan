//  pdfstream.cc -- image streams producing PDF files
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

#include "pdfstream.hh"
#include "jpegstream.hh"

#include <sstream>

namespace iscan
{

pdfstream::pdfstream (FILE *file, bool match_direction)
  : imgstream (),               // avoid recursion
    _file (file), _g3 (NULL), _rotate_180 (false)
{
  _match_direction = match_direction;
  init ();
}

void
pdfstream::init ()
{
  if (!is_usable ())
    {
      throw std::runtime_error ("pdf not usable");
    }

  _page = 0;
  _row = 0;
  _need_page_trailer = false;
  _data_size = 0;
  _pdf_h_sz = 0;
  _pdf_v_sz = 0;
  _do_jpeg = false;

  _doc = NULL;
  _pages = NULL;
  _page_list = NULL;
  _trailer = NULL;
  _img_height_obj = NULL;
  _stream = NULL;

  _doc = new pdf::writer (_file);

  pdf::object::reset_object_numbers ();
}

pdfstream::~pdfstream ()
{
  if (_need_page_trailer)
    {
      write_page_trailer ();
    }

  fflush (_file);

  delete _doc;
  delete _pages;
  delete _page_list;
  delete _trailer;
  delete _img_height_obj;

  delete _stream;

  delete _g3;
}

bool
pdfstream::is_usable ()
{
  return true;
}

void
pdfstream::rotate_180 (bool yes)
{
  _rotate_180 = yes;
}

void
pdfstream::next ()
{
  if (0 == _row)
    {
      return;
    }

  write_page_trailer ();
  _row = 0;
  _rotate_180 = _match_direction && is_back (_page);
}

imgstream&
pdfstream::write (const byte_type *line, size_type n)
{
  if (!line || 0 == n) return *this;
  if (0 == _page)
    {
      write_header ();
    }
  if (0 == _row)
    {
      // adjust to PDF coordinate system (is this correct?)
      _pdf_h_sz = (72 * _h_sz) / _hres;
      _pdf_v_sz = (72 * _v_sz) / _vres;

      write_page_header ();
      ++_page;
    }

  if (_stream)
    {
      _stream->write (line, n);
    }
  else if (_g3)
    {
      string buf = (*_g3) (line, n);
      _doc->write (buf.data (), buf.size ());
    }
  else
    {
      _doc->write (line, n);
    }
  ++_row;

  return *this;
}

void
pdfstream::write_header ()
{
  _doc->header ();

  delete _pages;
  _pages = new pdf::dictionary ();

  pdf::dictionary info;
  info.insert ("Producer", pdf::primitive ("(iscan)"));
  info.insert ("Creator", pdf::primitive ("(iscan)"));
  _doc->write (info);

  pdf::dictionary catalog;
  catalog.insert ("Type", pdf::primitive ("/Catalog"));
  catalog.insert ("Pages", pdf::object (_pages->obj_num ()));
  _doc->write (catalog);

  delete _trailer;
  _trailer = new pdf::dictionary ();
  _trailer->insert ("Info", pdf::object (info.obj_num ()));
  _trailer->insert ("Root", pdf::object (catalog.obj_num ()));

  delete _page_list;
  _page_list = new pdf::array ();
}

void
pdfstream::write_page_header ()
{
  pdf::dictionary page;

  _page_list->insert (pdf::object (page.obj_num ()));

  _pages->insert ("Type", pdf::primitive ("/Pages"));
  _pages->insert ("Kids", _page_list);
  _pages->insert ("Count", pdf::primitive (_page_list->size ()));

  _doc->write (*_pages);

  pdf::dictionary image;
  pdf::dictionary contents;

  pdf::array mbox;
  mbox.insert (pdf::primitive (0));
  mbox.insert (pdf::primitive (0));
  mbox.insert (pdf::primitive (_pdf_h_sz));
  mbox.insert (pdf::primitive (_pdf_v_sz));

  std::stringstream ss2;
  std::string img_name;

  ss2 << "iscanImage" << _page;
  img_name = ss2.str ();

  pdf::array procset;
  std::string cproc = "/ImageB";
  if (RGB == _cspc)
    {
      cproc = "/ImageC";
    }
  pdf::dictionary tmp;
  tmp.insert (img_name.c_str (), pdf::object (image.obj_num ()));

  procset.insert (pdf::primitive ("/PDF"));
  procset.insert (pdf::primitive (cproc));

  pdf::dictionary rsrc;
  rsrc.insert ("XObject", &tmp);
  rsrc.insert ("ProcSet", &procset);

  page.insert ("Type", pdf::primitive ("/Page"));
  page.insert ("Parent", pdf::object (_pages->obj_num ()));
  page.insert ("Resources", &rsrc);
  page.insert ("MediaBox", &mbox);
  page.insert ("Contents", pdf::object (contents.obj_num ()));

  _doc->write (page);

  _doc->begin_stream (contents);

  // transformation matrices must be specified in reverse order
  std::stringstream ss;
  ss << "q" << std::endl;
  ss << _pdf_h_sz << " 0 0 " << _pdf_v_sz << " 0 0 cm" << std::endl;
  if (_rotate_180)
    {
      // undo the translation below
      ss << "1 0 0 1 0.5 0.5 cm" << std::endl;

      // reflect along x and y axis
      ss << "-1 0 0 -1 0 0 cm" << std::endl;

      // translate so the image midpoint lies on the origin
      ss << "1 0 0 1 -0.5 -0.5 cm" << std::endl;
    }
  ss << "/" << img_name << " Do" << std::endl;
  ss << "Q";

  _doc->write (ss.str ());
  _doc->end_stream ();

  write_image_object (image, img_name);

  _need_page_trailer = true;
}

void
pdfstream::write_image_object (pdf::dictionary& image, std::string name)
{
  delete _img_height_obj;
  _img_height_obj = new pdf::primitive ();

  image.insert ("Type", pdf::primitive ("/XObject"));
  image.insert ("Subtype", pdf::primitive ("/Image"));
  image.insert ("Width", pdf::primitive (_h_sz));
  image.insert ("Height", pdf::object (_img_height_obj->obj_num ()));

  pdf::array decode;
  std::string dev = "/DeviceGray";
  if (RGB == _cspc) dev = "/DeviceRGB";
  
  if (monochrome == _cspc)
    {
      if (!_g3) _g3 = new fax_encoder;
    }
  else
    {
      _do_jpeg = iscan::jpegstream::is_usable ();
    }
  image.insert ("ColorSpace", pdf::primitive (dev));
  image.insert ("BitsPerComponent", pdf::primitive (_bits));
  image.insert ("Interpolate", pdf::primitive ("true"));

  pdf::dictionary parms;
  if (_do_jpeg)
    {
      image.insert ("Filter", pdf::primitive ("/DCTDecode"));
    }
  else if (_g3)
    {
      image.insert ("Filter", pdf::primitive ("/CCITTFaxDecode"));

      parms.insert ("Columns", pdf::primitive (_h_sz));
      parms.insert ("Rows", pdf::primitive (_v_sz));
      parms.insert ("EndOfBlock", pdf::primitive ("false"));
      parms.insert ("EndOfLine", pdf::primitive ("true"));
      parms.insert ("EncodedByteAlign", pdf::primitive ("true"));
      parms.insert ("K", pdf::primitive (0));   // CCITT3 1-D encoding
      image.insert ("DecodeParms", &parms);
    }

  // see PDF reference 1.7 p. 342 and p. 1107 # 53
  image.insert ("Name", pdf::primitive ("/" + name));

  _doc->begin_stream (image);

  if (_do_jpeg)
    {
      _stream = new jpegstream (_file);
    }

  if (_stream)
    {
      _stream->size (_h_sz, _v_sz);
      _stream->depth (_bits);
      _stream->colour (_cspc);
      _stream->resolution (_hres, _vres);
    }
}

void
pdfstream::write_page_trailer ()
{
  delete _stream;
  _stream = NULL;

  _doc->end_stream ();

  *_img_height_obj = pdf::primitive (_row);
  _doc->write (*_img_height_obj);

  _doc->trailer (*_trailer);

  _need_page_trailer = false;

  _pdf_h_sz = 0;
  _pdf_v_sz = 0;

  _do_jpeg = false;
}

}       // namespace iscan
