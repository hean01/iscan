/*  xmltest.c -- simple model cache info checker
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


#ifndef included_xmltest_h
#define included_xmltest_h

#include "../get-infofile.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
  char *fw_name;                /* key for _model_info_cache_get_info */
  char *overseas;               /* model name */
  char *japan;                  /* model name */
  char *name;                   /* points to one of overseas, japan or
				 * fw_name, never NULL */
  scan_command_t *command;      /* command customisation info */
  EpsonScanHard   profile;      /* colour profiles */

  bool from_file;               /* origin of our data, used to control
				 * which members need to be free()d at
				 * destruction time */

} _model_info_t;

typedef struct
{
  char *fw_name;                /* key for _model_info_cache_get_info */
  char *overseas;               /* model name */
  char *japan;                  /* model name */

  scan_command_t   command;     /* command customisation info */
  EpsonScanHardRec profile;     /* colour profiles */

} _model_info_t_test;

const _model_info_t_test gt_x970 = {
  "GT-X970",
  NULL,
  NULL,

  {0x01, 0, 0, 0xFF, true, true},
  {0x87,
   {{1.1978,-0.1417,-0.0561,-0.0852, 1.1610,-0.0758,-0.0395,-0.3212, 1.3607},
    {1.0000, 0.0009,-0.0009,-0.1268, 1.0523, 0.0745,-0.0075,-0.0873, 1.0948},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.4475,-0.3957,-0.0518,-0.0138, 1.0644,-0.0506,-0.0199,-0.2050, 1.2249}}},

};

const _model_info_t_test es_h300 = {
  "ES-H300",
  "GT-2500",
  "ES-H300",

  {0x01, 0, 0, 0xFF, false, false},
  {0x87,
   {{1.0359,-0.0146,-0.0213,-0.0752, 1.0963,-0.0211,-0.0456,-0.3238, 1.3693},
    {1, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 1}}},

};

const _model_info_t_test perfection_610 = {
  "Perfection610",
  "Perfection 610",
  NULL,

  {0x01, 0, 0, 0xFF, false, false},
  {0x87,
   {{1.1442,-0.0705,-0.0737,-0.0702, 1.1013,-0.0311,-0.0080,-0.3588, 1.3668},
    {1, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 1}}},

};

const _model_info_t_test lp_m5600 = {
  "LP-M5600",
  NULL,
  "LP-M5600",

  {0x01, 0, 0x19, 0xFF, false, false},
  {0x87,
   {{1.0784,-0.0560,-0.0224,-0.1793, 1.2234,-0.0441,-0.0041,-0.2636, 1.2677},
    {1, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 1}}},

};

const _model_info_t_test pm_a820 = {
  "PM-A820",
  "Stylus Photo RX560/RX580/RX590",
  "PM-A820",

  {0x01, 0, 0, 0xFF, false, false},
  {0x87,
   {{0.9533, 0.0885,-0.0418, 0.0033, 1.0627,-0.0660,-0.0137,-0.1904, 1.2041},
    {1, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 1}}},

};

const _model_info_t_test cx_4600 = {
  "CX4600",
  "Stylus CX4500/CX4600",
  NULL,

  {0x01, 0, 0, 0xFF, false, false},
  {0x87,
   {{0.9828, 0.0924,-0.0752, 0.0255, 1.151,-0.1765, 0.0049,-0.325, 1.3201},
    {1, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 1}}},

};

#ifdef __cplusplus
}       /* extern "C" */
#endif

#endif  /* !defined (included_xmltest_h) */
