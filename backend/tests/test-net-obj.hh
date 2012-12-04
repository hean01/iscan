//  test-net-obj.hh -- test suite for network objects
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


#ifndef included_test_net_obj_hh
#define included_test_net_obj_hh

#ifndef __cplusplus
#error "This is a C++ include file.  Use a C++ compiler to compile"
#error "code that includes this file."
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../net-obj.h"

#include <cxxtest/TestSuite.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include "../message.h"


class test_net_obj : public CxxTest::TestSuite
{
public:

  void test_lifecycle (void)
  {
    void *net = net_init (get_current_dir_name (), NULL);
    TS_ASSERT (net);
    net = net_exit (net);
    TS_ASSERT (!net);
  }

  void test_missing_program (void)
  {
    char dirname_template[] = "network-XXXXXX";
    char *dirname = mkdtemp (dirname_template);
    TS_ASSERT (dirname);

    void *net = net_init (dirname, NULL);
    TS_ASSERT (!net);
    net = net_exit (net);
    TS_ASSERT (!net);

    if (0 != rmdir (dirname))
      {
        err_minor ("%s: %s", dirname, strerror (errno));
      }
  }

private:

  void setUp (void)
  {
    msg_init ();
  }
};


#endif  /* !defined (included_test_net_obj_hh) */
