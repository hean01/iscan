/*  hw-data.c -- selected hardware specific data
 *  Copyright (C) 2004--2009  SEIKO EPSON CORPORATION
 *
 *  License: GPLv2+|iscan
 *  Authors: AVASYS CORPORATION
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

#include "hw-data.h"

#include <ctype.h>
#include <locale.h>
#include <stddef.h>
#include <string.h>
#include <time.h>


#include "command.h"

#include "utils.h"

/* Not all scanners report the model name that is printed on the case.
   Here we try to adjust for that by second guessing the name received
   from the hardware with the aid of some environmental data.
   WARNING: this is not and can not be foolproof.

   The caller gets to manage the memory occupied by the string that is
   returned.
 */

/*! Returns a copy of the F/W name of the device on the other end of a channel.
 */
char *
get_fw_name (channel *ch)
{
  SANE_Status status = SANE_STATUS_GOOD;
  const byte cmd[] = { ESC, 'f' };
  char *fw_name = NULL;

  if (!ch) return NULL;

  channel_send (ch, cmd, num_of (cmd), &status);
  if (SANE_STATUS_GOOD == status)
    {
      byte info[4];

      channel_recv (ch, info, num_of (info), &status);
      if (SANE_STATUS_GOOD == status)
        {
          char reply[42+1];   /* extra byte to enable string
                                 termination */
          channel_recv (ch, reply, num_of (reply) - 1, &status);
          if (SANE_STATUS_GOOD == status)
            {
              char *lc_ctype = setlocale (LC_CTYPE, "C");
              size_t n = num_of (reply);

              do
                {
                  --n, reply[n] = '\0';
                }
              while (26 < n && (isspace (reply[n-1]) || '\0' == reply[n-1]));
                                             /* workaround for non-compliant
                                                interpreter firmware names that
                                                are padded with NULs and spaces
                                              */
              fw_name = strdup (reply + 26);
              setlocale (LC_CTYPE, lc_ctype);
            }
        }
    }
  if (SANE_STATUS_GOOD != status)
    err_minor ("%s", sane_strstatus (status));

  /* Devices with USB product IDs 0x085C and 0x0883 may report the
   * same firmware name.  That breaks our data file finding scheme
   * so we fix up the firmware name here to be spec compliant.
   */
  if (0 == strcmp_c ("PID 085C", fw_name)
      && (CHAN_USB == ch->type && 0x0883 == ch->id))
    {
      strcpy (fw_name, "PID 0883");
    }

  return fw_name;
}

static SANE_Bool
_is_listed (const char *needle, const char **haystack)
{
  if (!needle || !haystack) return SANE_FALSE;

  while (*haystack)
    {
      if (0 == strcmp_c (needle, *haystack))
        return SANE_TRUE;
      ++haystack;
    }
  return SANE_FALSE;
}


SANE_Bool
adf_needs_manual_centering (const device *hw)
{
  const char *fw_names[] = {
    "LP-M6000",
    "LP-M5000",
    "LP-M5300",
    "LP-M8040",
    "ES-H300",
    "CX9400Fax",
    "PID 087C",
    "GT-S80",
    "GT-S50",
    "GT-S85",
    "GT-S55",
    NULL
  };

  require (using (hw, adf));
  return _is_listed (hw->fw_name, fw_names);
}


SANE_Bool
adf_has_auto_form_feed (const device *hw)
{
  const char *fw_names[] = {
    "LP-M6000",
    "LP-M5000",
    "LP-M5300",
    NULL
  };

  require (using (hw, adf));
  return _is_listed (hw->fw_name, fw_names)
         || (FSI_CAP_AFF & hw->fsi_cap_2);
}

SANE_Bool
adf_early_paper_end_kills_scan (const device *hw)
{
  const char *fw_names[] = {
    "ES-10000G",
    "ES-7000H",
    "ES-H7200",
    "Expression10000",
    "GT-20000",
    NULL,
  };

  require (using (hw, adf));
  return _is_listed (hw->fw_name, fw_names);
}

SANE_Bool
push_button_is_black_listed (const device *hw)
{
  const char *fw_names[] = {
    "LP-M6000",
    "LP-M5000",
    "LP-M5300",
    NULL,
  };

  // whitelist of scanners that support push button via the network
  const char *fw_names_net[] = {
    "ES-H7200",
    "GT-20000",
    NULL,
  };


  return _is_listed (hw->fw_name, fw_names)
         || hw->uses_locking
         || (CHAN_NET == hw->channel->type &&
             !_is_listed (hw->fw_name, fw_names_net));
}

SANE_Int
large_res_kills_adf_scan (const device *hw)
{
  const char *fw_names1[] = {
    "ES-H300",
    "LP-M6000",
    "LP-M5000",
    "LP-M5300",
    "LP-M8040",
    NULL,
  };

  const char *fw_names2[] = {
    "NX300",
    "WorkForce 600",
    "Artisan 800",
    NULL,
  };

  require (hw->adf);
  if (_is_listed (hw->fw_name, fw_names1)) return 600;
  if (_is_listed (hw->fw_name, fw_names2)) return 1200;

  return 0;
}
SANE_Bool
zoom_kills_adf_scan (const device *hw)
{
  const char *fw_names[] = {
    "LP-M6000",
    "LP-M5000",
    "LP-M5300",
    NULL,
  };

  require (hw->adf);
  return _is_listed (hw->fw_name, fw_names);
}

/* Say whether duplex scans scan front and back sides in the same
 * direction.  This is definitely the case for single pass duplex
 * devices (which scan front and back simultaneously) but is only
 * seldomly encountered with double pass duplexers (which scan the
 * front and back one after another).
 */
SANE_Bool
adf_duplex_direction_matches (const device *hw)
{
  const char *fw_names[] = {
    "GT-S80",
    "GT-S50",
    "GT-S85",
    "GT-S55",
    NULL,
  };

  return hw->adf && _is_listed (hw->fw_name, fw_names);
}

/*! Return an override for the current source's max_y value (in pixels)
 *  to be used with autocropping.  In case no override is needed, zero
 *  is returned.
 *
 *  A number of models support scanning a slightly taller document than
 *  the firmware would have you believe.  This "feature" is referred to
 *  as overscanning and used to minimize the risks of chopping off bits
 *  from skewed originals when performing autocropping.
 *
 *  \todo  Allow for model specific max_y values.
 */
SANE_Int
autocrop_max_y (const device *hw)
{
  SANE_Int rv = 0;

  const char *fw_names[] = {
    "GT-S80",
    "GT-S50",
    "GT-S85",
    "GT-S55",
    NULL,
  };

  if (_is_listed (hw->fw_name, fw_names))
    {
      if (using (hw, adf))
        {
          rv = 15 * hw->base_res;
        }
    }
  return rv;
}

/* Restrict functionality to tested devices only.
 */
SANE_Bool
enable_dip_deskew (const device *hw)
{
  const char *fw_names[] = {
    "GT-S80",
    "GT-S50",
    "GT-S85",
    "GT-S55",
    NULL,
  };

  return _is_listed (hw->fw_name, fw_names);
}

SANE_Bool
push_button_needs_polling (const device *hw)
{
  const char *fw_names[] = {
    "DS-30",
    NULL,
  };

  return _is_listed (hw->fw_name, fw_names);
}

SANE_Bool
maintenance_is_supported (const device *hw)
{
  const char *fw_names[] = {
    "DS-30",
    NULL,
  };

  return _is_listed (hw->fw_name, fw_names);
}
