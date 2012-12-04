//  test-model-info.hh -- test suite for model info objects
//  Copyright (C) 2010  SEIKO EPSON CORPORATION
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


#ifndef included_test_model_info_hh
#define included_test_model_info_hh

#ifndef __cplusplus
#error "This is a C++ include file.  Use a C++ compiler to compile"
#error "code that includes this file."
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../model-info.h"
#include <cxxtest/TestSuite.h>

#include <cstdlib>
#include <cstring>
#include <string>

#include "../list.h"
#include "../message.h"
#include "../get-infofile.h"

struct base
{
  std::string dir;
  void       *cache;

  void setUp (void)
  {
    //  Log at least broken promises and unmet requirements.
    setenv ("SANE_DEBUG_EPKOWA", "FATAL", false);
    msg_init ();

    char *srcdir = getenv ("srcdir");
    dir = (srcdir ? srcdir : ".");

    cache = NULL;
  }

  void tearDown (void)
  {
    cache = model_info_cache_exit (cache);
    promise (!cache);
  }
};

class test_model_cache_info : public CxxTest::TestSuite, public base
{
public:

  void test_cache_life_cycle (void)
  {
    cache = model_info_cache_init (dir.c_str (), NULL);
    TS_ASSERT (cache);

    cache = model_info_cache_exit (cache);
    TS_ASSERT (!cache);
  }

  void test_cache_life_cycle_status (void)
  {
    SANE_Status status = SANE_STATUS_NO_MEM;

    cache = model_info_cache_init (dir.c_str (), &status);
    TS_ASSERT (cache);

    TS_ASSERT (SANE_STATUS_NO_MEM != status);

    cache = model_info_cache_exit (cache);
    TS_ASSERT (!cache);
  }

  void test_cache_unique_entries (void)
  {
    cache = model_info_cache_init (dir.c_str (), NULL);
    TS_ASSERT (cache);

    for (int i = 0; i < 10; ++i) {
      model_info_cache_get_info ("GT-X970", NULL);
      model_info_cache_get_info ("ES-H300", NULL);
    }
    TS_ASSERT_EQUALS (2, list_size (static_cast<list *> (cache)));
  }

private:
  void setUp (void) { base::setUp (); }
  void tearDown (void) { base::setUp (); }
};

class test_model_info : public CxxTest::TestSuite, public base
{
public:

  void test_get_non_existent_model (void)
  {
    char *model = model_info_cache_get_model (bad_fw_name);
    TS_ASSERT_EQUALS (std::string (model), std::string (bad_fw_name));
    free (model);
  }

  void test_get_non_existent_info (void)
  {
    const void *info = model_info_cache_get_info (bad_fw_name, NULL);
    TS_ASSERT (info);
  }

  void test_get_info_from_loaded_cache (void)
  {
    // load cache
    model_info_cache_get_info (bad_fw_name, NULL);
    model_info_cache_get_info ("GT-X970", NULL);        // XML data
    model_info_cache_get_info ("ES-H300", NULL);        // XML data

    // get info
    const void *info = model_info_cache_get_info ("GT-X970", NULL);
    TS_ASSERT (info);
    TS_ASSERT (model_info_cache_get_info ("ES-H300", NULL));
    TS_ASSERT (model_info_cache_get_info (bad_fw_name, NULL));
    TS_ASSERT_EQUALS (info, model_info_cache_get_info ("GT-X970", NULL));
  }

  void test_get_existing_model (void)
  {
    setenv ("TZ", "", true);    // force UTC, i.e. overseas model name

    char *model = model_info_cache_get_model ("ES-H300");
    TS_ASSERT_EQUALS (std::string ("GT-2500"), std::string (model));
    free (model);
  }

  void test_default_values (void)
  {
    const void *p = model_info_cache_get_info (bad_fw_name, NULL);

    const char *name = model_info_get_name (p);
    TS_ASSERT_EQUALS (std::string (bad_fw_name), std::string (name));

    const EpsonScanHard profile = model_info_get_profile (p);
    TS_ASSERT_SAME_DATA (profile, &epson_scan_hard[0],
                         sizeof (profile));
  }

  void test_profile_equality_values (void)
  {
    //  Identical models but for their fw_names.
    const void *p1 = model_info_cache_get_info ("GT-10000", NULL);
    const void *p2 = model_info_cache_get_info ("ES-6000", NULL);

    TS_ASSERT_DIFFERS (p1, p2);
    TS_ASSERT_SAME_DATA (model_info_get_profile (p1),
                         model_info_get_profile (p2),
                         sizeof (const EpsonScanHard));
  }

private:

  //  A firmware name that is guaranteed not to be used.
  static const char *bad_fw_name;

  void setUp (void)
  {
    base::setUp ();
    cache = model_info_cache_init (dir.c_str (), NULL);
    TS_ASSERT (cache);
    //  The ESC/I spec has a 16 byte limit on the F/W name.
    TS_ASSERT (16 < strlen (bad_fw_name));
  }

  void tearDown (void)
  {
    base::tearDown ();
  }
};

const char * test_model_info::bad_fw_name = " __ BAD F/W NAME __ ";

#endif  /* !defined (included_test_model_info_hh) */
