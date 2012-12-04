//  file-opener.hh -- dealing with files when doing multi-image scans
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


#ifndef iscan_file_opener_hh_included
#define iscan_file_opener_hh_included

#ifndef __cplusplus
#error "This is a C++ header file; use a C++ compiler to compile it."
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstdio>
#include <string>

namespace iscan
{
  using std::string;

  class file_opener
  {
  public:
    explicit file_opener (bool collate);
    explicit file_opener (const string& name);
    file_opener (const string& pattern, unsigned int start_index);
    ~file_opener (void);

    operator FILE * (void);

    const string& name (void) const;
    const string& temp (void) const;
    string extension (void) const;

    file_opener& operator++ (void);

    bool is_collating (void) const;

    void remove (void);

    static const string dir_sep;
    static const string ext_sep;
    static const char hash_mark;

    static const string null_ext;

  private:
    void common_init (const string& s);
    void set_names (bool next = false);

    void open (void);
    void close (void);
    void rename (void);

    bool _collate;

    string _filename;
    string _tempfile;
    FILE *_fp;

    struct pattern
    {
      string extension;
      string basename;
      string dirname;           // includes final dir_sep
      size_t digits;
      unsigned int index;
    };
    struct pattern *_pattern;
  };

} // namespace iscan

#endif /* !defined (iscan_file_opener_hh_included) */
