//  writer.cc -- putting PDF object in a file
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
#include <config.h>
#endif

#include "writer.hh"

#include <sstream>
#include <stdexcept>

namespace iscan
{

namespace pdf
{
  using std::runtime_error;
  using std::string;

writer::writer (FILE *file)
  : _xref (xref ()), _file (file)
{
  _xref_pos = 0;
  _last_xref_pos = 0;
  _saved_pos = 0;
  _mode = object_mode;
  _stream_len_obj = NULL;
}

writer::~writer ()
{
  delete _stream_len_obj;
  _stream_len_obj = NULL;
}

void
writer::write (object& obj)
{
  if (object_mode != _mode)
    {
      throw runtime_error ("invalid call to pdf::writer::write (object&)");
    }

  _xref[obj.obj_num ()] = ftell (_file);

  fprintf (_file, "%zu 0 obj\n", obj.obj_num ());
  obj.print (_file);
  fprintf (_file, "\n");
  fprintf (_file, "endobj\n");
}

void
writer::begin_stream (dictionary& dict)
{
  if (stream_mode == _mode)
    {
      throw runtime_error ("invalid call to pdf::writer::begin_stream ()");
    }
  _mode = stream_mode;

  _stream_len_obj = new primitive ();
  dict.insert ("Length", object (_stream_len_obj->obj_num ()));

  _xref[dict.obj_num ()] = ftell (_file);

  fprintf (_file, "%zu 0 obj\n", dict.obj_num ());
  dict.print (_file);
  fprintf (_file, "\n");
  fprintf (_file, "stream\n");

  _saved_pos = ftell (_file);
}

void
writer::write (const char *buf, size_t n)
{
  if (stream_mode != _mode)
    {
      throw runtime_error ("invalid call to pdf::writer::write ()");
    }
  size_t rv = fwrite (buf, sizeof (char), n, _file);
  if (rv != n) throw std::ios_base::failure ("write error");
}

void
writer::write (const string& s)
{
  if (stream_mode != _mode)
    {
      throw runtime_error ("invalid call to pdf::writer::write ()");
    }
  fprintf (_file, "%s", s.c_str ());
}

void
writer::end_stream ()
{
  if (stream_mode != _mode)
    {
      throw runtime_error ("invalid call to pdf::writer::end_stream ()");
    }
  _mode = object_mode;

  size_t pos = ftell (_file);
  size_t length = pos - _saved_pos;

  fprintf (_file, "\n");
  fprintf (_file, "endstream");
  fprintf (_file, "\n");
  fprintf (_file, "endobj");
  fprintf (_file, "\n");

  // FIXME: overload the '=' operator in pdf::primitive
  *_stream_len_obj = primitive (length);

  write (*_stream_len_obj);
  delete _stream_len_obj;
  _stream_len_obj = NULL;
}

void
writer::header ()
{
  if (_mode == stream_mode)
    {
      throw runtime_error ("cannot write header in stream mode");
    }
  fprintf (_file, "%%PDF-1.0\n");
}

void
writer::trailer (dictionary& trailer_dict)
{
  if (_mode == stream_mode)
    {
      throw runtime_error ("cannot write trailer in stream mode");
    }
  write_xref ();
  write_trailer (trailer_dict);
}

// FIXME: clean up this kludge
void
writer::write_xref ()
{
  xref::const_iterator it;

  _last_xref_pos = _xref_pos;
  _xref_pos = ftell (_file);

  fprintf (_file, "xref\n");

  stringstream ss;
  size_t start_obj_num = 0;
  size_t cur_obj_num = 0;
  size_t last_obj_num = 0;

  ss << "0000000000 65535 f " << endl;
  for(it = _xref.begin (); _xref.end () != it; ++it)
    {
      cur_obj_num = it->first;

      if (cur_obj_num != last_obj_num + 1)
        {
          // write out the current xref section and start a new one
          fprintf (_file, "%zu %zu\n", start_obj_num,
                   last_obj_num + 1 - start_obj_num);
          fprintf (_file, "%s", ss.str ().c_str ());

          ss.str ("");          // flush stream
          start_obj_num = cur_obj_num;
        }

        last_obj_num = cur_obj_num;

        ss.width (10);
        ss.fill ('0');
        ss << it->second << " 00000 n " << endl;
    }

  if (!ss.str ().empty ())
    {
      fprintf (_file, "%zu %zu\n", start_obj_num,
               last_obj_num + 1 - start_obj_num);
      fprintf (_file, "%s", ss.str ().c_str ());
    }
}

void
writer::write_trailer (dictionary& trailer_dict)
{
  trailer_dict.insert ("Size", primitive (_xref.size () + 1));
  if (_last_xref_pos != 0)
    {
      trailer_dict.insert ("Prev", primitive (_last_xref_pos));
    }

  fprintf (_file, "trailer\n");
  trailer_dict.print (_file);
  fprintf (_file, "\n");
  fprintf (_file, "startxref\n");
  fprintf (_file, "%zu\n", _xref_pos);
  fprintf (_file, "%%%%EOF\n");

  _xref.clear ();
}

}       // namespace pdf
}       // namespace iscan
