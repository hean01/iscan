/*  device.c -- physical device representation
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


/*! \file
    \brief  Implements a hardware device object.

    The hardware device object is built on top of the ESC/I commands.
    Whereas the commands are only responsible for the I/O details and
    the encoding and decoding of parameters, hardware device objects
    handle the protocol logic and keep state.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>

#include "command.h"
#include "hw-data.h"
#include "utils.h"


/*! Releases all resources associated with a scanner device.
 *
 *  The channel will be closed if open and acquired memory is returned
 *  to the operating system.
 */
device *
dev_dtor (device *hw)
{
  if (!hw) return hw;

  hw->channel = hw->channel->dtor (hw->channel);

  delete (hw->fbf);
  delete (hw->adf);
  delete (hw->tpu);
  delete (hw->fw_name);
  if (hw->res_y.list != hw->res.list
      && hw->res_y.list != hw->res_x.list)
    {
      delete (hw->res_y.list);
    }
  if (hw->res_x.list != hw->res.list)
    {
      delete (hw->res_x.list);
    }
  delete (hw->res.list);
  delete (hw->resolution.list);
  delete (hw);

  return hw;
}

/*!  Updates the extended status of a hardware device object.
 */
SANE_Status
dev_request_extended_status (device *hw)
{
  SANE_Status status = SANE_STATUS_GOOD;

  if (hw->using_fs)
    {
      status = cmd_request_scanner_status (hw);
      if (SANE_STATUS_GOOD != status) return status;

      status = cmd_request_extended_identity (hw);
      if (SANE_STATUS_GOOD != status) return status;

      status = cmd_request_scanner_status (hw);
    }
  else
    status = cmd_request_extended_status (hw);

  return status;
}

#include <string.h>
/*!  Loads a sheet on a page type ADF extension.
 */
SANE_Status
dev_load_paper (device *hw)
{
  SANE_Status status = SANE_STATUS_GOOD;

  if (!hw->adf) return status;  /* guard clauses */
  if (!(ADF_STATUS_IST & hw->adf->status)) return status;
  if (!(ADF_STATUS_EN  & hw->adf->status)) return status;
  hw->adf->sheet_count++;
  if (!(EXT_STATUS_ADFT & hw->ext_status)) return status;

  log_call ();

  status = cmd_load_paper (hw);
  if (SANE_STATUS_GOOD != status)
    {
      hw->adf->sheet_count--;
      return status;
    }
  log_info ("loaded sheet #%d", hw->adf->sheet_count);

  status = dev_request_extended_status (hw);

  if (ADF_STATUS_PE & hw->adf->status
      && adf_early_paper_end_kills_scan (hw))
    {                           /* so we can scan the last sheet */
      cmd_control_option_unit (hw, 0x00);
      hw->adf->status &= ~ ADF_STATUS_EN;
    }

  /* Clear the ADF_STATUS_PE bit.  We just successfully loaded a
     sheet.  Also update the ADF_STATUS_ERR bit.
   */
  hw->adf->status &= ~ADF_STATUS_PE;
  if (   (ADF_STATUS_PE  & hw->adf->status)
      || (ADF_STATUS_PJ  & hw->adf->status)
      || (ADF_STATUS_OPN & hw->adf->status))
    hw->adf->status |=  ADF_STATUS_ERR;
  else
    hw->adf->status &= ~ADF_STATUS_ERR;

  return status;
}

/*! Ejects sheets from the ADF extension.
 */
SANE_Status
dev_eject_paper (device *hw)
{
  SANE_Status status = SANE_STATUS_GOOD;

  if (!hw->adf) return status;  /* guard clauses */
  if (!(ADF_STATUS_IST & hw->adf->status)) return status;
  if (!(ADF_STATUS_EN  & hw->adf->status)) return status;

  log_call ();

  status = cmd_eject_paper (hw);
  hw->adf->sheet_count = 0;

  return status;
}

/*! Open the scanner device. Depending on the connection method, 
 * different open functions are called. 
 */
SANE_Status
dev_open (device *hw)
{
  SANE_Status status = SANE_STATUS_GOOD;

  log_call ();
  require (hw->channel);

  if (hw->channel->is_open (hw->channel))
  {
    log_info ("scanner is already open: fd = %d", hw->channel->fd);
    return SANE_STATUS_GOOD;	/* no need to open the scanner */
  }

  hw->channel->open (hw->channel, &status);

  return status;
}

/*! Log extended (FS W) parameter settings
 */
SANE_Status
dev_log_scanning_parameter (device *hw)
{
  SANE_Status status = SANE_STATUS_GOOD;

  log_call ();

  byte *buf = hw->param_buf; /* for convenience */

  log_info ("SANE_START: Color: %d", (int) buf[24]);
  log_info ("SANE_START: Resolution (x, y): (%u, %u)",
            buf_to_uint32 (buf+0), buf_to_uint32 (buf+4));
  log_info ("SANE_START: Scan offset (x, y): (%d, %d)",
            buf_to_uint32 (buf+8), buf_to_uint32 (buf+12));
  log_info ("SANE_START: Scan size (w, h): (%d, %d)",
            buf_to_uint32 (buf+16), buf_to_uint32 (buf+20));
  log_info ("SANE_START: Data format: %d", (int) buf[25]);
  log_info ("SANE_START: Halftone: %d", (int) buf[32]);
  log_info ("SANE_START: Brightness: %d", (int) buf[30]);
  log_info ("SANE_START: Gamma: %d", (int) buf[29]);
  log_info ("SANE_START: Color correction: %d", (int) buf[31]);
  log_info ("SANE_START: Sharpness control: %d", (int) buf[35]);
  log_info ("SANE_START: Scanning mode: %d", (int) buf[27]);
  log_info ("SANE_START: Mirroring: %d", (int) buf[36]);
  log_info ("SANE_START: Auto area segmentation: %d", (int) buf[34]);
  log_info ("SANE_START: Threshold: %d", (int) buf[33]);
  log_info ("SANE_START: Line counter: %d", (int) buf[28]);
  log_info ("SANE_START: Option unit control: %d", (int) buf[26]);
  log_info ("SANE_START: Film type: %d", (int) buf[37]);

  return status;
}

/*! Obtain the offset and size of the parameter corresponding
 *  to the given ESC command in the FS W packet structure.
 *  Return true if the given command is in the FS W packet
 *  structure and obtaining the parameter info succeeded,
 *  otherwise return false.
 */
static bool
get_extended_param_info (byte cmd, short *offset, short *size)
{
  if (NULL == offset) return false;
  if (NULL == size) return true;

  *offset = 0;
  *size = 1; /* default size of most parameters */

  if ('R' == cmd) { *offset = 0; *size = 8; }
  else if ('A' == cmd) { *offset = 8; *size = 16; }
  else if ('C' == cmd) *offset = 24;
  else if ('D' == cmd) *offset = 25;
  else if ('e' == cmd) *offset = 26;
  else if ('g' == cmd) *offset = 27;
  else if ('d' == cmd) *offset = 28;
  else if ('Z' == cmd) *offset = 29;
  else if ('L' == cmd) *offset = 30;
  else if ('M' == cmd) *offset = 31;
  else if ('B' == cmd) *offset = 32;
  else if ('t' == cmd) *offset = 33;
  else if ('s' == cmd) *offset = 34;
  else if ('Q' == cmd) *offset = 35;
  else if ('K' == cmd) *offset = 36;
  else if ('N' == cmd) *offset = 37;
  else *size = 0;

  if (0 == *size) return false;
  return true;
}

/*! Copies a number of bytes from \a param into the FS W packet buffer.
 *  The size and offset are looked up based on the \a cmd passed.
 */
SANE_Status
dev_set_scanning_parameter (device *hw, byte cmd, const byte* param)
{
  short offset = 0;
  short size = 0;

  log_call ();
  require (hw);
  
  if (NULL == param) return SANE_STATUS_INVAL;
  if (!get_extended_param_info (cmd, &offset, &size)) return SANE_STATUS_INVAL;
  
  memcpy (hw->param_buf + offset, param, size);

  return SANE_STATUS_GOOD;
}

SANE_Status
dev_set_scanning_resolution (device *hw, SANE_Int x_dpi, SANE_Int y_dpi)
{
  byte buf[8];
  uint32_t x;
  uint32_t y;
  
  log_call ();
  require (hw);

  if (x_dpi < 0 || x_dpi > UINT32_MAX) return SANE_STATUS_INVAL;
  if (y_dpi < 0 || y_dpi > UINT32_MAX) return SANE_STATUS_INVAL;

  x = x_dpi;
  y = y_dpi;

  uint32_to_buf (x, buf + 0);
  uint32_to_buf (y, buf + 4);

  return dev_set_scanning_parameter (hw, 'R', buf);
}

SANE_Status
dev_set_scanning_area (device *hw, SANE_Int left, SANE_Int top,
                       SANE_Int width, SANE_Int height)
{
  byte buf[16];
  uint32_t nx;
  uint32_t ny;
  uint32_t nw;
  uint32_t nh;
  
  log_call ();
  require (hw);

  if (left > UINT32_MAX) return SANE_STATUS_INVAL;
  if (top > UINT32_MAX) return SANE_STATUS_INVAL;
  if (width > UINT32_MAX) return SANE_STATUS_INVAL;
  if (height > UINT32_MAX) return SANE_STATUS_INVAL;

  nx = left;
  ny = top;
  nw = width;
  nh = height;

  uint32_to_buf (nx, buf + 0);
  uint32_to_buf (ny, buf + 4);
  uint32_to_buf (nw, buf + 8);
  uint32_to_buf (nh, buf + 12);

  return dev_set_scanning_parameter (hw, 'A', buf);
}

SANE_Status
dev_set_option_unit (device *hw, byte adf_mode)
{
  byte val = 0;

  log_call ();
  require (hw);

  val = (using (hw, fbf) ? 0 : 1);
  if (hw->adf && 1 == val)
    val += adf_mode; /* set simplex/duplex */

  return dev_set_scanning_parameter (hw, 'e', &val);
}

static void
limit_res_list (resolution_info *res, int limit)
{
  int i = 0;
  int new_size = 0;

  for(i=1; i<res->size; i++) // first entry is the list size, skip it
    {
      if (res->list[i] > limit) break;
      ++new_size;
    }

  res->list[0] = new_size;
  res->size = new_size;
  res->last = 0;

  log_info ("Limit resolution to %ddpi", res->list[res->size]);
}

void
dev_limit_res (device *self, SANE_Constraint_Type type, int limit)
{
  if (SANE_CONSTRAINT_RANGE == type)
    {
      self->old_max = self->dpi_range.max;
      self->dpi_range.max = limit;
    }
  else
    {
      self->old_max = self->res.size;
      limit_res_list (&self->res, limit);
      limit_res_list (&self->res_x, limit);
      limit_res_list (&self->res_y, limit);
    }
}

void
dev_restore_res (device *self, SANE_Constraint_Type type)
{
  if (0 == self->old_max) return;

  if (SANE_CONSTRAINT_RANGE == type)
    {
      self->dpi_range.max = self->old_max;
    }
  else
    {
      // res_x and res_y are recreated, only restore res
      self->res.size = self->old_max;
      self->res.list[0] = self->old_max;
      self->res.last = 0;
    }
}

/*! Sends a cancel to the device when not in the middle of retrieving scan data.
    Return true if a cancel command was successfully sent, false, if not.
 */
bool
dev_force_cancel (device *self)
{
  u_char      buf[14];
  u_char      params[2];
  u_char     *dummy = NULL;
  uint32_t    block_size = 0;
  SANE_Status status = SANE_STATUS_GOOD;

  if (!self->using_fs) return false;

  params[0] = FS;
  params[1] = self->cmd->start_scanning;

  channel_send (self->channel, params, 2, &status);
  if (SANE_STATUS_GOOD != status) return false;

  channel_recv (self->channel, buf, num_of (buf), &status);
  if (SANE_STATUS_GOOD != status) return false;
  if (STX != buf[0]) return false;

  block_size = buf_to_uint32 (buf +  2);
  
  dummy = t_malloc (block_size, u_char);
  if (dummy == NULL)
    {
      err_fatal ("%s", strerror (errno));
      return false;
    }

  channel_recv_all (self->channel, dummy,
                    block_size, &status);
  delete (dummy);
  
  if (SANE_STATUS_GOOD != status) return false;

  buf[0] = CAN;
  channel_send (self->channel, buf, 1, &status);
  if (SANE_STATUS_GOOD != status) return false;

  channel_recv (self->channel, buf, 1, &status);
  if (SANE_STATUS_GOOD != status) return false;
  if (ACK != buf[0]) return false;

  return true;
}

SANE_Status
dev_lock (device *hw)
{
  SANE_Status status = SANE_STATUS_GOOD;

  log_call ();
  require (hw);

  if (!hw->uses_locking) return status;
  if ( hw->is_locked)    return status;

  return cmd_lock (hw);
}

SANE_Status
dev_unlock (device *hw)
{
  SANE_Status status = SANE_STATUS_GOOD;

  log_call ();
  require (hw);

  if(!hw->uses_locking) return status;
  if(!hw->is_locked)    return status;

  return cmd_unlock (hw);
}

static SANE_Status
dev_maintenance (device *hw, uint16_t mode)
{
  SANE_Status status = SANE_STATUS_GOOD;

  log_call ("(%04x)", mode);
  require (hw);
  if (!maintenance_is_supported (hw))
  {
    return SANE_STATUS_UNSUPPORTED;
  }
  if (   ESC1_REQ_CLEANING    != mode
      && ESC1_REQ_CALIBRATION != mode)
  {
    return SANE_STATUS_INVAL;
  }

  status = cmd_request_scanner_maintenance (hw, mode);
  if (SANE_STATUS_GOOD == status)
  {
    do
    {
      microsleep (hw->polling_time);
      status = cmd_request_scanner_maintenance (hw, ESC1_REQ_STATUS);
    } while (SANE_STATUS_DEVICE_BUSY == status);
  }

  return status;
}

/* convenience function */
SANE_Status
dev_clean (device *hw)
{
  return dev_maintenance (hw, ESC1_REQ_CLEANING);
}

/* convenience function */
SANE_Status
dev_calibrate (device *hw)
{
  return dev_maintenance (hw, ESC1_REQ_CALIBRATION);
}
