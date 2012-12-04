//  test-cfg-obj.hh -- test suite for configuration objects
//  Copyright (C) 2008  SEIKO EPSON CORPORATION
//
//  License: GPLv2+|iscan
//  Authors: AVASYS CORPORATION
//
//  This file is part of Image Scan!'s SANE backend test suite.
//
//  Image Scan!'s SANE backend test suite is free software.
//  You can redistribute it and/or modify it under the terms of the GNU
//  General Public License as published by the Free Software Foundation;
//  either version 2 of the License or at your option any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//  You ought to have received a copy of the GNU General Public License
//  along with this package.  If not, see <http://www.gnu.org/licenses/>.
//
//
//  Linking Image Scan!'s SANE backend statically or dynamically with
//  other modules is making a combined work based on this SANE backend.
//  Thus, the terms and conditions of the GNU General Public License
//  cover the whole combination.
//
//  As a special exception, the copyright holders of Image Scan!'s SANE
//  backend give you permission to link Image Scan!'s SANE backend with
//  SANE frontends that communicate with Image Scan!'s SANE backend
//  solely through the SANE Application Programming Interface,
//  regardless of the license terms of these SANE frontends, and to
//  copy and distribute the resulting combined work under terms of your
//  choice, provided that every copy of the combined work is
//  accompanied by a complete copy of the source code of Image Scan!'s
//  SANE backend (the version of Image Scan!'s SANE backend used to
//  produce the combined work), being distributed under the terms of
//  the GNU General Public License plus this exception.  An independent
//  module is a module which is not derived from or based on Image
//  Scan!'s SANE backend.
//
//  As a special exception, the copyright holders of Image Scan!'s SANE
//  backend give you permission to link Image Scan!'s SANE backend with
//  independent modules that communicate with Image Scan!'s SANE
//  backend solely through the "Interpreter" interface, regardless of
//  the license terms of these independent modules, and to copy and
//  distribute the resulting combined work under terms of your choice,
//  provided that every copy of the combined work is accompanied by a
//  complete copy of the source code of Image Scan!'s SANE backend (the
//  version of Image Scan!'s SANE backend used to produce the combined
//  work), being distributed under the terms of the GNU General Public
//  License plus this exception.  An independent module is a module
//  which is not derived from or based on Image Scan!'s SANE backend.
//
//  Note that people who make modified versions of Image Scan!'s SANE
//  backend are not obligated to grant special exceptions for their
//  modified versions; it is their choice whether to do so.  The GNU
//  General Public License gives permission to release a modified
//  version without this exception; this exception also makes it
//  possible to release a modified version which carries forward this
//  exception.


#ifndef included_test_cfg_obj_hh
#define included_test_cfg_obj_hh

#ifndef __cplusplus
#error "This is a C++ include file.  Use a C++ compiler to compile"
#error "code that includes this file."
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../cfg-obj.h"
#include <cxxtest/TestSuite.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <unistd.h>

#include "../message.h"


class test_cfg_obj : public CxxTest::TestSuite
{
public:

  void test_life_cycle (void)
  {
    cfg = cfg_init (dir.c_str (), NULL);
    TS_ASSERT (cfg);

    cfg = cfg_exit (cfg);
    TS_ASSERT (!cfg);
  }

  void test_life_cycle_status (void)
  {
    SANE_Status status = SANE_STATUS_NO_MEM;

    cfg = cfg_init (dir.c_str (), &status);
    TS_ASSERT (cfg);

    TS_ASSERT (SANE_STATUS_NO_MEM != status);

    cfg = cfg_exit (cfg);
    TS_ASSERT (!cfg);
  }

  void test_key_query (void)
  {
    cfg = cfg_init (dir.c_str (), NULL);
    TS_ASSERT (cfg);

    TS_ASSERT (cfg_has (cfg, CFG_KEY_USB));
  }

  void test_option_value_query (void)
  {
    cfg = cfg_init (dir.c_str (), NULL);
    TS_ASSERT (cfg);

    TS_ASSERT (cfg_has (cfg, CFG_KEY_OPTION));
    TS_ASSERT (cfg_has_value (cfg, CFG_KEY_OPTION, "prefer-adf"));
    TS_ASSERT (!cfg_has_value (cfg, CFG_KEY_OPTION, "not-a-valid-option"));
    TS_ASSERT (!cfg_has_value (cfg, CFG_KEY_OPTION, "not-in-config"));
  }

  void test_key_mutator (void)
  {
    cfg = cfg_init (dir.c_str (), NULL);
    TS_ASSERT (cfg);

    TS_ASSERT (cfg_has (cfg, CFG_KEY_USB));
    cfg_set (cfg, CFG_KEY_USB, false);
    TS_ASSERT (!cfg_has (cfg, CFG_KEY_USB));
    cfg_set (cfg, CFG_KEY_USB, true);
    TS_ASSERT (cfg_has (cfg, CFG_KEY_USB));
  }

  void test_net_registration (void)
  {
    add_cfg_entry ("net\n");

    cfg = cfg_init (dir.c_str (), NULL);
    TS_ASSERT (!cfg_has (cfg, CFG_KEY_NET));
  }

private:

  void       *cfg;
  std::string dir;

  /*! Create a temporary, minimal configuration directory.

      The configuration directory is made available to the unit test
      environment via the \c SANE_CONFIG_DIR environment variable so
      configuration objects (should) take note.  In addition, a very
      minimal configuration file, containing the \c usb keyword and
      a few options is created in this directory as well.
   */
  void setUp (void)
  {
    int result;
    msg_init ();

    char dirname_template[] = ".cfg-obj-XXXXXX";
    char *dirname = mkdtemp (dirname_template);
    if (!dirname)
      {
        err_fatal ("mkdtemp: %s", strerror (errno));
      }
    require (dirname);

    result = chdir (dirname);
    require (0 == result);

    std::ofstream ofs (cfg_file_name);
    ofs << "usb\n";
    ofs << "option   prefer-adf  \n";
    ofs << "option not-a-valid-option\n";

    result = chdir ("..");
    require (0 == result);

    result = setenv ("SANE_CONFIG_DIR", dirname, true);
    require (0 == result);

    cfg = NULL;
    dir = dirname;
  }

  /*! Attempts to undo the effects of setUp().

      The environment's \c SANE_CONFIG_DIR is unset and the temporary
      configuration directory is recursively removed.  Failures to
      undo any of the necessary actions are logged but will not cause
      a test to fail.
   */
  void tearDown (void)
  {
    int result;

    if (0 != unsetenv ("SANE_CONFIG_DIR"))
      {
        err_minor ("unsetenv: %s", strerror (errno));
      }

    if (0 != chdir (dir.c_str ()))
      {
        err_minor ("%s: %s", dir.c_str (), strerror (errno));
      }
    else
      {
        if (0 != unlink (cfg_file_name))
          {
            err_minor ("%s: %s", cfg_file_name, strerror (errno));
          }
        if (0 != unlink ("usb"))
          {
            err_minor ("%s: %s", "usb", strerror (errno));
          }
        result = chdir ("..");
        require (0 == result);
      }

    if (0 != rmdir (dir.c_str ()))
      {
        err_minor ("%s: %s", dir.c_str (), strerror (errno));
      }
    dir = "";

    cfg = cfg_exit (cfg);
    promise (!cfg);
  }

  void add_cfg_entry (const char *str)
  {
    chdir (dir.c_str ());
    std::ofstream ofs (cfg_file_name,
                       std::ios_base::out | std::ios_base::app);

    ofs << str;
    chdir ("..");
  }
};


#endif  /* !defined (included_test_cfg_obj_hh) */
