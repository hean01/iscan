/*  model-info.h -- per model information objects and cache
 *  Copyright (C) 2010  SEIKO EPSON CORPORATION
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


#ifndef model_info_h_included
#define model_info_h_included

/*! \file
 *  \brief  Model specific information.
 *
 *  There is a fair bit of device information that does not change
 *  during the life-time of the device.  The ESC/I scanner protocol
 *  provides support to query the device to get a significant part of
 *  this information.  However, some of the information we would like
 *  to use is not available from the device.  In addition, the device
 *  sometimes returns incorrect information.
 *
 *  For a long time we have been working around this by putting the
 *  additional information and the corrections directly in our code.
 *  The model info support provided here implements a unified API to
 *  the device information allowing us to move it out of the code and
 *  into per model resource files.
 *
 *  The API aims for on-demand, cached data retrieval.  In order to
 *  support that, the sane_init() and sane_exit() functions need to
 *  handle cache initialisation and clean-up.  They can do so through
 *  the model_info_cache_init() and model_cache_exit() functions.
 *
 *  The sane_get_devices() and sane_open() functions can get access to
 *  per model information through the model_info_cache_get_info() API
 *  and use its return value in calls to the model info accessors.  Of
 *  course, other SANE API entries may do so as well, though we cannot
 *  think of a good need for them to do so (as the information can be
 *  stored in the SANE_Handle during sane_open()).
 *
 *  The number of model info accessors is still quite limited but we
 *  expect that to change when the implementation proceeds.
 *
 *  There is also convenience API meant to make initialisation of the
 *  SANE_Device structure marginally less verbose.
 */

#include <sane/sane.h>

#include "defines.h"
#include "device.h"

#ifdef __cplusplus
extern "C"
{
#endif

  struct ScanArea
  {
    SANE_Fixed width;
    SANE_Fixed height;
  };
  typedef struct ScanArea scan_area_t;

  /* Model info cache creation and destruction */
  void * model_info_cache_init (const char *pkgdatadir, SANE_Status *status);
  void * model_info_cache_exit (void *self);

  /* Model info cache accessors */
  const void * model_info_cache_get_info (const char *fw_name,
                                          SANE_Status *status);

  /* Model info cache convenience methods */
  char * model_info_cache_get_model (const char *fw_name);
  /* ?FIXME? add convenience methods for vendor and type? */

  /* Model info accessors */
  const char * model_info_get_name (const void *self);
  const EpsonScanHard model_info_get_profile (const void *self);
  bool model_info_customise_commands (const void *self, EpsonCmd cmd);
  bool model_info_has_lock_commands (const void *self);

  scan_area_t model_info_max_scan_area(const void *self, const char *option, const char *mode);
  /* :FIXME: add more accessors */

#ifdef __cplusplus
}       /* extern "C" */
#endif

#endif  /* !defined (model_info_h_included) */
