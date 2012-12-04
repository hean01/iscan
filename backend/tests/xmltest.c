/*  xmltest.c -- simple model cache info checker
 *  Copyright (C) 2010  SEIKO EPSON CORPORATION
 *
 *  License: GPLv2+|iscan
 *  Authors: SEIKO EPSON CORPORATION
 *           AVASYS CORPORATION
 *
 *  This file is part of the SANE backend distributed with Image Scan!
 *
 *  Image Scan!'s SANE backend is free software.
 *  You can redistribute it and/or modify it under the terms of the GNU
 *  General Public License as published by the Free Software Foundation;
 *  either version 2 of the License or at your option any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *  You ought to have received a copy of the GNU General Public License
 *  along with this package.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 *  Linking Image Scan!'s SANE backend statically or dynamically with
 *  other modules is making a combined work based on this SANE backend.
 *  Thus, the terms and conditions of the GNU General Public License
 *  cover the whole combination.
 *
 *  As a special exception, the copyright holders of Image Scan!'s SANE
 *  backend give you permission to link Image Scan!'s SANE backend with
 *  SANE frontends that communicate with Image Scan!'s SANE backend
 *  solely through the SANE Application Programming Interface,
 *  regardless of the license terms of these SANE frontends, and to
 *  copy and distribute the resulting combined work under terms of your
 *  choice, provided that every copy of the combined work is
 *  accompanied by a complete copy of the source code of Image Scan!'s
 *  SANE backend (the version of Image Scan!'s SANE backend used to
 *  produce the combined work), being distributed under the terms of
 *  the GNU General Public License plus this exception.  An independent
 *  module is a module which is not derived from or based on Image
 *  Scan!'s SANE backend.
 *
 *  As a special exception, the copyright holders of Image Scan!'s SANE
 *  backend give you permission to link Image Scan!'s SANE backend with
 *  independent modules that communicate with Image Scan!'s SANE
 *  backend solely through the "Interpreter" interface, regardless of
 *  the license terms of these independent modules, and to copy and
 *  distribute the resulting combined work under terms of your choice,
 *  provided that every copy of the combined work is accompanied by a
 *  complete copy of the source code of Image Scan!'s SANE backend (the
 *  version of Image Scan!'s SANE backend used to produce the combined
 *  work), being distributed under the terms of the GNU General Public
 *  License plus this exception.  An independent module is a module
 *  which is not derived from or based on Image Scan!'s SANE backend.
 *
 *  Note that people who make modified versions of Image Scan!'s SANE
 *  backend are not obligated to grant special exceptions for their
 *  modified versions; it is their choice whether to do so.  The GNU
 *  General Public License gives permission to release a modified
 *  version without this exception; this exception also makes it
 *  possible to release a modified version which carries forward this
 *  exception.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "xmltest.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "../list.h"
#include "../message.h"
#include "../model-info.h"
#include "../utils.h"

static bool check_cache_content (list *info);
static bool check_info_content (const _model_info_t *info);
static bool model_info_test_cmp (const _model_info_t *info,
                                 const _model_info_t_test *reference);

int
main (int argc, char** argv)
{
  bool  pass  = false;
  list *cache = NULL;
  SANE_Status status = SANE_STATUS_GOOD;

  /*  Log at least broken promises and unmet requirements.
   */
  setenv ("SANE_DEBUG_EPKOWA", "FATAL", false);
  msg_init ();

  cache = (list *) model_info_cache_init (getenv ("srcdir"), &status);
  if (!cache)
    {
      err_fatal ("cannot initialise model info cache (%s)",
                 sane_strstatus (status));
      return EXIT_FAILURE;
    }

  while (--argc && ++argv)
    {
      const void *info = model_info_cache_get_info (*argv, &status);
      if (!info)
        {
          err_major ("cannot get info for '%s' (%s)", *argv,
                     sane_strstatus (status));
        }
    }

  pass  = check_cache_content (cache);
  cache = model_info_cache_exit (cache);

  return (pass ? EXIT_SUCCESS : EXIT_FAILURE);
}

/*! \brief  Loops over all cache entries and checks each one of them.
 */
static bool
check_cache_content (list *cache)
{
  bool pass = true;
  _model_info_t *info;

  list_reset (cache);
  while ((info = list_next (cache)))
    {
      pass &= check_info_content (info);
    }
  return pass;
}

/*! \brief  Checks a cache entry against known good data.
 *
 *  \note  Unknown entries are skipped.
 */
static bool
check_info_content (const _model_info_t *info)
{
  if (0 == strcmp_c (info->fw_name, gt_x970.fw_name))
    return model_info_test_cmp (info, &gt_x970);

  if (0 == strcmp_c (info->fw_name, es_h300.fw_name))
    return model_info_test_cmp (info, &es_h300);

  if (0 == strcmp_c (info->fw_name, perfection_610.fw_name))
    return model_info_test_cmp (info, &perfection_610);

  if (0 == strcmp_c (info->fw_name, lp_m5600.fw_name))
    return model_info_test_cmp (info, &lp_m5600);

  if (0 == strcmp_c (info->fw_name, pm_a820.fw_name))
    return model_info_test_cmp (info, &pm_a820);

  if (0 == strcmp_c (info->fw_name, cx_4600.fw_name))
    return model_info_test_cmp (info, &cx_4600);

                                /* cannot do remaining tests */
  printf ("    SKIP: unexpected fw_name (%s)\n", info->fw_name);
  return true;
}

/*! \brief  Compares \a info against a \a reference.
 */
static bool
model_info_test_cmp (const _model_info_t *info,
                     const _model_info_t_test *reference)
{
  bool pass = true;
  int i, j;

  require (info && reference);

  /*  Compare model names  */
  if (0 != strcmp_c (info->overseas, reference->overseas))
    {
      pass = false;
      printf ("FAIL: overseas -> %s != %s\n", info->overseas,
              reference->overseas);
    }

  if (0 != strcmp_c (info->japan, reference->japan))
    {
      pass = false;
      printf ("FAIL: japan -> %s != %s\n", info->japan, reference->japan);
    }

  /*  Compare color profiles  */
  for (i = 0; i < 4; i++)
    {
      for (j = 0; j < 9; j++)
        {
          if (info->profile->color_profile[i][j]
              != reference->profile.color_profile[i][j])
            {
              pass = false;
              printf ("FAIL: profile[%i][%i] -> %f != %f\n", i, j,
                      info->profile->color_profile[i][j],
                      reference->profile.color_profile[i][j]);
            }
        }
    }

  /*  Compare custom command entries  */
  if (info->command->set_focus_position
      != reference->command.set_focus_position)
    {
      pass = false;
      printf ("FAIL: focus -> %d != %d\n", info->command->set_focus_position,
              reference->command.set_focus_position);
    }

  if (info->command->feed != reference->command.feed)
    {
      pass = false;
      printf ("FAIL: feed -> %d != %d\n", info->command->feed,
              reference->command.feed);
    }

  if (info->command->eject != reference->command.eject)
    {
      pass = false;
      printf ("FAIL: eject -> %d != %d\n", info->command->eject,
              reference->command.eject);
    }

  if (info->command->lock != reference->command.lock)
    {
      pass = false;
      printf ("FAIL: lock -> %d != %d\n", info->command->lock,
              reference->command.lock);
    }

  if (info->command->unlock != reference->command.unlock)
    {
      pass = false;
      printf ("FAIL: unlock -> %d != %d\n", info->command->unlock,
              reference->command.lock);
    }

  return pass;
}
