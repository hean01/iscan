//  file-opener.cc -- dealing with files when doing multi-image scans
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

#include "file-opener.hh"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <list>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <sys/types.h>
#include <sys/stat.h>

namespace iscan
{
  static string tempfile (const string& dirname = string ());

  //! Opening one or more files in a temporary file location.
  file_opener::file_opener (bool collate)
    : _collate (collate), _filename (string ()), _tempfile (string ()),
      _fp (NULL), _pattern (NULL)
  {
  }

  //! Opening a file by \a name.
  file_opener::file_opener (const string& name)
    : _collate (true), _filename (string ()), _tempfile (string ()),
      _fp (NULL), _pattern (NULL)
  {
    common_init (name);
  }

  //! Opening files following a naming \a pattern.
  file_opener::file_opener (const string& pattern, unsigned int start_index)
    : _collate (false), _filename (string ()), _tempfile (string ()),
      _fp (NULL), _pattern (NULL)
  {
    common_init (pattern);

    string::size_type hash = _pattern->basename.find_last_not_of (hash_mark);

    if (string::npos == hash)
      {
        delete _pattern;        // new'd in common_init()
        throw std::invalid_argument ("invalid file name pattern");
      }
    ++hash;                     // first hash_mark

    _pattern->digits   = _pattern->basename.length () - hash;
    _pattern->basename = _pattern->basename.substr (0, hash);
    _pattern->index    = start_index;
  }

  file_opener::~file_opener (void)
  {
    if (_fp) close ();
    if (_tempfile != _filename)
      rename ();
    delete _pattern;
  }

  //! Returns the C \c FILE pointer associated with a name().
  file_opener::operator FILE * (void)
  {
    if (_filename.empty ()) set_names ();
    if (!_fp) open ();
    return _fp;
  }

  //! Returns the pathname of the output destination.
  const string&
  file_opener::name (void) const
  {
    if (_filename.empty ())
      const_cast<file_opener *> (this)->set_names ();
    return _filename;
  }

  //! Returns the pathname of the interim output destination.
  /*! \deprecated  Flaming hack to quickly fix our TIFF support.
   */
  const string&
  file_opener::temp (void) const
  {
    if (_filename.empty ())
      const_cast<file_opener *> (this)->set_names ();
    return _tempfile;
  }

  //! Returns the file's extension (without extension separator).
  string
  file_opener::extension (void) const
  {
    if (!_pattern) return null_ext;
    return _pattern->extension.substr (ext_sep.size ());
  }

  //! Opens the next file, unless the file_opener is_collating().
  file_opener&
  file_opener::operator++ (void)
  {
    if (_collate) return *this;

    if (_fp) close ();
    if (_tempfile != _filename)
      rename ();
    set_names (true);
    open ();

    return *this;
  }

  //! Tells whether or not output will be collated.
  bool
  file_opener::is_collating (void) const
  {
    return _collate;
  }

  //! Removes the output destination.
  void
  file_opener::remove (void)
  {
    if (_fp)
      {
        close ();
        if (0 != ::remove (_tempfile.c_str ()))
          throw std::ios_base::failure (strerror (errno));
      }
    _tempfile = string ();
    _filename = string ();
  }

  const string file_opener::dir_sep = "/";
  const string file_opener::ext_sep = ".";
  const char file_opener::hash_mark = '#';

  const string file_opener::null_ext = string ();

  //! Handles common part of the constructors taking a string.
  void
  file_opener::common_init (const string& s)
  {
    string::size_type sep = s.rfind (dir_sep);
    string::size_type dot = s.rfind (ext_sep);

    if (string::npos != sep && dot < sep)
      dot = string::npos;

    _pattern = new struct pattern;
    if (string::npos != sep)
      {
        ++sep;
        _pattern->dirname  = s.substr (0, sep);
        _pattern->basename = s.substr (sep, dot - sep);
      }
    else
      {
        _pattern->dirname  = string ();
        _pattern->basename = s.substr (0, dot);
      }

    if (string::npos != dot)
      {
        _pattern->extension = s.substr (dot);
      }
    else
      {
        _pattern->extension = null_ext;
      }
    _pattern->digits = 0;
    _pattern->index  = 0;
  }

  void
  file_opener::set_names (bool next)
  {
    using std::stringstream;
    using std::setfill;
    using std::setw;

    if (!_pattern)
      {
        _tempfile = tempfile ();
        _filename = _tempfile;
      }
    else
      {
        if (next) ++_pattern->index;

        stringstream ss;
        ss << _pattern->dirname
           << _pattern->basename;
        if (_pattern->digits)
          {
            ss << setfill ('0')
               << setw (_pattern->digits)
               << _pattern->index;
          }
        ss << _pattern->extension;

        _filename = ss.str ();
        _tempfile = (!_pattern->digits
                     ? _filename : tempfile (_pattern->dirname.empty ()
                                             ? "." : _pattern->dirname));
      }
  }

  //! Error handling wrapper around the C fopen() call.
  void
  file_opener::open (void)
  {
    _fp = fopen (_tempfile.c_str (), "wb");
    if (!_fp)
      throw std::ios_base::failure (strerror (errno));
  }

  //! Error handling wrapper around the C fclose() call.
  void
  file_opener::close (void)
  {
    if (!_fp) return;

    int rv = fclose (_fp);
    _fp = NULL;
    
    if (0 != rv)
      throw std::ios_base::failure (strerror (errno));
  }

  void
  file_opener::rename (void)
  {
    if (0 != ::rename (_tempfile.c_str (), _filename.c_str ()))
      throw std::ios_base::failure (strerror (errno));

    _tempfile = _filename;
  }


  //! Creates a temporary file in a secure way.
  static string
  tempfile (const string& dirname)
  {
    using std::list;

    list<string> dirs;
    if (!dirname.empty ()) dirs.push_back (dirname);
    if (getenv ("TMPDIR")) dirs.push_back (getenv ("TMPDIR"));
#ifdef P_tmpdir
    dirs.push_back (P_tmpdir);  // C library default
#endif
    dirs.push_back ("/tmp");    // last resort

    string filename;
    list<string>::iterator it = dirs.begin ();

    while (dirs.end () != it && filename.empty ())
      {
        string ts = (*it + file_opener::dir_sep
                     + (!dirname.empty () ? "." : "")
                     + PACKAGE_TARNAME "XXXXXX");
        char  *tc = new char [ts.length() + 1];

        ts.copy (tc, ts.length ());
        tc[ts.length ()] = '\0';

        mode_t um = umask (0);
        umask (dirname.empty () ? 0077 : um);   // use safe permissions
        if (0 <= mkstemp (tc))
          filename = tc;
        umask (um);

        // mkstemp from glibc 2.0.7 and later always uses 0600, (try
        // to) revert here
        if (!dirname.empty ())
          {
            chmod (filename.c_str (), (  S_IRUSR | S_IRGRP | S_IROTH
                                       | S_IWUSR | S_IWGRP | S_IWOTH) & ~um);
          }

        delete [] tc;
        ++it;
      }

    return filename;
  }

} // namespace iscan
