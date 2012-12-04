/*  device.h -- physical device representation
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


#ifndef device_h_included
#define device_h_included

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include "channel.h"
#include "extension.h"

#define DEVNAME_LENGTH	16


typedef struct
{
  char *level;

  unsigned char request_identity;
  unsigned char request_identity2;	/* new request identity command for Dx command level */
  unsigned char request_status;
  unsigned char request_condition;
  unsigned char set_color_mode;
  unsigned char start_scanning;
  unsigned char set_data_format;
  unsigned char set_resolution;
  unsigned char set_zoom;
  unsigned char set_scan_area;
  unsigned char set_bright;
  SANE_Range bright_range;
  unsigned char set_gamma;
  unsigned char set_halftoning;
  unsigned char set_color_correction;
  unsigned char initialize_scanner;
  unsigned char set_speed;	/* B4 and later */
  unsigned char set_lcount;
  unsigned char mirror_image;	/* B5 and later */
  unsigned char set_gamma_table;	/* B4 and later */
  unsigned char set_outline_emphasis;	/* B4 and later */
  unsigned char set_dither;	/* B4 and later */
  unsigned char set_color_correction_coefficients;	/* B3 and later */
  unsigned char request_extended_status;	/* get extended status from scanner */
  unsigned char control_an_extension;	/* for extension control */
  unsigned char eject;		/* for extension control */
  unsigned char feed;
  unsigned char request_push_button_status;
  unsigned char control_auto_area_segmentation;
  unsigned char set_film_type;	/* for extension control */
  unsigned char set_exposure_time;	/* F5 only */
  unsigned char set_bay;	/* F5 only */
  unsigned char set_threshold;
  unsigned char set_focus_position;	/* B8 only */
  unsigned char request_focus_position;	/* B8 only */
} EpsonCmdRec, *EpsonCmd;

typedef struct
{
  SANE_Int   last;
  SANE_Int   size;
  SANE_Word *list;
  SANE_Bool  deep;
} resolution_info;

typedef struct
{
  int modelID;

  double color_profile[4][9];
} EpsonScanHardRec, *EpsonScanHard;


/*! Software abstraction of the \e physical device.
 */
struct device
{
  struct channel *channel;      /*<! for all device I/O */

  char  cmd_lvl[2+1];           /* +1 for string termination */
  char  version[4+1];           /* +1 for string termination */
  char *fw_name;                /*!< model name reported by firmware */

  SANE_Byte status;             /*!< info block status byte  */
  SANE_Byte ext_status;
  SANE_Byte fsf_status;
  SANE_Byte fsi_cap_1;
  SANE_Byte fsi_cap_2;

  const extension *src;         /*!< current document source */
  fbf_extension   *fbf;         /*!< \c NULL if not available */
  adf_extension   *adf;         /*!< \c NULL if not available */
  tpu_extension   *tpu;         /*!< \c NULL if not available */

  /*! \brief Selectable scan sources.
   *  A \c NULL terminated list of up to three scan sources.  Which
   *  ones are listed depends on device capabilities.
   */
  SANE_String_Const sources[4];

  resolution_info resolution;   /* full-blown list */
  resolution_info res;          /* advertised list */
  SANE_Int max_x;               /* in pixels */
  SANE_Int max_y;

  SANE_Int  optical_res;        /* resolution in pixels per inch */
  SANE_Byte sensor_info;
  SANE_Byte scan_order;
  SANE_Byte line_dist_x;
  SANE_Byte line_dist_y;
  resolution_info res_x;
  resolution_info res_y;


  SANE_Int level;
  SANE_Range dpi_range;

  SANE_Range matrix_range;

  const int       *gamma_type;
  const SANE_Bool *gamma_user_defined;

  const int       *color_type;
  const SANE_Bool *color_user_defined;

  SANE_Bool color_shuffle;	/* does this scanner need color shuffling */
  SANE_Int maxDepth;		/* max. color depth */

  SANE_Int max_line_distance;

  SANE_Bool need_color_reorder;
  SANE_Bool need_reset_on_source_change;

  SANE_Bool wait_for_button;	/* do we have to wait until the scanner button is pressed? */

  SANE_Int doctype;

  unsigned int productID;

  EpsonCmd cmd;
  const EpsonScanHardRec *scan_hard;

  SANE_Bool using_fs; /*! determines whether to use fs commands */

  SANE_Bool block_mode;
  uint32_t  image_block_size;
  uint32_t  final_block_size;
  uint32_t  block_total;        /* does NOT include final block! */
  uint32_t  block_count;

  unsigned char param_buf[64]; /*! holds the FS W parameter buffer */

  SANE_Int scan_width_limit;

  SANE_Int base_res;  /*! resolution at which the max scan area is defined */

  SANE_Int old_max; /*! used to restore after limiting resolutions */

  SANE_Int polling_time;

  bool uses_locking;
  bool is_locked;
};

typedef struct device device;

device *dev_dtor (device *hw);

SANE_Status dev_request_extended_status (device *hw);
SANE_Status dev_load_paper (device *hw);
SANE_Status dev_eject_paper (device *hw);
SANE_Status dev_open (device *hw);

/* FS W/S related functions */
SANE_Status dev_log_scanning_parameter (device *hw);
SANE_Status dev_set_scanning_parameter (device *hw,
                                        byte cmd,
                                        const byte* param);
SANE_Status dev_set_scanning_resolution (device *hw, SANE_Int x_dpi,
                                         SANE_Int y_dpi);
SANE_Status dev_set_scanning_area (device *hw,
                                   SANE_Int left, SANE_Int top,
                                   SANE_Int width, SANE_Int height);
SANE_Status dev_set_option_unit (device *hw, byte adf_mode);

/* other */

/*! supports workarounds to temporarily limit resolution lists */
void dev_limit_res (device *self, SANE_Constraint_Type type, int limit);

/*! restore original resolution list after it has been limited */
void dev_restore_res (device *self, SANE_Constraint_Type type);

bool dev_force_cancel (device *self);

SANE_Status dev_lock (device *hw);
SANE_Status dev_unlock (device *hw);

SANE_Status dev_clean (device *hw);
SANE_Status dev_calibrate (device *hw);

#endif  /* !defined (device_h_included) */
