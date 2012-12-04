/*  hw-data.h -- selected hardware specific data
 *  Copyright (C) 2008, 2009  SEIKO EPSON CORPORATION
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


#ifndef hw_data_h_included
#define hw_data_h_included

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "device.h"

struct ScannerName
{
  char *overseas;
  char *japan;
};
typedef struct ScannerName scanner_name_t;

struct ScannerData
{
  char *fw_name;
  int profile_ID;
  int command_ID;
  const scanner_name_t name;
};
typedef struct ScannerData scanner_data_t;

#define ILLEGAL_CMD   0xFF

struct EpsonScanCommand
{
  int command_ID;
  unsigned char set_focus_position;
  unsigned char feed;
  unsigned char eject;

  bool lock;
  bool unlock;
};
typedef struct EpsonScanCommand scan_command_t;

struct CapabilityData
{
  char *option;
  char *mode;

  long width;
  long height;
  long base;
};
typedef struct CapabilityData capability_data_t;

char * get_fw_name (channel *ch);

SANE_Bool adf_early_paper_end_kills_scan (const device *hw);
SANE_Bool adf_has_auto_form_feed (const device *hw);
SANE_Bool adf_needs_manual_centering (const device *hw);
SANE_Bool push_button_is_black_listed (const device *hw);
SANE_Int large_res_kills_adf_scan (const device *hw);
SANE_Bool zoom_kills_adf_scan (const device *hw);
SANE_Bool adf_duplex_direction_matches (const device *hw);
SANE_Int autocrop_max_y (const device *hw);
SANE_Bool enable_dip_deskew (const device *hw);
SANE_Bool push_button_needs_polling (const device *hw);
SANE_Bool maintenance_is_supported (const device *hw);

/*! Array with colour correction profiles.

    \todo  Replace this with a data member in the scanner that gets
           initialized with data read from file.
 */
extern const EpsonScanHardRec *epson_scan_hard;

#endif  /* !defined (hw_data_h_included) */
