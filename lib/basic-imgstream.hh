//  basic-imgstream.hh -- the mother of all image streams
//  Copyright (C) 2008, 2009  SEIKO EPSON CORPORATION
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


#ifndef iscan_basic_imgstream_hh_included
#define iscan_basic_imgstream_hh_included

#ifndef __cplusplus
#error "This is a C++ header file; use a C++ compiler to compile it."
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"

#ifndef LT_MODULE_EXT
#define LT_MODULE_EXT LTDL_SHLIB_EXT
#endif

#ifndef LT_MODULE_PATH_VAR
#define LT_MODULE_PATH_VAR LTDL_SHLIBPATH_VAR
#endif

#ifndef LT_DLSEARCH_PATH
#define LT_DLSEARCH_PATH LTDL_SYSSEARCHPATH
#endif

#endif

#include <dirent.h>
#include <ltdl.h>
#include <stdexcept>

namespace iscan
{
  using std::runtime_error;

  enum colour_space
    {
      NONE,

      monochrome,
      mono = monochrome,

      grayscale,
      greyscale = grayscale,
      gray = grayscale,
      grey = grayscale,

      RGB,
      RGB_alpha
    };

  class basic_imgstream
  {
  public:
    typedef char   byte_type;
    typedef size_t size_type;

    virtual ~basic_imgstream (void);

    virtual basic_imgstream& write (const byte_type *line, size_type n) = 0;
    virtual basic_imgstream& flush (void);

    virtual basic_imgstream& size       (size_type h_sz, size_type v_sz);
    virtual basic_imgstream& resolution (size_type hres, size_type vres);
    virtual basic_imgstream& depth      (size_type bits);
    virtual basic_imgstream& colour     (colour_space space);
    virtual void rotate_180 (bool yes) {};

  protected:
    basic_imgstream (void);

    size_type _h_sz;
    size_type _v_sz;
    size_type _hres;
    size_type _vres;

    size_type    _bits;
    colour_space _cspc;

  private:                        // undefined to prevent copying
    basic_imgstream (const basic_imgstream&);
    basic_imgstream& operator= (const basic_imgstream&);


  // API to deal with external image format libraries

  protected:
    typedef lt_dlhandle dl_handle;
    typedef lt_ptr      dl_ptr;

    static dl_handle dlopen  (const char *libname,
                              bool (*validate) (dl_handle) = NULL);
    static dl_ptr    dlsym   (dl_handle lib, const char *funcname);
    static int       dlclose (dl_handle lib);

  private:
    static dl_handle    find_dlopen (const char *libname,
                                     bool (*validate) (dl_handle));
    static const char *_libname;

    friend int selector (const dirent *);

#ifdef __GNUC__
#define fundecl(returntype,funcname,arglist...)       \
    typedef returntype (*funcname##_f) (arglist);     \
    funcname##_f funcname;
#else
#error "Your compiler is not known to support macros with a variable"
#error "number of arguments.  In case it does, please report this to"
#error "the library maintainers and include a suitable preprocessor"
#error "check for them to add.  A patch will be most appreciated."
#endif

  };

} // namespace iscan

#endif /* !defined (iscan_basic_imgstream_hh_included) */
