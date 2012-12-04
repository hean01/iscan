/*  cfg-obj.h -- configuration objects
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


#ifndef included_cfg_obj_h
#define included_cfg_obj_h

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdint.h>

#include <sane/sane.h>

#include "list.h"

#ifdef __cplusplus
extern "C"
{
#endif

  extern const char *cfg_file_name;

  typedef enum
    {
      CFG_KEY_NET = 0,
      CFG_KEY_PIO,
      CFG_KEY_SCSI,
      CFG_KEY_USB,

      CFG_KEY_INTERPRETER,
      CFG_KEY_FS_BLACKLIST,

      CFG_KEY_OPTION,

      _CFG_KEY_ID_TERMINATOR_
    }
    cfg_key_id_type;

  typedef const char * cfg_key_type;

  typedef struct
  {
    const char *spec;

  } cfg_net_info;

  typedef struct
  {
    const char *vendor;
    const char *model;

  } cfg_scsi_info;

  typedef struct
  {
    uint16_t vendor;
    uint16_t product;

  } cfg_usb_info;

  typedef struct
  {
    uint16_t vendor;
    uint16_t product;
    const char *library;
    const char *firmware;

  } cfg_interpreter_info;


  void * cfg_init (const char *pkgdatadir, SANE_Status *status);
  void * cfg_exit (void *self);

  void   cfg_find (const void *self, cfg_key_id_type id, list *dev_list);
  list * cfg_seen (const void *self, cfg_key_id_type id);

  bool   cfg_has (const void *self, cfg_key_id_type id);
  void   cfg_set (void *self, cfg_key_id_type id, bool value);

  bool   cfg_has_value (const void *self, cfg_key_id_type id, const char* val);

  cfg_key_type cfg_key (const void *self, cfg_key_id_type id);


#ifdef __cplusplus
}       /* extern "C" */
#endif

#endif  /* !defined (included_cfg_obj_h) */
