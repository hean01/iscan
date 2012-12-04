//  basic-imgstream.cc -- the mother of all image streams
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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "basic-imgstream.hh"

#include <cstdlib>
#include <argz.h>

namespace iscan
{

#if __GLIBC_PREREQ(2, 10)
  typedef const dirent** dirtype;
#else
  typedef const void* dirtype;
#endif

  basic_imgstream::basic_imgstream (void)
    : _h_sz (0), _v_sz (0), _hres (0), _vres (0), _bits (0), _cspc (NONE)
  {
  }

  basic_imgstream::~basic_imgstream (void)
  {
  }

  basic_imgstream&
  basic_imgstream::flush (void)
  {
    return *this;
  }

  basic_imgstream&
  basic_imgstream::size (size_type h_sz, size_type v_sz)
  {
    _h_sz = h_sz;
    _v_sz = v_sz;
    return *this;
  }

  basic_imgstream&
  basic_imgstream::resolution (size_type hres, size_type vres)
  {
    _hres = hres;
    _vres = vres;
    return *this;
  }

  basic_imgstream&
  basic_imgstream::depth (size_type bits)
  {
    _bits = bits;
    return *this;
  }

  basic_imgstream&
  basic_imgstream::colour (colour_space space)
  {
    _cspc = space;
    return *this;
  }


  basic_imgstream::dl_handle
  basic_imgstream::dlopen (const char *libname,
                           bool (*validate) (lt_dlhandle))
  {
    if (0 != lt_dlinit ())
      {
        throw runtime_error (lt_dlerror ());
      }

    dl_handle lib = find_dlopen (libname, validate);
    if (!lib)
      {
        lt_dlexit ();
        throw runtime_error ("no usable library found");
      }

    return lib;
  }

  basic_imgstream::dl_ptr
  basic_imgstream::dlsym (dl_handle lib, const char *funcname)
  {
    return lt_dlsym (lib, funcname);
  }

  int
  basic_imgstream::dlclose (dl_handle lib)
  {
    return lt_dlclose (lib);
  }

  // forward declarations
  static int reversionsort (dirtype, dirtype);
  int selector (const dirent *);

  //!
  /*! A number of distributions seems to have switched to a policy where
      the lib*.so files are provided by their -devel packages.  Moreover,
      the typical workstation install does not include such packages and
      lt_dlopenext() will understandably have trouble finding your lib*.

      This function is a fallback for such cases.  It will look for your
      library in the exact same places as lt_dlopenext(), but with this
      difference that it will try to open any file that matches lib*.so,
      starting with the one with the highest version number.

      Actually, it is just as smart lt_dlopenext() and uses the correct
      shared library extension for your platform.  However, it does not
      try libtool's .la extension.

      The general policy for memory allocation and access problems is to
      ignore them and muddle on or return the current result rightaway.

      This function returns NULL if no suitable library could be found
      and a handle to library otherwise.
   */
  basic_imgstream::dl_handle
  basic_imgstream::find_dlopen (const char *libname,
                                bool (*validate) (lt_dlhandle))
  {
    using std::bad_alloc;
    using std::string;

    dl_handle result = NULL;

    try
      {                         // prepping the selector()
        char *name = new char[strlen (libname)
                              + strlen (LT_MODULE_EXT) + 1];
        name = strcpy (name, libname);
        name = strcat (name, LT_MODULE_EXT);

        _libname = name;        // deleting _libname below, never mind
                                // that name goes out of scope here
      }
    catch (bad_alloc& oops)
      {
        return result;
      }

    char   *pathz  = NULL;
    size_t  length = 0;
    bool    is_pathz_ok = true;
    {                           // set up a library search path like
                                // that used by lt_dlopen()
      int delimiter = ':';

      const char *path = NULL;

      if ((path = lt_dlgetsearchpath ())
          && 0 != argz_add_sep (&pathz, &length, path, delimiter))
        {
          is_pathz_ok = false;
        }
      if ((path = getenv ("LTDL_LIBRARY_PATH"))
          && 0 != argz_add_sep (&pathz, &length, path, delimiter))
        {
          is_pathz_ok = false;
        }
      if ((path = getenv (LT_MODULE_PATH_VAR))
          && 0 != argz_add_sep (&pathz, &length, path, delimiter))
        {
          is_pathz_ok = false;
        }
      if ("x86_64" == string (ISCAN_HOST_CPU)
          && (path = "/usr/local/lib64:/usr/lib64:/lib64")
          && 0 != argz_add_sep (&pathz, &length, path, delimiter))
        {
          is_pathz_ok = false;
        }
      // Kludge for multiarch support introduced in Ubuntu 11.04
      if ("x86_64" == string (ISCAN_HOST_CPU))
        {
          if ((path = "/usr/lib/x86_64-linux-gnu:/lib/x86_64-linux-gnu")
              && 0 != argz_add_sep (&pathz, &length, path, delimiter))
            {
              is_pathz_ok = false;
            }
        }
      else
        {
          if ((path = "/usr/lib/i386-linux-gnu:/lib/i386-linux-gnu")
              && 0 != argz_add_sep (&pathz, &length, path, delimiter))
            {
              is_pathz_ok = false;
            }
        }
      if ((path = LT_DLSEARCH_PATH)
          && 0 != argz_add_sep (&pathz, &length, path, delimiter))
        {
          is_pathz_ok = false;
        }
    }

    if (is_pathz_ok)
      {                         // go fetch!
        const char *dir_name = NULL;
        while (!result
               && (dir_name = argz_next (pathz, length, dir_name)))
          {
            struct dirent **match = NULL;
            int count = scandir (dir_name, &match, selector, reversionsort);

            for (int i = 0; !result && i < count; ++i)
              {

                const char *file_name = match[i]->d_name;
                try
                  {
                    char *abs_file_name
                      = new char[strlen (dir_name) + strlen ("/")
                                 + strlen (file_name) + 1];
                    strcpy (abs_file_name, dir_name);
                    strcat (abs_file_name, "/");
                    strcat (abs_file_name, file_name);

                    result = lt_dlopen (abs_file_name);
                    if (validate && !validate (result))
                      {
                        lt_dlclose (result);
                        result = NULL;
                      }
                    delete [] abs_file_name;
                  }
                catch (bad_alloc& oops)
                  {
                    // just ignore and continue with the next match
                  }
                free (match[i]);
              }
            if (match) free (match);    // malloc'd by scandir()
          }
      }

    delete [] _libname;         // we new'd a name for our selector()
    free (pathz);               // malloc'd by argz_add_sep()

    return result;
  }

  //! Library name we are looking for.
  /*! The scandir() API does not allow for passing arbitrary data to the
      selector().  We use this variable to work around that limitation.

      Note that this makes users of selector() thread unsafe.
   */
  const char *basic_imgstream::_libname = NULL;

  //! Selects relevant library filenames.
  /*! Returns true if the leading part of the directory entry's file
      name matches the library name we are looking for.  The file name
      may contain trailing version information which is ignored.
   */
  int
  selector (const dirent *dir)
  {
    return (0 == strncmp (dir->d_name, basic_imgstream::_libname,
                          strlen (basic_imgstream::_libname)));
  }

  //! The C library's versionsort() function in reverse.
  static int
  reversionsort (dirtype a, dirtype b)
  {
    return versionsort (b, a);
  }

} // namespace iscan
