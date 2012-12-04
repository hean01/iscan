/*  command.h -- assorted ESC/I protocol commands
 *  Copyright (C) 2008--2009  SEIKO EPSON CORPORATION
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


#ifndef command_h_included
#define command_h_included

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


/* ESC/I protocol constants.
 */

/* Readability definitions for selected ASCII codes used by the ESC/I
   protocol.
 */
#define NUL                     0x00    /* '\0' */
#define STX                     0x02
#define ACK                     0x06
#define BUSY                    0x07    /* BEL in ascii(7) */
#define FF                      0x0c    /* '\f' */
#define NAK                     0x15
#define CAN                     0x18
#define PF                      0x19    /* EM in ascii(7) */
#define ESC                     0x1b
#define FS                      0x1c


/* Bit flags for the status byte in the information block.
 */
#define STATUS_FATAL_ERROR      0x80
#define STATUS_NOT_READY        0x40    /* in use via other interface */
#define STATUS_AREA_END         0x20    /* scan finished */
#define STATUS_OPTION           0x10    /* option detected */
#define STATUS_COLOR_ATTR_MASK  0x0c
#define STATUS_EXT_COMMANDS     0x02    /* FS commands supported */

/* Bit flags for the extended image acquisition (FS G) error byte
 */
#define FSG_FATAL_ERROR         0x80
#define FSG_NOT_READY           0x40    /* in use via other interface */
#define FSG_PAGE_END            0x20    /* notify of the paper end */
#define FSG_CANCEL_REQUEST      0x10    /* cancel request from scanner */

/* Bit flags for selected bytes in the extended status and scanner
   status replies.  Synchronised status flags are defined in terms
   of the corresponding flag.  All other values are supposed to be
   unlinked.  The various *_ERR flags indicate that at least one of
   the other error bit flags in the _same_ block is set.

   The EXT_STATUS_* flags are specific to the ESC f command's reply,
   The FSF_STATUS_*, ADF_EXT_STATUS_* and AFL_STATUS_* flags to that
   of the FS F command.
 */
#define EXT_STATUS_FER          0x80    /* fatal error */
#define EXT_STATUS_NO_FBF       0x40    /* no flat bed */
#define EXT_STATUS_ADFT         0x20    /* ADF unit type */
#define EXT_STATUS_ADFS         0x10    /* simplex/duplex */
#define EXT_STATUS_ADFO         0x08    /* feed from first/last sheet */
#define EXT_STATUS_LID          0x04    /* lid type option */
#define EXT_STATUS_WU           0x02    /* warming up */
#define EXT_STATUS_PB           0x01    /* scanner has a push button */

#define FSF_STATUS_FER          0x80    /* fatal error */
#define FSF_STATUS_NR           0x40    /* in use via other interface */
#define FSF_STATUS_WU           0x02    /* warming up */
#define FSF_STATUS_CWU          0x01    /* can cancel warming up */

#define ADF_STATUS_IST          0x80    /* option detected */
#define ADF_STATUS_EN           0x40    /* option enabled */
#define ADF_STATUS_ERR          0x20    /* option error detected */
#define ADF_STATUS_ATYP         0x10    /* photo ADF type detected */
#define ADF_STATUS_PE           0x08    /* no paper */
#define ADF_STATUS_PJ           0x04    /* paper jam */
#define ADF_STATUS_OPN          0x02    /* cover open */
#define ADF_STATUS_PAG          0x01    /* duplex selected */

#define ADF_EXT_STATUS_IST      ADF_STATUS_IST
#define ADF_EXT_STATUS_EN       ADF_STATUS_EN
#define ADF_EXT_STATUS_ERR      0x20    /* option error detected */
#define ADF_EXT_STATUS_DFE      0x10    /* double feed */
#define ADF_EXT_STATUS_TR_OPN   0x02    /* tray open */

#define TPU_STATUS_IST          0x80    /* option detected */
#define TPU_STATUS_EN           0x40    /* option enabled */
#define TPU_STATUS_ERR          0x20    /* option error detected */
#define TPU_STATUS_OPN          0x02    /* cover open */
#define TPU_STATUS_LTF          0x01    /* lamp luminescence too low */

#define AFL_STATUS_IST          0x80    /* option detected */
#define AFL_STATUS_EN           0x40    /* option enabled */
#define AFL_STATUS_ERR          0x20    /* option error detected */
#define AFL_STATUS_OPN          0x02    /* cover open */
#define AFL_STATUS_LTF          0x01    /* lamp luminescence too low */

#define DV3_STATUS_ERR          0x20
#define DV3_STATUS_PE           0x08    /* no paper */
#define DV3_STATUS_PJ           0x04    /* paper jam */
#define DV3_STATUS_OPN          0x02    /* cover open */

/* Bit flags
 */
#define FSI_CAP_DLF             0x80    /* ??? */
#define FSI_CAP_NO_FBF          0x40    /* no flat bed */
#define FSI_CAP_ADFT            0x20    /* ADF unit type */
#define FSI_CAP_ADFS            0x10    /* simplex/duplex */
#define FSI_CAP_ADFO            0x08    /* feed from first/last sheet */
#define FSI_CAP_LID             0x04    /* lid type option */
#define FSI_CAP_TPIR            0x02    /* TPU with IR support */
#define FSI_CAP_PB              0x01    /* scanner has a push button */

#define FSI_CAP_ADFAS           0x10    /* ADF with auto scan support */
#define FSI_CAP_DFD             0x08    /* double feed detection */
#define FSI_CAP_AFF             0x04    /* auto form feed */
#define FSI_CAP_ESST            0x02    /* ??? */
#define FSI_CAP_PED             0x01    /* paper end detection support */

/* Maintenance requests.
 */
#define ESC1_REQ_CLEANING     0x0001
#define ESC1_REQ_CALIBRATION  0x0002
#define ESC1_REQ_STATUS       0xffff

#include "device.h"


/* Getter commands.
 */
SANE_Status cmd_request_identity (device *hw);
SANE_Status cmd_request_hardware_property (device *hw);
SANE_Status cmd_request_scanner_status (device *hw);
SANE_Status cmd_request_extended_identity (device *hw);
SANE_Status cmd_request_extended_status (device *hw);
SANE_Status cmd_request_scanning_parameter (device* hw);


/* Setter commands.
 */
SANE_Status cmd_control_option_unit (device *hw, byte value);
SANE_Status cmd_set_scanning_parameter (device* hw);


/* Action commands.
 */
SANE_Status cmd_initialize (device *hw);
SANE_Status cmd_load_paper (device *hw);
SANE_Status cmd_eject_paper (device *hw);
SANE_Status cmd_lock (device *hw);
SANE_Status cmd_unlock (device *hw);
SANE_Status cmd_request_scanner_maintenance (device *hw, uint16_t mode);


#endif  /* !defined (command_h_included) */
