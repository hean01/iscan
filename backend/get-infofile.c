/*  get-infofile.c -- 
 *  Copyright (C) 2010  SEIKO EPSON CORPORATION
 *
 *  License: GPLv2+|iscan
 *  Authors: SEIKO EPSON CORPORATION
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


#include <errno.h>
#include <ltdl.h>
#include <string.h>

#include "get-infofile.h"
#include "profile.c"

const struct ScannerData scanner_data[] = {
  {"GT-10000", 0x05, 0x05, {NULL, NULL}},
  {"ES-6000", 0x05, 0x05, {NULL, NULL}},
  {"Perfection610", 0x06, 0x01, {"Perfection 610", NULL}},
  {"GT-6600", 0x06, 0x01, {NULL, NULL}},
  {"Perfection1200", 0x07, 0x01, {"Perfection 1200", NULL}},
  {"GT-7600", 0x07, 0x01, {NULL, NULL}},
  {"Expression1600", 0x0D, 0x02, {"Expression 1600", NULL}},
  {"ES-2000", 0x0D, 0x02, {NULL, NULL}},
  {"Expression1640XL", 0x0F, 0x04, {"Expression 1640XL", NULL}},
  {"ES-8500", 0x0F, 0x04, {NULL, NULL}},
  {"Perfection640", 0x15, 0x01, {"Perfection 640", NULL}},
  {"GT-6700", 0x15, 0x01, {NULL, NULL}},
  {"Perfection1640", 0x16, 0x01, {"Perfection 1640", NULL}},
  {"GT-8700", 0x16, 0x01, {NULL, NULL}},
  {"Perfection1240", 0x18, 0x01, {"Perfection 1240", NULL}},
  {"GT-7700", 0x18, 0x01, {NULL, NULL}},
  {"GT-30000", 0x1A, 0x05, {NULL, NULL}},
  {"ES-9000H", 0x1A, 0x05, {NULL, NULL}},
  {"Expression1680", 0x1B, 0x02, {"Expression 1680", NULL}},
  {"ES-2200", 0x1B, 0x02, {NULL, NULL}},
  {"GT-7200", 0x1D, 0x01, {"Perfection 1250", "GT-7200"}},
  {"GT-8200", 0x1F, 0x01, {"Perfection 1650", "GT-8200"}},
  {"GT-9700", 0x21, 0x01, {"Perfection 2450", "GT-9700"}},
  {"GT-7300", 0x23, 0x01, {"Perfection 1260", "GT-7300"}},
  {"GT-8300", 0x25, 0x01, {"Perfection 1660", "GT-8300"}},
  {"GT-9300", 0x27, 0x01, {"Perfection 2400", "GT-9300"}},
  {"GT-9800", 0x29, 0x01, {"Perfection 3200", "GT-9800"}},
  {"ES-7000H", 0x2B, 0x05, {"GT-15000", "ES-7000H"}},
  {"LP-A500", 0x51, 0x01, {NULL, NULL}},
  {"AL-CX11", 0x51, 0x01, {"AcuLaser CX11", NULL}},
  {"GT-9400", 0x32, 0x06, {"Perfection 3170", "GT-9400"}},
  {"CC-600PX", 0x2D, 0x01, {"Stylus CX5100/CX5200", "CC-600PX"}},
  {"PM-A850", 0x3A, 0x01, {"Stylus Photo RX600", "PM-A850"}},
  {"CX5400", 0x36, 0x01, {"Stylus CX5300/CX5400", NULL}},
  {"GT-X700", 0x34, 0x01, {"Perfection 4870", "GT-X700"}},
  {"RX500", 0x38, 0x01, {"Stylus Photo RX500/RX510", NULL}},
  {"PX-A650", 0x37, 0x01, {"Stylus CX6300/CX6400", NULL}},
  {"ES-10000G", 0x3F, 0x05, {NULL, NULL}},
  {"Expression10000", 0x3F, 0x05, {"Expression 10000XL", NULL}},
  {"CX4600", 0x46, 0x01, {"Stylus CX4500/CX4600", NULL}},
  {"CX6600", 0x49, 0x01, {"Stylus CX6500/CX6600", NULL}},
  {"CX3600", 0x46, 0x01, {"Stylus CX3500/CX3600/CX3650", "PX-A550"}},
  {"RX420", 0x48, 0x01, {"Stylus Photo RX420/RX425/RX430", NULL}},
  {"PM-A700", 0x48, 0x01, {NULL, NULL}},
  {"PM-A870", 0x4B, 0x01, {"Stylus Photo RX620/RX630", "PM-A870"}},
  {"GT-F500", 0x41, 0x06, {"Perfection 2480/2580", "GT-F500/F550"}},
  {"GT-F600", 0x43, 0x06, {"Perfection 4180", "GT-F600"}},
  {"PM-A900", 0x4D, 0x01, {"Stylus Photo RX700", "PM-A900"}},
  {"GT-X800", 0x4F, 0x01, {"Perfection 4990", "GT-X800"}},
  {"GT-X750", 0x54, 0x01, {"Perfection 4490", "GT-X750"}},
  {"LP-M5500", 0x56, 0x05, {NULL, NULL}},
  {"LP-M5600", 0x78, 0x05, {NULL, "LP-M5600"}},
  {"GT-F520", 0x52, 0x01, {"Perfection 3490/3590", "GT-F520/F570"}},
  {"CX3800", 0x57, 0x01, {"Stylus CX3700/CX3800/DX3800", NULL}},
  {"CX7800", 0x5B, 0x01, {"Stylus CX7700/CX7800", NULL}},
  {"PM-A750", 0x5D, 0x01, {"Stylus Photo RX520/RX530", "PM-A750"}},
  {"CX4800", 0x58, 0x01, {"Stylus CX4700/CX4800/DX4800", "PX-A650"}},
  {"CX4200", 0x59, 0x01, {"Stylus CX4100/CX4200/DX4200", NULL}},
  {"PM-A950", 0x61, 0x01, {NULL, "PM-A950"}},
  {"PM-A890", 0x5F, 0x01, {"Stylus Photo RX640/RX650", "PM-A890"}},
  {"GT-X900", 0x63, 0x01, {"Perfection V700/V750", "GT-X900"}},
  {"CX4000", 0x6B, 0x01, {"Stylus CX3900/DX4000", "PX-A620"}},
  {"CX3000v", 0x6A, 0x01, {"Stylus CX2800/CX2900/ME 200", NULL}},
  {"ES-H300", 0x65, 0x01, {"GT-2500", "ES-H300"}},
  {"CX6000", 0x6C, 0x01, {"Stylus CX5900/CX6000/DX6000", "PX-A720"}},
  {"PM-A820", 0x70, 0x01, {"Stylus Photo RX560/RX580/RX590", "PM-A820"}},
  {"PM-A920", 0x71, 0x01, {NULL, "PM-A920"}},
  {"PM-A970", 0x73, 0x01, {NULL, "PM-A970"}},
  {"PM-T990", 0x75, 0x01, {NULL, "PM-T990"}},
  {"CX5000", 0x77, 0x01, {"Stylus CX4900/CX5000/DX5000", NULL}},
  {"GT-S600", 0x66, 0x01, {"Perfection V10/V100", "GT-S600/GT-F650"}},
  {"GT-F700", 0x68, 0x01, {"Perfection V350", "GT-F700"}},
  {"AL-CX21", 0x79, 0x01, {"AcuLaser CX21", NULL}},
  {"GT-F670", 0x7A, 0x01, {"Perfection V200", "GT-F670"}},
  {"GT-X770", 0x7C, 0x06, {"Perfection V500", "GT-X770"}},
  {"CX4400", 0x7E, 0x01, {"Stylus CX4300/CX4400/CX5500/CX5600/DX4400", NULL}},
  {"CX7400", 0x7F, 0x01, {"Stylus CX7300/CX7400/DX7400", "PX-A640"}},
  {"CX8400", 0x80, 0x01, {"Stylus CX8300/CX8400/DX8400", "PX-A740"}},
  {"CX9400Fax", 0x81, 0x01, {"Stylus CX9300F/CX9400Fax/DX9400F", "PX-FA700"}},
  {"PM-T960", 0x82, 0x01, {NULL, "PM-T960"}},
  {"PM-A940", 0x84, 0x01, {"Stylus Photo RX680/RX685/RX690", "PM-A940"}},
  {"PM-A840", 0x85, 0x01, {"Stylus Photo RX585/RX595/RX610", "PM-A840/PM-A840S"}},
  {"GT-D1000", 0x86, 0x01, {"GT-1500", "GT-D1000"}},
  {"GT-X970", 0x87, 0x01, {NULL, NULL}},
  {"LP-M5000", 0x97, 0x01, {NULL, NULL}},
  {"LP-M6000", 0x89, 0x01, {NULL, NULL}},
  {"ES-H7200", 0x8A, 0x05, {NULL, NULL}},
  {"GT-20000", 0x8A, 0x05, {NULL, NULL}},
  {"NX200", 0x8D, 0x01, {"Stylus NX200/SX200/TX200", NULL}},
  {"NX400", 0x8E, 0x01, {"Stylus NX400/SX400/TX400", "PX-501A"}},
  {"NX100", 0x93, 0x01, {"Stylus NX100/SX100/TX100/ME 300", "PX-401A"}},
  {"NX300", 0x8F, 0x01, {"Stylus BX300F/TX300F/NX300/ME Office 600F", NULL}},
  {"WorkForce 600", 0x90, 0x01, {"Stylus BX600FW/SX600FW/TX600FW/ME Office 700FW/WorkForce 600", "PX-601F"}},
  {"Artisan 800", 0x91, 0x01, {"Stylus Photo PX800FW/TX800FW/Artisan 800", "EP-901A/EP-901F"}},
  {"Artisan 700", 0x92, 0x01, {"Stylus Photo PX700W/TX700W/Artisan 700", "EP-801A"}},
  {"WorkForce 500", 0x96, 0x01, {NULL, NULL}},
  {"GT-F720", 0x8B, 0x01, {"Perfection V300", "GT-F720"}},
  {"GT-S620", 0x8B, 0x01, {"Perfection V30", "GT-S620"}},
  {"GT-S50", 0x00, 0x01, {"GT-S50", "ES-D200"}},
  {"GT-S80", 0x00, 0x01, {"GT-S80", "ES-D400"}},
  {"PID 0851", 0x98, 0x01, {"Stylus NX410/SX410/TX410 Series", NULL}},
  {"PID 084D", 0x99, 0x01, {"Stylus NX110/SX110/TX110 Series", "PX-402A"}},
  {"PID 084F", 0x9A, 0x01, {"Stylus NX210/SX210/TX210/ME OFFICE 510 Series", NULL}},
  {"PID 0854", 0x9B, 0x01, {"Stylus Office TX510FN/BX310FN/WorkForce 310/ME OFFICE 650FN Series", NULL}},
  {"PID 0856", 0x9C, 0x01, {"Stylus NX510/SX510W/TX550W Series", "PX-502A"}},
  {"PID 0855", 0x9D, 0x01, {"Stylus Office TX610FW/BX610FW/SX610FW/WorkForce 610 Series", "PX-602F"}},
  {"PID 0850", 0x9E, 0x01, {"Stylus Photo PX650/TX650 Series", "EP-702A"}},
  {"PID 0852", 0xA0, 0x01, {"Stylus Photo TX710W/PX710W/Artisan 710 Series", "EP-802A"}},
  {"PID 0853", 0x9F, 0x01, {"Stylus Photo PX810FW/Artisan 810 Series", "EP-902A"}},
  {"GT-X820", 0xA1, 0x01, {"Perfection V600 Photo", "GT-X820"}},
  {"GT-S55", 0x00, 0x01, {"GT-S55", NULL}},
  {"GT-S85", 0x00, 0x01, {"GT-S85", "ES-D350"}},

  /* array terminator */
  {NULL, 0x00, 0x00, {NULL, NULL}},
};


static struct EpsonScanCommand scan_command[] = {
  {0x01, 0, 0, ILLEGAL_CMD},
  {0x02, ILLEGAL_CMD, 0, ILLEGAL_CMD},
  {0x03, 0, ILLEGAL_CMD, ILLEGAL_CMD},
  {0x04, ILLEGAL_CMD, ILLEGAL_CMD, ILLEGAL_CMD},
  {0x05, 0, 0x19, ILLEGAL_CMD},
  {0x06, 0, 0, '\f'},
};

static
const scanner_data_t *
get_scanner (const char *fw_name)
{
  const scanner_data_t *data = scanner_data;
  require (data);
  {                             /* input validation*/
    if (!fw_name || 0 == strlen (fw_name))
      return NULL;
  }

  while (data->fw_name != NULL  /* end of array */
         && (0 != strcmp (data->fw_name, fw_name)))
  {
    ++data;
  }

  if(!data->fw_name){
    err_major("Unknown model name.");
    return NULL;
  }

  return data;
}

char *
get_scanner_data(const char* fw_name, const char *name)
{
  const scanner_data_t *data = get_scanner (fw_name);

  if (!data) return NULL;

  if(strcmp(name, FIRMWARE) == 0)    return data->fw_name;
  else if(strcmp(name, MODEL_OVERSEAS) == 0)    return data->name.overseas;
  else if(strcmp(name, MODEL_JAPAN) == 0)    return data->name.japan;

  return NULL;
}

struct EpsonScanCommand *get_scan_command(const char *fw_name)
{
  const scanner_data_t *data = get_scanner (fw_name);
  scan_command_t *custom_command = NULL;

  if (data && data->command_ID)
    {
      int id = data->command_ID - 1; /* adjust for offset */
      require (id >= 0);
      require ((unsigned) id < num_of (scan_command));

      custom_command = &scan_command[id];
    }
  else
    {
      custom_command = &scan_command[0];
    }

  return custom_command;
}

EpsonScanHard get_epson_scan_hard (const char *fw_name)
{
  const scanner_data_t *data = get_scanner (fw_name);
  EpsonScanHard profile = NULL;

  int i = 0;

  if (data && data->profile_ID)
    {
      i = num_of (_epson_scan_hard);

      while (--i && data->profile_ID != epson_scan_hard[i].modelID)
        ;
    }
  profile = (EpsonScanHard) &epson_scan_hard[i];

  return profile;
}
