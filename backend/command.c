/*  command.c -- assorted ESC/I protocol commands
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


/*! \file
    \brief  Implements 'bare' ESC/I commands.

    This file contains functions that implement of a number of ESC/I
    commands.  These functions only handle the sending and receiving
    of data, data interpretation and I/O or memory related errors.

    Interpretation of received data is limited to taking bytes apart
    into bit flags and combining bytes into larger entities, such as
    2- or 4-byte integers and strings.  The hardware device object's
    state is updated to correspond with the interpreted data.

    Under \e no circumstances shall the functions implemented in this
    file draw any conclusions whatsoever as to the device's resulting
    state.  This is the reponsibility of the hardware device object.
    Also, it is the hardware device object's responsibility to check
    the timing/validity of calling any of the functions implemented
    here.

    All functions shall return a status that is one of:

     - \c SANE_STATUS_GOOD
     - \c SANE_STATUS_NO_MEM
     - \c SANE_STATUS_IO_ERROR
     - \c SANE_STATUS_INVAL

    Function names follow the description in the ESC/I specification
    and shall start with a \c cmd_ prefix.

    \note  I want to replace the device::cmd crap with function
    pointers and use a no-op for commands that are not supported by
    the device.  Functions may return SANE_STATUS_UNSUPPORTED in the
    interim.

    \note  I also want to introduce a function pointer based callback
    mechanism to fix up broken firmware replies.  That way, we do not
    have to pass the firmware name everytime.  We can set appropriate
    callbacks for a device once and be done with it.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "command.h"

#include "utils.h"

#include <string.h>

#define SANE_INT_MAX 2147483647


static void
fixme_request_identity (const char *fw_name, byte *buf, size_t size)
{
  if (!fw_name) return;
  if (!buf) return;

  if (0 == strcmp_c ("NX100", fw_name) && 16 < size)
    {
      buf[12] = 'A';
      buf[13] = 0xEC;
      buf[14] = 0x13;
      buf[15] = 0x6C;
      buf[16] = 0x1B;
    }
}

/*! \brief  Establishes basic device capabilities.

    \note  This command is assumed to be supported by \e all ESC/I
           devices.

    \todo  Implement error checking.
 */
SANE_Status
cmd_request_identity (device *hw)
{
  SANE_Status status = SANE_STATUS_GOOD;

  const byte cmd[] = { ESC, 'I' };
  byte info[4];

  byte  *data = NULL;
  size_t size;

  log_call ();
  require (hw);

  channel_send (hw->channel, cmd, num_of (cmd), &status);
  channel_recv (hw->channel, info, num_of (info), &status);

  hw->status = info[1];
  size = info[3] << 8 | info[2];

  if (0 < size)
    {
      data = t_calloc (size, byte);
      if (!data)
        {
          return SANE_STATUS_NO_MEM;
        }

      channel_recv (hw->channel, data, size, &status);
      if (SANE_STATUS_GOOD == status)
        {
          byte *p = data + 2;

          fixme_request_identity (hw->fw_name, data, size);

          hw->cmd_lvl[0] = data[0];
          hw->cmd_lvl[1] = data[1];

          free_resolution_info (&hw->res);
          init_resolution_info (&hw->res, p);
          init_resolution_info (&hw->resolution, NULL);
          copy_resolution_info (&hw->resolution, &hw->res, SANE_TRUE);

          hw->max_x = data[size-3] << 8 | data[size-4];
          hw->max_y = data[size-1] << 8 | data[size-2];
        }
      delete (data);
    }

  return status;
}

static void
fixme_request_hardware_property (const char *fw_name, byte *buf, size_t size)
{
  if (!fw_name) return;
  if (!buf) return;

  if (0 == strcmp_c ("NX100", fw_name) && 33 < size)
    {
      buf[32] = 0xB0;
      buf[33] = 0x04;
    }
}

/*! \brief  Query additional device capabilities.

    \note  This command is not supported for B level devices.  It is
           supported for D level devices.

    \todo  Implement error checking.
 */
SANE_Status
cmd_request_hardware_property (device *hw)
{
  SANE_Status status = SANE_STATUS_GOOD;

  const byte cmd[] = { ESC, 'i' };
  byte info[4];

  byte  *data = NULL;
  size_t size;

  log_call ();
  require (hw);

  channel_send (hw->channel, cmd, num_of (cmd), &status);
  channel_recv (hw->channel, info, num_of (info), &status);

  hw->status = info[1];
  size = info[3] << 8 | info[2];

  if (0 < size)
    {
      data = t_calloc (size, byte);
      if (!data)
        {
          return SANE_STATUS_NO_MEM;
        }

      channel_recv (hw->channel, data, size, &status);
      if (SANE_STATUS_GOOD == status)
        {
          byte *p = data + 14;

          fixme_request_hardware_property (hw->fw_name, data, size);

          hw->optical_res = data[1] << 8 | data[0];
          hw->sensor_info = data[2];
          hw->scan_order  = data[3];
          hw->line_dist_x = data[4];
          hw->line_dist_y = data[5];

          free_resolution_info (&hw->res_x);
          init_resolution_info (&hw->res_x, p);

          while (resolution_info_ESC_i_cond (p))
            p += 2;

          p += 2;               /* start of sub resolution info */
          free_resolution_info (&hw->res_y);
          init_resolution_info (&hw->res_y, p);
        }
      delete (data);
    }

  return status;
}

SANE_Status
cmd_set_scanning_parameter (device* hw)
{
  SANE_Status status = SANE_STATUS_GOOD;
  
  const byte FS_W[] = {FS, 'W'};

  byte ack_buf;

  log_call ();
  require (hw);

  channel_send (hw->channel, FS_W, 2, &status);
  if (SANE_STATUS_GOOD != status) return status;
  channel_recv (hw->channel, &ack_buf, 1, &status);
  if (SANE_STATUS_GOOD != status) return status;
  if (ACK != ack_buf) return SANE_STATUS_UNSUPPORTED;
  
  channel_send (hw->channel, hw->param_buf, 64, &status);
  if (SANE_STATUS_GOOD != status) return status;
  channel_recv (hw->channel, &ack_buf, 1, &status);
  if (SANE_STATUS_GOOD != status) return status;
  if (ACK != ack_buf) return SANE_STATUS_INVAL;
  
  return status;
}

SANE_Status
cmd_request_scanning_parameter (device* hw)
{
  SANE_Status status = SANE_STATUS_GOOD;
  const byte FS_S[] = {FS, 'S'};
  
  log_call ();
  require (hw);

  channel_send (hw->channel, FS_S, 2, &status);
  if (SANE_STATUS_GOOD != status) return status;
  channel_recv (hw->channel, hw->param_buf, 64, &status);

  return status;
}

SANE_Status
cmd_request_scanner_status (device *hw)
{
  SANE_Status status = SANE_STATUS_GOOD;

  const byte cmd[] = { FS, 'F' };
  byte buf[16];

  log_call ();
  require (hw);

  channel_send (hw->channel, cmd, num_of (cmd), &status);
  channel_recv (hw->channel, buf, num_of (buf), &status);

  hw->fsf_status = buf[0];
  {                             /* transfer corresponding flags */
    byte mask = FSF_STATUS_FER | FSF_STATUS_WU;
    hw->ext_status &= ~mask;
    hw->ext_status |= (mask & hw->fsf_status);
  }

  if ((ADF_STATUS_IST & buf[1]) && !hw->adf)
    {
      hw->adf = t_calloc (1, adf_extension);
      if (!hw->adf) return SANE_STATUS_NO_MEM;
    }
  if ((TPU_STATUS_IST & buf[2]) && !hw->tpu)
    {
      hw->tpu = t_calloc (1, tpu_extension);
      if (!hw->tpu) return SANE_STATUS_NO_MEM;
    }

  if (hw->fbf)
    {
      hw->fbf->status = buf[3];
      update_doc_size (hw->fbf, buf[8] << 8 | buf[7]);
    }
  if (hw->adf)
    {
      hw->adf->status = buf[1];
      hw->adf->ext_status = buf[10];
      update_doc_size (hw->adf, buf[6] << 8 | buf[5]);
    }
  if (hw->tpu)
    {
      hw->tpu->status = buf[2];
    }

  return SANE_STATUS_GOOD;
}

static SANE_Int
buf_to_sane_int (const byte *p, const char *var)
{
  SANE_Int result = 0;

  require (p);
  result = buf_to_uint32 (p);

  if (SANE_INT_MAX < result)
    {
      err_major ("overflow: %s", var);
      result = SANE_INT_MAX;
    }
  return result;
}

SANE_Status
cmd_request_extended_identity (device *hw)
{
  SANE_Status status = SANE_STATUS_GOOD;

  const byte cmd[] = { FS, 'I' };
  byte buf[80];

  log_call ();
  require (hw);

  channel_send (hw->channel, cmd, num_of (cmd), &status);
  channel_recv (hw->channel, buf, num_of (buf), &status);

  hw->cmd_lvl[0] = buf[0];
  hw->cmd_lvl[1] = buf[1];

  hw->version[0] = buf[62];
  hw->version[1] = buf[63];
  hw->version[2] = buf[64];
  hw->version[3] = buf[65];

  hw->fsi_cap_1 = buf[44];
  hw->fsi_cap_2 = buf[45];
  {                             /* transfer corresponding flags */
    byte mask = (EXT_STATUS_NO_FBF | EXT_STATUS_ADFT | EXT_STATUS_ADFS
                 | EXT_STATUS_ADFO | EXT_STATUS_LID | EXT_STATUS_PB);
    hw->ext_status &= ~mask;
    hw->ext_status |= (mask & hw->fsi_cap_1);
  }

  if (!(EXT_STATUS_NO_FBF & hw->fsi_cap_1) && !hw->fbf)
    {
      hw->fbf = t_calloc (1, fbf_extension);
      if (!hw->fbf) return SANE_STATUS_NO_MEM;
    }

  hw->cmd->request_push_button_status
    = ((EXT_STATUS_PB & hw->fsi_cap_1) ? '!' : 0);

  hw->base_res = buf_to_sane_int (buf + 4, "base resolution");

  hw->dpi_range.min = buf_to_sane_int (buf +  8, "minimum resolution");
  hw->dpi_range.max = buf_to_sane_int (buf + 12, "maixmum resolution");
  hw->dpi_range.quant = 1;

  hw->scan_width_limit = buf_to_sane_int (buf + 16, "scan width");

  if (hw->fbf)
    {
      hw->fbf->max_x = buf_to_sane_int (buf + 20, "fbf max_x");
      hw->fbf->max_y = buf_to_sane_int (buf + 24, "fbf max_y");
      update_ranges (hw, hw->fbf);
    }
  if (hw->adf)
    {
      hw->adf->max_x = buf_to_sane_int (buf + 28, "adf max_x");
      hw->adf->max_y = buf_to_sane_int (buf + 32, "adf max_y");
      update_ranges (hw, hw->adf);
    }
  if (hw->tpu)
    {
      hw->tpu->max_x = buf_to_sane_int (buf + 36, "tpu max_x");
      hw->tpu->max_y = buf_to_sane_int (buf + 40, "tpu max_y");
      update_ranges (hw, hw->tpu);
    }

  return SANE_STATUS_GOOD;
}

static void
fixme_request_extended_status (const char *fw_name, byte *buf, size_t size)
{
  if (!fw_name) return;
  if (!buf) return;

  if (0 == strcmp_c ("GT-8200", fw_name) && 15 < size)
    {
      uint16_t max_x;
      uint16_t max_y;

      max_x = buf[13] << 8 | buf[12];
      max_y = buf[15] << 8 | buf[14];
      if (max_y < max_x)
        {
          err_minor ("Fixing up buggy FBF max scan dimensions.");
          max_y *= 2;
          buf[14] = 0xFF &  max_y;
          buf[15] = 0xFF & (max_y >> 8);
        }

      max_x = buf[ 8] << 8 | buf[7];
      max_y = buf[10] << 8 | buf[9];
      if (max_y < max_x)
        {
          err_minor ("Fixing up buggy TPU max scan dimensions.");
          max_y *= 2;
          buf[ 9] = 0xFF &  max_y;
          buf[10] = 0xFF & (max_y >> 8);
        }
    }

  if ((0 == strcmp_c ("ES-9000H", fw_name) ||
       0 == strcmp_c ("GT-30000", fw_name))
      && 5 < size)
    {
      err_minor ("Fixing up buggy ADF max scan dimensions.");
      buf[2] = 0xB0;
      buf[3] = 0x6D;
      buf[4] = 0x60;
      buf[5] = 0x9F;
    }
}

/*!  Updates the extended status of a hardware device object.
 */
SANE_Status
cmd_request_extended_status (device *hw)
{
  SANE_Status status = SANE_STATUS_GOOD;

  const byte cmd[] = { ESC, 'f' };
  byte info[4];

  byte *data = NULL;
  size_t size;

  const size_t DEVNAME_OFFSET = 26;

  const byte DEVT_MASK = 0xC0;
  const byte DEVTYPE_3 = 0xC0;

  log_call ();
  require (hw);

  if (!hw->cmd->request_extended_status)
    return SANE_STATUS_UNSUPPORTED;

  channel_send (hw->channel, cmd, num_of (cmd), &status);
  channel_recv (hw->channel, info, num_of (info), &status);

  hw->status = info[1];
  size = info[3] << 8 | info[2];

  require (DEVNAME_OFFSET + DEVNAME_LENGTH <= size);

  if (0 < size)
    {
      data = t_calloc (size, byte);
      if (!data)
        {
          return SANE_STATUS_NO_MEM;
        }

      channel_recv (hw->channel, data, size, &status);
      if (SANE_STATUS_GOOD == status)
        {
          fixme_request_extended_status (hw->fw_name, data, size);

          hw->ext_status = data[0];

          hw->cmd->request_push_button_status
            = ((EXT_STATUS_PB & data[0]) ? '!' : 0);

          if (!(EXT_STATUS_NO_FBF & data[0]) && !hw->fbf)
            {
              hw->fbf = t_calloc (1, fbf_extension);
              if (!hw->fbf) status = SANE_STATUS_NO_MEM;
            }
          if ((ADF_STATUS_IST & data[1]) && !hw->adf)
            {
              hw->adf = t_calloc (1, adf_extension);
              if (!hw->adf) status = SANE_STATUS_NO_MEM;
            }
          if ((TPU_STATUS_IST & data[6]) && !hw->tpu)
            {
              hw->tpu = t_calloc (1, tpu_extension);
              if (!hw->tpu) status = SANE_STATUS_NO_MEM;
            }

          if (hw->fbf)
            {
              hw->fbf->status = 0x00;
              if (DEVTYPE_3 == (DEVT_MASK & data[11]))
                {
                  hw->fbf->status = data[11];
                  hw->fbf->max_x = data[13] << 8 | data[12];
                  hw->fbf->max_y = data[15] << 8 | data[14];
                }
              else
                {
                  hw->fbf->max_x = hw->max_x;
                  hw->fbf->max_y = hw->max_y;
                }
              update_ranges (hw, hw->fbf);
              update_doc_size (hw->fbf, data[19] << 8 | data[18]);
            }
          if (hw->adf)
            {
              hw->adf->status = data[1];
              hw->adf->max_x = data[3] << 8 | data[2];
              hw->adf->max_y = data[5] << 8 | data[4];
              update_ranges (hw, hw->adf);
              update_doc_size (hw->adf, data[17] << 8 | data[16]);
            }
          if (hw->tpu)
            {
              hw->tpu->status = data[6];
              hw->tpu->max_x = data[ 8] << 8 | data[7];
              hw->tpu->max_y = data[10] << 8 | data[9];
              update_ranges (hw, hw->tpu);
            }
        }
      delete (data);
    }

  return status;
}


/*! Sets the option unit to use as well as the unit's behaviour.

    \todo  Implement error checking.
 */
SANE_Status
cmd_control_option_unit (device *hw, byte value)
{
  SANE_Status status = SANE_STATUS_GOOD;

  const byte cmd[] = { ESC, 'e' };
  byte reply = NUL;

  log_call ();
  require (hw);

  channel_send (hw->channel, cmd, num_of (cmd), &status);
  channel_recv (hw->channel, &reply, 1, &status);
  channel_send (hw->channel, &value, 1, &status);
  channel_recv (hw->channel, &reply, 1, &status);

  return status;
}


/*! Resets the device to a well known state.

    \todo  Implement error checking.
 */
SANE_Status
cmd_initialize (device *hw)
{
  SANE_Status status = SANE_STATUS_GOOD;

  const byte cmd[] = { ESC, '@' };
  byte reply = NUL;

  log_call ();
  require (hw);

  channel_send (hw->channel, cmd, num_of (cmd), &status);
  channel_recv (hw->channel, &reply, 1, &status);

  return status;
}

/*! Loads a sheet on a page type ADF extension.

    \todo  Implement error checking.
 */
SANE_Status
cmd_load_paper (device *hw)
{
  SANE_Status status = SANE_STATUS_GOOD;

  const byte cmd[] = { PF };
  byte reply = NUL;

  log_call ();
  require (hw);

  channel_send (hw->channel, cmd, num_of (cmd), &status);
  channel_recv (hw->channel, &reply, 1, &status);

  return status;
}

/*! Ejects sheets from the ADF extension.

    \todo  Implement error checking.
 */
SANE_Status
cmd_eject_paper (device *hw)
{
  SANE_Status status = SANE_STATUS_GOOD;

  const byte cmd[] = { FF };
  byte reply = NUL;

  log_call ();
  require (hw);

  channel_send (hw->channel, cmd, num_of (cmd), &status);
  channel_recv (hw->channel, &reply, 1, &status);

  return status;
}

SANE_Status
cmd_lock (device *hw)
{
  SANE_Status status = SANE_STATUS_GOOD;

  const byte cmd[] = { ESC, '(' };
  byte reply = NUL;

  log_call();
  require (hw);

  channel_send (hw->channel, cmd, num_of (cmd), &status);
  if (SANE_STATUS_GOOD != status) return status;
  channel_recv (hw->channel, &reply, 1, &status);

  if (SANE_STATUS_GOOD == status)
    {
      if (0x80 == reply)
        {
          hw->is_locked = true;
        }
      else if (0x40 == reply)
        {
          err_minor ("failed to acquire lock");
          status = SANE_STATUS_DEVICE_BUSY;
        }
      else if (NAK == reply)
        {
          err_minor ("locking not supported by device, disabling");
          hw->uses_locking = false;
          status = SANE_STATUS_GOOD;
        }
      else
        {
          err_major ("unexpected reply to lock command (%02x)", reply);
          status = SANE_STATUS_IO_ERROR;
        }
    }
  return status;
}

SANE_Status
cmd_unlock (device *hw)
{
  SANE_Status status = SANE_STATUS_GOOD;

  const byte cmd[] = { ESC, ')' };
  byte reply = NUL;

  log_call();
  require (hw);

  channel_send (hw->channel, cmd, num_of (cmd), &status);
  if (SANE_STATUS_GOOD != status) return status;
  channel_recv (hw->channel, &reply, 1, &status);

  if (SANE_STATUS_GOOD == status)
    {
      if (0x80 == reply)
        {
          hw->is_locked = false;
        }
      else if (NAK == reply)
        {
          err_minor ("locking not supported by device, disabling");
          hw->uses_locking = false;
          status = SANE_STATUS_GOOD;
        }
      else
        {
          err_major ("unexpected reply to unlock command (%02x)", reply);
          status = SANE_STATUS_IO_ERROR;
        }
    }

  return status;
}

SANE_Status
cmd_request_scanner_maintenance (device *hw, uint16_t mode)
{
  SANE_Status status = SANE_STATUS_GOOD;

  const byte cmd[] = { ESC, '1' };
  byte param[8];
  byte reply = NUL;

  log_call ("(%04x)", mode);
  require (hw);

  memset (param, 0, sizeof (param));
  uint16_to_buf (mode, param);

  channel_send (hw->channel, cmd, num_of (cmd), &status);
  if (SANE_STATUS_GOOD != status) return status;
  channel_recv (hw->channel, &reply, sizeof (reply), &status);
  if (SANE_STATUS_GOOD != status) return status;

  if (ACK != reply)
    {
      err_major ("unexpected reply to maintenance command (%02x)", reply);
      return SANE_STATUS_IO_ERROR;
    }

  channel_send (hw->channel, (const byte *)param, num_of (param), &status);
  if (SANE_STATUS_GOOD != status) return status;
  channel_recv (hw->channel, &reply, sizeof (reply), &status);
  if (SANE_STATUS_GOOD != status) return status;

  if (BUSY == reply)
    {
      status = SANE_STATUS_DEVICE_BUSY;
    }
  else if (NAK == reply)
    {
      err_minor ("invalid maintenance command (%04x)", mode);
      status = SANE_STATUS_INVAL;
    }
  else if (ACK != reply)
    {
      err_major ("unexpected reply to maintenance command (mode=%04x, %02x)",
                 mode, reply);
      status = SANE_STATUS_IO_ERROR;
    }
  return status;
}
