/* epkowa.c - SANE backend for EPSON flatbed scanners
   	      (Image Scan! version)

   Based on the SANE Epson backend (originally from sane-1.0.3)
   - updated to sane-backends-1.0.6
   - renamed from epson to epkowa to avoid confusion
   - updated to sane-backends-1.0.12
   - updated to sane-backends-1.0.15
	      
   Based on Kazuhiro Sasayama previous
   Work on epson.[ch] file from the SANE package.

   Original code taken from sane-0.71
   Copyright (C) 1997 Hypercore Software Design, Ltd.

   modifications
   Copyright (C) 1998-1999 Christian Bucher <bucher@vernetzt.at>
   Copyright (C) 1998-1999 Kling & Hautzinger GmbH
   Copyright (C) 1999 Norihiko Sawa <sawa@yb3.so-net.ne.jp>
   Copyright (C) 2000 Mike Porter <mike@udel.edu> (mjp)
   Copyright (C) 1999-2004 Karl Heinz Kremer <khk@khk.net>
   Copyright (C) 2001-2013 SEIKO EPSON CORPORATION

   This file is part of the EPKOWA SANE backend.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston,
   MA 02111-1307, USA.

   As a special exception, the authors of SANE give permission for
   additional uses of the libraries contained in this release of SANE.

   The exception is that, if you link a SANE library with other files
   to produce an executable, this does not by itself cause the
   resulting executable to be covered by the GNU General Public
   License.  Your use of that executable is in no way restricted on
   account of linking the SANE library code into it.

   This exception does not, however, invalidate any other reasons why
   the executable file might be covered by the GNU General Public
   License.

   If you submit changes to SANE to the maintainers to be included in
   a subsequent release, you agree by submitting the changes that
   those changes may be distributed with this exception intact.

   If you write modifications of your own for SANE, it is your choice
   whether to permit this exception to apply to your modifications.
   If you do not wish that, delete this exception notice.
 */


#ifndef SANE_I18N
#define SANE_I18N(text) (text)
#endif

#ifdef HAVE_CONFIG_H
#include  <config.h>
#endif

#include  <limits.h>
#include  <string.h>
#include  <unistd.h>
#include  <errno.h>
#include  <math.h>

#include  <sane/saneopts.h>

#ifndef BACKEND_NAME
#define BACKEND_NAME epkowa
#endif

#include "backend.h"
#include "command.h"
#include "hw-data.h"
#include "utils.h"
#include "timing.h"
#include "cfg-obj.h"
#include "dip-obj.h"
#include "model-info.h"
#include "utils.h"

#include  "epkowa_scsi.h"
#include  "sane/sanei_pio.h"
#include  "epkowa_ip.h"		/* interpreter-based scanner support */

#include "sane/sanei.h"

#define DEFAULT_RESOLUTION		300	/* dpi */
#define DEFAULT_X_RESOLUTION	DEFAULT_RESOLUTION
#define DEFAULT_Y_RESOLUTION	DEFAULT_RESOLUTION

#define	 S_ACK	"\006"
#define	 S_CAN	"\030"

/* Usable values when defining EPSON_LEVEL_DEFAULT
 * There is also a function level "A5", used for the GT-300, a
 * monochrome only scanner.  This level is not supported.
 */
#define	 EPSON_LEVEL_A1		 0
#define	 EPSON_LEVEL_A2		 1
#define	 EPSON_LEVEL_B1		 2
#define	 EPSON_LEVEL_B2		 3
#define	 EPSON_LEVEL_B3		 4
#define	 EPSON_LEVEL_B4		 5
#define	 EPSON_LEVEL_B5		 6
#define	 EPSON_LEVEL_B6		 7
#define	 EPSON_LEVEL_B7		 8
#define	 EPSON_LEVEL_B8		 9
#define	 EPSON_LEVEL_F5		10
#define  EPSON_LEVEL_D1		11
#define  EPSON_LEVEL_D2		12
#define  EPSON_LEVEL_D7		13
#define  EPSON_LEVEL_D8		14

#define	 EPSON_LEVEL_DEFAULT	EPSON_LEVEL_B3

static EpsonCmdRec epson_cmd[] = {
/*
 *       request identity
 *       |   request identity2
 *       |   |   request status
 *       |   |   |   request condition
 *       |   |   |   |   set color mode
 *       |   |   |   |   |   start scanning
 *       |   |   |   |   |   |   set data format
 *       |   |   |   |   |   |   |   set resolution
 *       |   |   |   |   |   |   |   |   set zoom
 *       |   |   |   |   |   |   |   |   |   set scan area
 *       |   |   |   |   |   |   |   |   |   |   set brightness
 *       |   |   |   |   |   |   |   |   |   |   |              set gamma
 *       |   |   |   |   |   |   |   |   |   |   |              |   set halftoning
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   set color correction
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   initialize scanner
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   set speed
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   set lcount
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   mirror image
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   set gamma table
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   set outline emphasis
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   set dither
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   set color correction coefficients
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   |   request extension status
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   |   |   control an extension
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   |   |   |    forward feed / eject
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   |   |   |    |   feed
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   |   |   |    |   |     request push button status
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   |   |   |    |   |     |   control auto area segmentation
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   |   |   |    |   |     |   |   set film type
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   |   |   |    |   |     |   |   |   set exposure time
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   |   |   |    |   |     |   |   |   |   set bay
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   |   |   |    |   |     |   |   |   |   |   set threshold
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   |   |   |    |   |     |   |   |   |   |   |   set focus position
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   |   |   |    |   |     |   |   |   |   |   |   |   request focus position 
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   |   |   |    |   |     |   |   |   |   |   |   |   |
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   |   |   |    |   |     |   |   |   |   |   |   |   |
 */
  {"A1",'I', 0 ,'F','S', 0 ,'G', 0 ,'R', 0 ,'A', 0 ,{ 0, 0, 0}, 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,  0 , 0   , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
  {"A2",'I', 0 ,'F','S', 0 ,'G','D','R','H','A','L',{-3, 3, 0},'Z','B', 0 ,'@', 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,  0 , 0   , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
  {"B1",'I', 0 ,'F','S','C','G','D','R', 0 ,'A', 0 ,{ 0, 0, 0}, 0 ,'B', 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,  0 , 0   , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
  {"B2",'I', 0 ,'F','S','C','G','D','R','H','A','L',{-3, 3, 0},'Z','B', 0 ,'@', 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,  0 , 0   , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
  {"B3",'I', 0 ,'F','S','C','G','D','R','H','A','L',{-3, 3, 0},'Z','B','M','@', 0 , 0 , 0 , 0 , 0 , 0 ,'m','f','e',  0 , 0   , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
  {"B4",'I', 0 ,'F','S','C','G','D','R','H','A','L',{-3, 3, 0},'Z','B','M','@','g','d', 0 ,'z','Q','b','m','f','e',  0 , 0   , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
  {"B5",'I', 0 ,'F','S','C','G','D','R','H','A','L',{-3, 3, 0},'Z','B','M','@','g','d','K','z','Q','b','m','f','e',  0 , 0   , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
  {"B6",'I', 0 ,'F','S','C','G','D','R','H','A','L',{-3, 3, 0},'Z','B','M','@','g','d','K','z','Q','b','m','f','e',  0 , 0   , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
  {"B7",'I', 0 ,'F','S','C','G','D','R','H','A','L',{-3, 3, 0},'Z','B','M','@','g','d','K','z','Q','b','m','f','e','\f', 0   ,'!','s','N', 0 , 0 ,'t', 0 , 0 },
  {"B8",'I', 0 ,'F','S','C','G','D','R','H','A','L',{-3, 3, 0},'Z','B','M','@','g','d','K','z','Q','b','m','f','e','\f', 0x19,'!','s','N', 0 , 0 ,'t','p','q'},
  {"F5",'I', 0 ,'F','S','C','G','D','R','H','A','L',{-3, 3, 0},'Z', 0 ,'M','@','g','d','K','z','Q', 0 ,'m','f','e','\f', 0   , 0 , 0 ,'N','T','P', 0 , 0 , 0 },
  {"D1",'I','i','F', 0 ,'C','G','D','R', 0 ,'A', 0 ,{ 0, 0, 0},'Z', 0 , 0 ,'@','g','d', 0 ,'z', 0 , 0 , 0 ,'f', 0 ,  0 , 0   ,'!', 0 , 0 , 0 , 0 ,'t', 0 , 0 },
  {"D2",'I','i','F', 0 ,'C','G','D','R', 0 ,'A', 0 ,{ 0, 0, 0},'Z', 0 , 0 ,'@','g','d', 0 ,'z', 0 , 0 , 0 ,'f','e',  0 , 0   ,'!', 0 ,'N', 0 , 0 ,'t', 0 , 0 },
  {"D7",'I','i','F', 0 ,'C','G','D','R', 0 ,'A', 0 ,{ 0, 0, 0},'Z', 0 , 0 ,'@','g','d', 0 ,'z', 0 , 0 , 0 ,'f','e','\f', 0   ,'!', 0 ,'N', 0 , 0 ,'t', 0 , 0 },
  {"D8",'I','i','F', 0 ,'C','G','D','R', 0 ,'A', 0 ,{ 0, 0, 0},'Z', 0 , 0 ,'@','g','d', 0 ,'z', 0 , 0 , 0 ,'f','e','\f', 0   ,'!', 0 ,'N', 0 , 0 ,'t', 0 , 0 },
};


/* Definition of the mode_param struct, that is used to 
 * specify the valid parameters for the different scan modes.
 *
 * The depth variable gets updated when the bit depth is modified.
 */

struct mode_param
{
  int color;
  int mode_flags;
  int dropout_mask;
  int depth;
};

static struct mode_param mode_params[] = {
  {0, 0x00, 0x30, 1},
  {0, 0x00, 0x30, 8},
  {1, 0x02, 0x00, 8}
};

static const SANE_String_Const mode_list[] = {
  SANE_I18N ("Binary"),
  SANE_I18N ("Gray"),
  SANE_I18N ("Color"),
  NULL
};

static const SANE_String_Const adf_mode_list[] = {
  SANE_I18N ("Simplex"),
  SANE_I18N ("Duplex"),
  NULL
};

static const SANE_String_Const dfd_sensitivity[] = {
  SANE_I18N ("None"),
  SANE_I18N ("Low"),
  SANE_I18N ("High"),
  NULL
};

/* Define the different scan sources:
 */

#define  FBF_STR	SANE_I18N("Flatbed")
#define  TPU_STR	SANE_I18N("Transparency Unit")
#define  ADF_STR	SANE_I18N("Automatic Document Feeder")

#define FILM_TYPE_POSITIVE	(0)
#define FILM_TYPE_NEGATIVE	(1)

static const SANE_String_Const film_list[] = { 
  SANE_I18N ("Positive Film"), 
  SANE_I18N ("Negative Film"), 
  NULL
};

static const SANE_String_Const focus_list[] = {
  SANE_I18N ("Focus on glass"),
  SANE_I18N ("Focus 2.5mm above glass"),
  NULL
};

#define HALFTONE_NONE 0x01
#define HALFTONE_TET 0x03

static int halftone_params[] = { 
  HALFTONE_NONE, 
  0x00, 
  0x10, 
  0x20, 
  0x80, 
  0x90, 
  0xa0, 
  0xb0, 
  HALFTONE_TET,
  0xc0, 
  0xd0
};

static const SANE_String_Const halftone_list[] = { 
  SANE_I18N ("None"), 
  SANE_I18N ("Halftone A (Hard Tone)"),
  SANE_I18N ("Halftone B (Soft Tone)"), 
  SANE_I18N ("Halftone C (Net Screen)"),
  NULL
};

static const SANE_String_Const halftone_list_4[] = { 
  SANE_I18N ("None"), 
  SANE_I18N ("Halftone A (Hard Tone)"),
  SANE_I18N ("Halftone B (Soft Tone)"), 
  SANE_I18N ("Halftone C (Net Screen)"),
  SANE_I18N ("Dither A (4x4 Bayer)"),
  SANE_I18N ("Dither B (4x4 Spiral)"),
  SANE_I18N ("Dither C (4x4 Net Screen)"),
  SANE_I18N ("Dither D (8x4 Net Screen)"), 
  NULL
};

static const SANE_String_Const halftone_list_7[] = { 
  SANE_I18N ("None"), 
  SANE_I18N ("Halftone A (Hard Tone)"),
  SANE_I18N ("Halftone B (Soft Tone)"), 
  SANE_I18N ("Halftone C (Net Screen)"),
  SANE_I18N ("Dither A (4x4 Bayer)"),
  SANE_I18N ("Dither B (4x4 Spiral)"),
  SANE_I18N ("Dither C (4x4 Net Screen)"),
  SANE_I18N ("Dither D (8x4 Net Screen)"),
  SANE_I18N ("Text Enhanced Technology"), 
  SANE_I18N ("Download pattern A"),
  SANE_I18N ("Download pattern B"), 
  NULL
};

static int dropout_params[] = { 
  0x00,	/* none */
  0x10,	/* red */
  0x20,	/* green */
  0x30	/* blue */
};

static const SANE_String_Const dropout_list[] = { 
  SANE_I18N ("None"), 
  SANE_I18N ("Red"), 
  SANE_I18N ("Green"),
  SANE_I18N ("Blue"), 
  NULL
};

static const SANE_String_Const brightness_method_list[] = {
  SANE_I18N ("hardware"),       /* needs to be first */
  SANE_I18N ("iscan"),
  SANE_I18N ("gimp"),
  NULL
};

#define max_val 100                 /* any integer value will do */
static const SANE_Range brightness_range = { -max_val, max_val, 1};
static const SANE_Range contrast_range   = { -max_val, max_val, 1};
#if 0
/* We don't provide highlight and shadow options yet because we
 * haven't worked out the GIMP part of that and how that should
 * interact with the brightness-method option.  For the "iscan"
 * method, the values below are OK.
 */
static const SANE_Range highlight_range  = { 0, max_val, 0 };
static const SANE_Range shadow_range     = { 0, max_val, 0 };
#endif
#undef max_val

/* Color correction:
 * One array for the actual parameters that get sent to the scanner (color_params[]),
 * one array for the strings that get displayed in the user interface (color_list[])
 * and one array to mark the user defined color correction (dolor_userdefined[]). 
 */

static int color_params_ab[] = {
  0x00,
  0x01,
  0x10,
  0x20,
  0x40,
  0x80
};

static SANE_Bool color_userdefined_ab[] = {
  SANE_FALSE,
  SANE_TRUE,
  SANE_FALSE,
  SANE_FALSE,
  SANE_FALSE,
  SANE_FALSE
};

static const SANE_String_Const color_list_ab[] = {
  SANE_I18N ("No Correction"),
  SANE_I18N ("User defined"),
  SANE_I18N ("Impact-dot printers"),
  SANE_I18N ("Thermal printers"),
  SANE_I18N ("Ink-jet printers"),
  SANE_I18N ("CRT monitors"),
  NULL
};

static int color_params_d[] = {
  0x00
};

static SANE_Bool color_userdefined_d[] = {
  SANE_TRUE
};

static const SANE_String_Const color_list_d[] = {
  "User defined",
  NULL
};


/* Gamma correction:
 * The A and B level scanners work differently than the D level scanners, therefore
 * I define two different sets of arrays, plus one set of variables that get set to
 * the actally used params and list arrays at runtime.
 */

static int gamma_params_ab[] = {
  0x01,
  0x03,
  0x04,
  0x00,
  0x10,
  0x20
};

static const SANE_String_Const gamma_list_ab[] = {
  SANE_I18N ("Default"),
  SANE_I18N ("User defined (Gamma=1.0)"),
  SANE_I18N ("User defined (Gamma=1.8)"),
  SANE_I18N ("High density printing"),
  SANE_I18N ("Low density printing"),
  SANE_I18N ("High contrast printing"),
  NULL
};

static SANE_Bool gamma_userdefined_ab[] = {
  SANE_FALSE,
  SANE_TRUE,
  SANE_TRUE,
  SANE_FALSE,
  SANE_FALSE,
  SANE_FALSE,
};

static int gamma_params_d[] = { 
  0x03, 
  0x04
};

static const SANE_String_Const gamma_list_d[] = { 
  SANE_I18N ("User defined (Gamma=1.0)"),
  SANE_I18N ("User defined (Gamma=1.8)"), 
  NULL
};

static SANE_Bool gamma_userdefined_d[] = {
  SANE_TRUE,
  SANE_TRUE
};


/* Bay list:
 * this is used for the FilmScan
 */

static const SANE_String_Const bay_list[] = { 
  " 1 ", 
  " 2 ", 
  " 3 ", 
  " 4 ", 
  " 5 ", 
  " 6 ", 
  NULL
};

static const SANE_Range u8_range = { 0, 255, 0 };
static const SANE_Range s8_range = { -127, 127, 0 };
static const SANE_Range zoom_range = { 50, 200, 0 };

/* The "switch_params" are used for several boolean choices
 */
static int switch_params[] = {
  0,
  1
};

#define  mirror_params  switch_params
#define  speed_params	switch_params
#define  film_params	switch_params

static const SANE_Range outline_emphasis_range = { -2, 2, 0 };

enum
  {
    EXT_SANE_STATUS_NONE,
    EXT_SANE_STATUS_MULTI_FEED,
    EXT_SANE_STATUS_TRAY_CLOSED,
    EXT_SANE_STATUS_MAX,
  };
static const SANE_Range ext_sane_status = {
  EXT_SANE_STATUS_NONE,
  EXT_SANE_STATUS_MAX - 1,
  0
};

typedef struct {
  double width;                 /* in mm */
  double height;                /* in mm */
  SANE_String_Const name;
} media_type;

static SANE_String_Const media_maximum   = SANE_I18N ("Maximum");
static SANE_String_Const media_automatic = SANE_I18N ("Automatic");

/*! \brief List of preset media sizes
 *  This list is used to populate the constraint list for the
 *  scan-area option.
 *
 *  \remark  When adding Landscape and Portrait variants of a medium
 *           size, make sure to add three(!) entries using the same
 *           pattern as used for those already listed.  The algorithm
 *           used to populate the constraint list depends on this
 *           convention to create the most user-friendly list.
 */
static const media_type media_list[] = {
  /* ISO A Series */
  { 297, 420, SANE_I18N ("A3") },
  { 297, 210, SANE_I18N ("A4 Landscape") },
  { 210, 297, SANE_I18N ("A4 Portrait") },
  { 210, 297, SANE_I18N ("A4") },
  { 210, 148, SANE_I18N ("A5 Landscape") },
  { 148, 210, SANE_I18N ("A5 Portrait") },
  { 148, 210, SANE_I18N ("A5") },
  /* JIS B Series */
  { 257, 364, SANE_I18N ("B4") },
  { 257, 184, SANE_I18N ("B5 Landscape") },
  { 184, 257, SANE_I18N ("B5 Portrait") },
  { 184, 257, SANE_I18N ("B5") },
  /* North American sizes */
  { 11.00 * MM_PER_INCH, 17.00 * MM_PER_INCH, SANE_I18N ("Ledger") },
  {  8.50 * MM_PER_INCH, 14.00 * MM_PER_INCH, SANE_I18N ("Legal") },
  { 11.00 * MM_PER_INCH,  8.50 * MM_PER_INCH, SANE_I18N ("Letter Landscape") },
  {  8.50 * MM_PER_INCH, 11.00 * MM_PER_INCH, SANE_I18N ("Letter Portrait") },
  {  8.50 * MM_PER_INCH, 11.00 * MM_PER_INCH, SANE_I18N ("Letter") },
  { 10.50 * MM_PER_INCH,  7.25 * MM_PER_INCH, SANE_I18N ("Executive Landscape") },
  {  7.25 * MM_PER_INCH, 10.50 * MM_PER_INCH, SANE_I18N ("Executive Portrait") },
  {  7.25 * MM_PER_INCH, 10.50 * MM_PER_INCH, SANE_I18N ("Executive") },
  /* Miscellaneous */
  { 120, 120, SANE_I18N ("CD")},
};


static SANE_Word *bitDepthList = NULL;


/* Some utility macros
 */

/*! Returns the larger of the arguments \a a and \a b.
 */
#define max(a, b)  ((a) < (b) ? (b) : (a))

/*! Returns the smaller of the arguments \a a and \a b.
 */
#define min(a, b)  ((a) < (b) ? (a) : (b))


static size_t
max_string_size (const SANE_String_Const strings[])
{
  size_t size, max_size = 0;
  int i;

  for (i = 0; strings[i]; i++)
  {
    size = strlen (strings[i]) + 1;
    if (size > max_size)
      max_size = size;
  }
  return max_size;
}


static inline
bool
need_autocrop_override (const Epson_Scanner *s)
{
  return (SANE_OPTION_IS_ACTIVE (s->opt[OPT_AUTOCROP].cap)
          && s->val[OPT_AUTOCROP].b
          && 0 != autocrop_max_y (s->hw));
}

typedef struct
{
  u_char code;
  u_char status;
  u_char count1;
  u_char count2;

  u_char type;
  u_char level;

  u_char buf[1];

} EpsonIdentRec, *EpsonIdent;

typedef struct
{
  u_char code;
  u_char status;
  u_char count1;
  u_char count2;
  u_char buf[1];

} EpsonHdrRec, *EpsonHdr;


static SANE_Status
read_image_info_block (device *hw);

static SANE_Status
get_identity_information (device *hw);

static SANE_Status
get_hardware_property (device *hw);

static SANE_Status
get_identity2_information (device *hw, Epson_Scanner *s);

static SANE_Status
check_warmup (device *hw);

static SANE_Status
check_ext_status (device *hw);

SANE_Status
control_option_unit (device *hw, SANE_Bool use_duplex);

static SANE_Status
initialize (device *hw);

static SANE_Status
get_resolution_constraints (device *hw, Epson_Scanner *s);

static SANE_Status
get_push_button_status (device *hw, SANE_Bool *button_pushed);

static SANE_Status color_shuffle (Epson_Scanner *s, int *new_length);
static void activateOption (Epson_Scanner * s, SANE_Int option,
			    SANE_Bool * change);
static void deactivateOption (Epson_Scanner * s, SANE_Int option,
			      SANE_Bool * change);
static void setOptionState (Epson_Scanner * s, SANE_Bool state,
			    SANE_Int option, SANE_Bool * change);
static void filter_resolution_list (Epson_Scanner * s);
static void scan_finish (Epson_Scanner * s);

static void get_colorcoeff_from_profile (double *profile,
					 unsigned char *color_coeff);
static void round_cct (double org_cct[], int rnd_cct[]);
static int get_roundup_index (double frac[], int n);
static int get_rounddown_index (double frac[], int n);

static void handle_mode (Epson_Scanner * s, SANE_Int optindex,
			 SANE_Bool * reload);
static void handle_resolution (Epson_Scanner * s, SANE_Int option,
			       SANE_Word value);
static void change_profile_matrix (Epson_Scanner * s);
static void handle_depth_halftone (Epson_Scanner * s, SANE_Int optindex,
				   SANE_Bool * reload);

static SANE_Status create_epson_device (device **dev, channel* ch);
static SANE_Status create_epson_scanner (device *dev, Epson_Scanner **scanner);
static SANE_Status create_sane_handle (Epson_Scanner **scanner, const char *name, const void *dip);
static SANE_Status init_options (Epson_Scanner * s);

static scan_area_t get_model_info_max_scan_area (device *hw, int adf_mode);
static SANE_Status handle_scan_area(Epson_Scanner *s, int adf_mode); 

static SANE_Status handle_source (Epson_Scanner * s,
                                  SANE_Int optindex,
                                  char *value);

static void adf_handle_out_of_paper (Epson_Scanner * s);
static void adf_handle_manual_centering (Epson_Scanner *s, SANE_Bool finalize);

static void handle_autocrop (Epson_Scanner *s, SANE_Bool *value, SANE_Bool *reload);
static void handle_deskew (Epson_Scanner *s, SANE_Bool *value, SANE_Bool *reload);
static void handle_detect_doc_size (Epson_Scanner *s, SANE_Bool *value, SANE_Bool *reload);
static void handle_preview (Epson_Scanner *s, SANE_Bool *value, SANE_Bool *reload);

static SANE_Status
expect_ack (device *hw)
{
  u_char result[1];
  size_t len;
  SANE_Status status;

  log_call ();

  len = sizeof (result);

  channel_recv (hw->channel, result, len, &status);

  if (SANE_STATUS_GOOD != status)
    return status;

  if (ACK != result[0])
    return SANE_STATUS_INVAL;

  return SANE_STATUS_GOOD;
}


static SANE_Status
set_cmd (device *hw, u_char cmd, u_char val)
{
  SANE_Status status;
  u_char params[2];

  if (!cmd)
    return SANE_STATUS_UNSUPPORTED;

  log_call ("(%c)", cmd);

  /* Override with extended parameter command (FS W).
   * Do not override A, R and e, they are handled separately.
   */
  if (hw->using_fs && strchr ("CDgdZLMBtsQKN", cmd))
    return dev_set_scanning_parameter (hw, cmd, &val);

  params[0] = ESC;
  params[1] = cmd;

  channel_send (hw->channel, params, 2, &status);
  if (SANE_STATUS_GOOD != (status = expect_ack (hw)))
    return status;

  params[0] = val;
  channel_send (hw->channel, params, 1, &status);
  status = expect_ack (hw);

  return status;
}

static void
print_params (const SANE_Parameters params)
{
  log_data ("params.format = %d", params.format);
  log_data ("params.last_frame = %d", params.last_frame);
  log_data ("params.bytes_per_line = %d", params.bytes_per_line);
  log_data ("params.pixels_per_line = %d", params.pixels_per_line);
  log_data ("params.lines = %d", params.lines);
  log_data ("params.depth = %d", params.depth);
}


#define  set_focus_position(h,v)        set_cmd (h,(h)->cmd->set_focus_position,v)
#define  set_color_mode(h,v)		set_cmd (h,(h)->cmd->set_color_mode,v)
#define  set_data_format(h,v)		set_cmd (h,(h)->cmd->set_data_format, v)
#define  set_halftoning(h,v)		set_cmd (h,(h)->cmd->set_halftoning, v)
#define  set_gamma(h,v)			set_cmd (h,(h)->cmd->set_gamma, v)
#define  set_color_correction(h,v)	set_cmd (h,(h)->cmd->set_color_correction, v)
#define  set_lcount(h,v)		set_cmd (h,(h)->cmd->set_lcount, v)
#define  set_bright(h,v)		set_cmd (h,(h)->cmd->set_bright, v)
#define  mirror_image(h,v)		set_cmd (h,(h)->cmd->mirror_image, v)
#define  set_speed(h,v)			set_cmd (h,(h)->cmd->set_speed, v)
#define  set_outline_emphasis(h,v)	set_cmd (h,(h)->cmd->set_outline_emphasis, v)
#define  control_auto_area_segmentation(h,v)	set_cmd (h,(h)->cmd->control_auto_area_segmentation, v)
#define  set_film_type(h,v)		set_cmd (h,(h)->cmd->set_film_type, v)
#define  set_exposure_time(h,v)		set_cmd (h,(h)->cmd->set_exposure_time, v)
#define  set_bay(h,v)			set_cmd (h,(h)->cmd->set_bay, v)
#define  set_threshold(h,v)		set_cmd (h,(h)->cmd->set_threshold, v)


static SANE_Status
set_zoom (device *hw, int x_zoom, int y_zoom)
{
  SANE_Status status;
  u_char cmd[2];
  u_char params[2];

  if (!hw->cmd->set_zoom)
    return SANE_STATUS_GOOD;

  log_call ();

  cmd[0] = ESC;
  cmd[1] = hw->cmd->set_zoom;

  channel_send (hw->channel, cmd, 2, &status);
  status = expect_ack (hw);

  if (status != SANE_STATUS_GOOD)
    return status;

  params[0] = x_zoom;
  params[1] = y_zoom;

  channel_send (hw->channel, params, 2, &status);
  status = expect_ack (hw);

  return status;
}


static SANE_Status
set_resolution (device *hw, int xres, int yres)
{
  SANE_Status status;
  u_char params[4];

  if (!hw->cmd->set_resolution)
    return SANE_STATUS_GOOD;

  log_call ();

  params[0] = ESC;
  params[1] = hw->cmd->set_resolution;

  channel_send (hw->channel, params, 2, &status);
  status = expect_ack (hw);

  if (status != SANE_STATUS_GOOD)
    return status;

  params[0] = xres;
  params[1] = xres >> 8;
  params[2] = yres;
  params[3] = yres >> 8;

  channel_send (hw->channel, params, 4, &status);
  status = expect_ack (hw);

  return status;
}

/* set_scan_area() 
 *
 * Sends the "set scan area" command to the scanner with the currently selected 
 * scan area. This scan area is already corrected for "color shuffling" if 
 * necessary.
 */
static SANE_Status
set_scan_area (device *hw, int x, int y, int width, int height)
{
  SANE_Status status;
  u_char params[8];

  log_call ("(%d, %d, %d, %d)", x, y, width, height);

  if (!hw->cmd->set_scan_area)
  {
    err_major ("set_scan_area not supported");
    return SANE_STATUS_GOOD;
  }

  /* verify the scan area */
  if (x < 0 || y < 0 || width <= 0 || height <= 0)
    return SANE_STATUS_INVAL;

  params[0] = ESC;
  params[1] = hw->cmd->set_scan_area;

  channel_send (hw->channel, params, 2, &status);
  status = expect_ack (hw);
  if (status != SANE_STATUS_GOOD)
    return status;

  params[0] = x;
  params[1] = x >> 8;
  params[2] = y;
  params[3] = y >> 8;
  params[4] = width;
  params[5] = width >> 8;
  params[6] = height;
  params[7] = height >> 8;

  channel_send (hw->channel, params, 8, &status);
  status = expect_ack (hw);

  return status;
}

/* set_color_correction_coefficients()
 *
 * Sends the "set color correction coefficients" command with the currently selected
 * parameters to the scanner.
 */

static SANE_Status
set_color_correction_coefficients (device *hw, Epson_Scanner *s)
{
  SANE_Status status;
  u_char cmd = hw->cmd->set_color_correction_coefficients;
  u_char params[2];
  const int length = 9;
  unsigned char cccoeff[9];

  log_call ();

  s->cct[0] = SANE_UNFIX (s->val[OPT_CCT_1].w);
  s->cct[1] = SANE_UNFIX (s->val[OPT_CCT_2].w);
  s->cct[2] = SANE_UNFIX (s->val[OPT_CCT_3].w);
  s->cct[3] = SANE_UNFIX (s->val[OPT_CCT_4].w);
  s->cct[4] = SANE_UNFIX (s->val[OPT_CCT_5].w);
  s->cct[5] = SANE_UNFIX (s->val[OPT_CCT_6].w);
  s->cct[6] = SANE_UNFIX (s->val[OPT_CCT_7].w);
  s->cct[7] = SANE_UNFIX (s->val[OPT_CCT_8].w);
  s->cct[8] = SANE_UNFIX (s->val[OPT_CCT_9].w);

  if (!cmd)                     /* effect will be emulated */
    return SANE_STATUS_GOOD;

  params[0] = ESC;
  params[1] = cmd;

  channel_send (hw->channel, params, 2, &status);
  if (SANE_STATUS_GOOD != (status = expect_ack (hw)))
    return status;

  get_colorcoeff_from_profile (s->cct, cccoeff);

  log_data ("[ %d %d %d ][ %d %d %d ][ %d %d %d]",
            cccoeff[0], cccoeff[1], cccoeff[2],
            cccoeff[3], cccoeff[4], cccoeff[5],
            cccoeff[6], cccoeff[7], cccoeff[8]);

  channel_send (hw->channel, cccoeff, length, &status);
  status = expect_ack (hw);

  log_call ("exit");

  return status;
}


static SANE_Status
set_gamma_table (device *hw, const Epson_Scanner *s)
{
  SANE_Status status;
  u_char cmd = hw->cmd->set_gamma_table;
  u_char params[2];
  const int length = 257;
  u_char gamma[257];
  int n;
  int table;
  static char gamma_cmds[] = { 'R', 'G', 'B' };

  if (!cmd)
    return SANE_STATUS_UNSUPPORTED;

  log_call ();

  params[0] = ESC;
  params[1] = cmd;

/*  When handling inverted images, we must also invert the user
 *  supplied gamma function.  This is *not* just 255-gamma -
 *  this gives a negative image.
 */

  if (strcmp_c ("GT-6600", hw->fw_name) == 0 ||
      strcmp_c ("Perfection 610", hw->fw_name) == 0)
  {
    gamma_cmds[0] = 'R';
    gamma_cmds[1] = 'B';
    gamma_cmds[2] = 'G';
  }

  for (table = 0; table < 3; table++)
  {
    gamma[0] = gamma_cmds[table];
    if (s->invert_image)
    {
      for (n = 0; n < 256; ++n)
      {
	gamma[n + 1] = 255 - s->gamma_table[table][255 - n];
      }
    }
    else
    {
      for (n = 0; n < 256; ++n)
      {
	gamma[n + 1] = s->gamma_table[table][n];
      }
    }

    channel_send (hw->channel, params, 2, &status);
    if (SANE_STATUS_GOOD != (status = expect_ack (hw)))
      return status;

    channel_send (hw->channel, gamma, length, &status);
    if (SANE_STATUS_GOOD != (status = expect_ack (hw)))
      return status;
  }

  log_call ("exit");

  return status;
}

static SANE_Status
check_warmup (device *hw)
{
  SANE_Status status = check_ext_status (hw);

  log_call ();

  if (status == SANE_STATUS_DEVICE_BUSY)
  {
    int timeout;

    for (timeout = 0; timeout < 60; timeout++)
    {
      status = check_ext_status (hw);

      if (status == SANE_STATUS_DEVICE_BUSY)
	sleep (1);
      else
      {
	return status;
      }
    }
  }
  else
    return status;

  return status;
}


static SANE_Status
check_ext_status (device *hw)
{
  SANE_Status status;

  log_call ();
  require (hw);

  status = dev_request_extended_status (hw);

  if (EXT_STATUS_WU & hw->ext_status)
  {
    log_info ("option: warming up");
    status = SANE_STATUS_DEVICE_BUSY;
  }

  if (EXT_STATUS_FER & hw->ext_status)
  {
    log_info ("option: fatal error");
    status = SANE_STATUS_INVAL;
  }

  if (hw->adf)
  {
    if (ADF_STATUS_ERR & hw->adf->status
        || ADF_EXT_STATUS_ERR & hw->adf->ext_status)
    {
      log_info ("ADF: other error");
      status = SANE_STATUS_INVAL;
    }

    if (ADF_STATUS_PE & hw->adf->status)
    {
      log_info ("ADF: no paper");
      status = SANE_STATUS_NO_DOCS;
    }

    if (ADF_STATUS_PJ & hw->adf->status)
    {
      log_info ("ADF: paper jam");
      status = SANE_STATUS_JAMMED;
    }

    if (ADF_STATUS_OPN & hw->adf->status)
    {
      log_info ("ADF: cover open");
      status = SANE_STATUS_COVER_OPEN;
    }

    if (ADF_EXT_STATUS_DFE & hw->adf->ext_status)
      {
        log_info ("ADF: multi sheet feed");
        status = SANE_STATUS_JAMMED;
      }

    if (ADF_EXT_STATUS_TR_OPN & hw->adf->ext_status)
      {
        log_info ("ADF: tray open");
        status = SANE_STATUS_COVER_OPEN;
      }
  }

  if (hw->tpu)
  {
    if (TPU_STATUS_ERR & hw->tpu->status)
    {
      log_info ("TPU: other error");
      status = SANE_STATUS_INVAL;
    }
  }

  if (hw->fbf)
  {
    /* ??? is this for MFDs? */
    if (DV3_STATUS_OPN & hw->fbf->status)
    {
      log_info ("UNIT: Scanner Unit open");
      status = SANE_STATUS_COVER_OPEN;
    }
  }

  return status;
}


static SANE_Status
initialize (device *hw)
{
  SANE_Status status;
  u_char param[2];

  log_call ();

  if (!hw->cmd->initialize_scanner)
    return SANE_STATUS_GOOD;

  param[0] = ESC;
  param[1] = hw->cmd->initialize_scanner;

  channel_send (hw->channel, param, 2, &status);
  status = expect_ack (hw);

  return status;
}


static Epson_Scanner *first_handle = NULL;


static EpsonHdr
command (device *hw, const u_char * cmd, size_t cmd_size,
	 SANE_Status * status)
{
  EpsonHdr head;
  u_char *buf;
  int count;

  log_call ();

  if (NULL == (head = t_malloc (1,EpsonHdrRec)))
  {
    err_fatal ("%s", strerror (errno));
    *status = SANE_STATUS_NO_MEM;
    return (EpsonHdr) 0;
  }

  channel_send (hw->channel, cmd, cmd_size, status);

  if (SANE_STATUS_GOOD != *status)
  {
    /* this is necessary for the GT-8000. I don't know why, but
       it seems to fix the problem. It should not have any
       ill effects on other scanners.  */
    *status = SANE_STATUS_GOOD;
    channel_send (hw->channel, cmd, cmd_size, status);
    if (SANE_STATUS_GOOD != *status)
      return (EpsonHdr) 0;
  }

  buf = (u_char *) head;
  buf += channel_recv (hw->channel, buf, 4, status);

  if (SANE_STATUS_GOOD != *status)
  {
    delete (head);
    return (EpsonHdr) 0;
  }

  switch (head->code)
  {

  case NAK:
    /* fall through */
    /* !!! is this really sufficient to report an error ? */
  case ACK:
    break;			/* no need to read any more data after ACK or NAK */

  case STX:
    hw->status = head->status;

    if (SANE_STATUS_GOOD != *status)
    {
      delete (head);
      return (EpsonHdr) 0;
    }

    count = head->count2 * 255 + head->count1;
    log_info ("need to read %d data bytes", count);

    /* Grow head so it has enough space to hold an additional count
     * bytes of payload.  We can _not_ use t_realloc here.
     */
    if (NULL == (head = realloc (head, sizeof (EpsonHdrRec) + count)))
    {
      err_fatal ("%s", strerror (errno));
      *status = SANE_STATUS_NO_MEM;
      return (EpsonHdr) 0;
    }

    buf = head->buf;
    channel_recv (hw->channel, buf, count, status);

    if (SANE_STATUS_GOOD != *status)
    {
      delete (head);
      return (EpsonHdr) 0;
    }

    break;

  default:
    if (0 == head->code)
      { err_major ("Incompatible printer port (probably bi/directional)"); }
    else if (cmd[cmd_size - 1] == head->code)
      { err_major ("Incompatible printer port (probably not bi/directional)"); }

    err_major ("Illegal response of scanner for command: %02x", head->code);
    break;
  }

  return head;
}


static SANE_Status
create_epson_device (device **devp, channel* ch)
{
  SANE_Status status = SANE_STATUS_GOOD;
  device *dev = NULL;

  require (devp && ch);

  dev = t_calloc (1, device);
  if (!dev)
  {
    err_fatal ("%s", strerror (errno));
    return SANE_STATUS_NO_MEM;
  }

  dev->channel = ch;

  init_resolution_info (&dev->res  , NULL);
  init_resolution_info (&dev->res_x, NULL);
  init_resolution_info (&dev->res_y, NULL);

  dev->cmd = &epson_cmd[EPSON_LEVEL_DEFAULT];

  dev->fw_name = get_fw_name (dev->channel);

  {
    void *cfg  = cfg_init (NULL, NULL);
    dev->using_fs = !cfg_has_value (cfg, CFG_KEY_FS_BLACKLIST, dev->fw_name);
  }

  if (!dev->using_fs && CHAN_NET == ch->type)
    {
      err_fatal ("Network channel does not support scanning via ESC commands");
      return SANE_STATUS_IO_ERROR;
    }

  /* FIXME We should use this for all devices after the LP-M8040,
   *       maybe even for all devices that are using_fs.  For the
   *       truly bold, just use this for any supported device. 
   */
  if (0 == strcmp_c ("LP-M8040", dev->fw_name))
    {
      size_t protocol_max = (dev->using_fs ? UINT32_MAX : UINT16_MAX);

      ch->set_max_request_size (ch, min (protocol_max, 512 * 1024));
    }

  initialize (dev);

  if (dev->cmd->request_identity != 0)
  {
    status = get_identity_information (dev);
    if (status != SANE_STATUS_GOOD)
    {
      return status;
    }
  }

  if (dev->cmd->request_identity2 != 0)
  {
    get_hardware_property (dev);
    if (status != SANE_STATUS_GOOD)
    {
      return status;
    }
  }

  /* Check for the max. supported color depth and assign the values to
   * the bitDepthList.  Note that bitDepthList is a SANE word list and
   * therefore has an initial element that indicates the number of its
   * elements.  We store up to 2 bit depths (8 and only one of 16, 14,
   * and 12).
   */

  bitDepthList = t_malloc (1 + 2, SANE_Word);
  if (bitDepthList == NULL)
  {
    err_fatal ("%s", strerror (errno));
    return SANE_STATUS_NO_MEM;
  }

  bitDepthList[0] = 1;		/* we start with one element in the list */
  bitDepthList[1] = 8;		/* 8bit is the default */

  if (set_data_format (dev, 16) == SANE_STATUS_GOOD)
  {
    dev->maxDepth = 16;

    bitDepthList[0]++;
    bitDepthList[bitDepthList[0]] = 16;
  }
  else if (set_data_format (dev, 14) == SANE_STATUS_GOOD)
  {
    dev->maxDepth = 14;

    bitDepthList[0]++;
    bitDepthList[bitDepthList[0]] = 14;
  }
  else if (set_data_format (dev, 12) == SANE_STATUS_GOOD)
  {
    dev->maxDepth = 12;

    bitDepthList[0]++;
    bitDepthList[bitDepthList[0]] = 12;
  }
  else
  {
    dev->maxDepth = 8;
    /* the default depth is already in the list */
  }

  log_info ("maximum color depth = %d", dev->maxDepth);

  status = dev_request_extended_status (dev);
  if (SANE_STATUS_GOOD != status)
  {
    if (SANE_STATUS_UNSUPPORTED != status)
    {
      err_minor ("failure processing extended status request");
    }
  }
  else
  {
    const void *info = model_info_cache_get_info (dev->fw_name, &status);
    if (SANE_STATUS_GOOD == status && info)
    {
      dev->scan_hard = model_info_get_profile (info);
      model_info_customise_commands (info, dev->cmd);
      dev->uses_locking = model_info_has_lock_commands (info);
    }
    else
    {
      err_minor ("failure getting model info (%s)", sane_strstatus (status));
    }
  }

  if (0 == strcmp_c ("GT-8200", dev->fw_name))
  {
    /* Version 1.08 of the firmware is said to only report half of the
       vertical size of the scan area.  We should double that if it is
       smaller than the horizontal scan area.  Although we already try
       doing this by fixing up the extended status reply, that doesn't
       do the whole trick because we are dealing with a device type 0
       scanner.  In that case we get the FBF scan area via the 'ESC I'
       command which does not include model info so we can not safely
       fix up that reply.
     */
    if (dev->fbf && (dev->fbf->max_y < dev->fbf->max_x))
    {
      err_minor ("Fixing up buggy FBF max scan dimensions.");
      dev->fbf->max_y *= 2;
      update_ranges (dev, dev->fbf);
      dev->need_color_reorder = SANE_TRUE;
    }
  }

  if (dev->fbf)
  {
    log_info ("  FBF: TL (%.2f, %.2f) -- BR (%.2f, %.2f) [in mm]",
              SANE_UNFIX (dev->fbf->x_range.min),
              SANE_UNFIX (dev->fbf->y_range.min),
              SANE_UNFIX (dev->fbf->x_range.max),
              SANE_UNFIX (dev->fbf->y_range.max));

    // check for auto size detection support and cache the results
    dev->fbf->has_size_check = ((0 != dev->fbf->doc_x)
                                && (0 != dev->fbf->doc_y));
  }
  if (dev->adf)
  {
    log_info ("  ADF: TL (%.2f, %.2f) -- BR (%.2f, %.2f) [in mm]",
              SANE_UNFIX (dev->adf->x_range.min),
              SANE_UNFIX (dev->adf->y_range.min),
              SANE_UNFIX (dev->adf->x_range.max),
              SANE_UNFIX (dev->adf->y_range.max));
    log_info ("  ADF: %s, %s feed type",
              (EXT_STATUS_ADFS & dev->ext_status ? "duplex" : "simplex"),
              (EXT_STATUS_ADFT & dev->ext_status ? "page" : "sheet"));
    
    // check for auto size detection support and cache the results
    dev->adf->has_size_check = ((0 != dev->adf->doc_x)
                                && (0 != dev->adf->doc_y));
    
    // handle the special case where some scanners claim to have auto
    // document size detection support for flatbed but not for adf
    if (dev->fbf && (dev->fbf->has_size_check && !dev->adf->has_size_check))
      {
        cmd_control_option_unit (dev, 0x01); // turn on ADF
        dev_request_extended_status (dev);   // now re-query scanner
        // re-check for document size detection support
        dev->adf->has_size_check = ((0 != dev->adf->doc_x)
                                    && (0 != dev->adf->doc_y));
        cmd_control_option_unit (dev, 0x00); // turn off ADF
      }
  }
  if (dev->tpu)
  {
    log_info ("  TPU: TL (%.2f, %.2f) -- BR (%.2f, %.2f) [in mm]",
              SANE_UNFIX (dev->tpu->x_range.min),
              SANE_UNFIX (dev->tpu->y_range.min),
              SANE_UNFIX (dev->tpu->x_range.max),
              SANE_UNFIX (dev->tpu->y_range.max));
    
    // check for auto size detection support and cache the results
    dev->tpu->has_size_check = ((0 != dev->tpu->doc_x)
                                && (0 != dev->tpu->doc_y));
  }


  /* establish defaults */
  dev->need_reset_on_source_change = SANE_FALSE;

  if (strcmp_c ("ES-9000H", dev->fw_name) == 0 ||
      strcmp_c ("GT-30000", dev->fw_name) == 0)
  {
    dev->cmd->set_focus_position = 0;
    dev->cmd->feed = 0x19;
  }
  else if (strcmp_c ("GT-8200", dev->fw_name) == 0 ||
	   strcmp_c ("Perfection1640", dev->fw_name) == 0 ||
	   strcmp_c ("GT-8700", dev->fw_name) == 0)
  {
    dev->cmd->feed = 0;
    dev->cmd->set_focus_position = 0;
    dev->need_reset_on_source_change = SANE_TRUE;
  }

  {                             /* set up supported scan sources */
    int i = 0;
    if (dev->fbf) dev->sources[i++] = FBF_STR;
    if (dev->adf) dev->sources[i++] = ADF_STR;
    if (dev->tpu) dev->sources[i++] = TPU_STR;
    dev->sources[i] = NULL;    /* terminator */
  }

  if (!dev->src) dev->src = (extension *) dev->fbf;
  if (!dev->src) dev->src = (extension *) dev->adf;
  if (!dev->src) dev->src = (extension *) dev->tpu;

  require (dev->src);

  if (!using (dev, fbf))
    dev_set_option_unit (dev, false);

  dev->polling_time = DEFAULT_POLLING_TIME;
  if (push_button_needs_polling (dev)
      || maintenance_is_supported (dev))
  {
    dev->polling_time = (250 * 1000);
  }

  *devp = dev;

  return SANE_STATUS_GOOD;
} /* create epson device */


static SANE_Status
create_epson_scanner (device *dev, Epson_Scanner **scanner)
{
  Epson_Scanner *s = t_calloc (1, Epson_Scanner);
  
  if (!s)
  {
    err_fatal ("%s", strerror (errno));
    return SANE_STATUS_NO_MEM;
  }

  s->hw  = dev;
  s->src = &s->raw;

  if (s->hw->cmd->request_identity2 != 0)
  {
    /* reset constraints because we are pointing to different lists now
       FIXME: Only needs to be done the first time we (successfully)
       send the ESC i command.  This should be done when constructing
       the device and is therefore done by the time we construct a
       scanner.  While the content and size of the lists may vary
       depending on the selected option, the list nature is constant.
       Hmm, we may actually be zapping the lists ...
    */
    s->opt[OPT_X_RESOLUTION].constraint_type      = SANE_CONSTRAINT_WORD_LIST;
    s->opt[OPT_X_RESOLUTION].constraint.word_list = s->hw->res_x.list;
    s->opt[OPT_Y_RESOLUTION].constraint_type      = SANE_CONSTRAINT_WORD_LIST;
    s->opt[OPT_Y_RESOLUTION].constraint.word_list = s->hw->res_y.list;
  }

  s->frame_count = 0;

  promise (s->hw);
  promise (s->src);

  *scanner = s;
  return SANE_STATUS_GOOD;
}


/*! \todo Release resources in error recovery.
 */
static SANE_Status
create_sane_handle (Epson_Scanner **handle, const char *name, const void *dip)
{
  SANE_Status status = SANE_STATUS_GOOD;
  device *dev = NULL;
  Epson_Scanner *s = NULL;
  channel *ch = NULL;

  dev = NULL;

  ch = channel_create (name, &status);
  if (SANE_STATUS_GOOD != status) return status;
  
  ch->open (ch, &status);
  if (SANE_STATUS_GOOD != status) return status;
  
  status = create_epson_device (&dev, ch);
  if (SANE_STATUS_GOOD != status)
    return status;

  status = create_epson_scanner (dev, &s);
  if (!s || SANE_STATUS_GOOD != status)
    return status;

  s->dip = dip;
  init_options (s);

  *handle = s;
  return SANE_STATUS_GOOD;
}

static SANE_Status
init_options (Epson_Scanner * s)
{
  int i;
  SANE_Bool dummy;
  SANE_Bool reload;

  log_call ();

  for (i = 0; i < NUM_OPTIONS; ++i)
  {
    s->opt[i].size = sizeof (SANE_Word);
    s->opt[i].cap = SANE_CAP_SOFT_SELECT | SANE_CAP_SOFT_DETECT;
  }

  s->opt[OPT_NUM_OPTS].title = SANE_TITLE_NUM_OPTIONS;
  s->opt[OPT_NUM_OPTS].desc = SANE_DESC_NUM_OPTIONS;
  s->opt[OPT_NUM_OPTS].cap = SANE_CAP_SOFT_DETECT;
  s->val[OPT_NUM_OPTS].w = NUM_OPTIONS;

  /* "Scan Mode" group: */
  s->opt[OPT_MODE_GROUP].title = SANE_I18N ("Scan Mode");
  s->opt[OPT_MODE_GROUP].desc = "";
  s->opt[OPT_MODE_GROUP].type = SANE_TYPE_GROUP;
  s->opt[OPT_MODE_GROUP].cap = 0;

  /* scan mode */
  s->opt[OPT_MODE].name = SANE_NAME_SCAN_MODE;
  s->opt[OPT_MODE].title = SANE_TITLE_SCAN_MODE;
  s->opt[OPT_MODE].desc = SANE_DESC_SCAN_MODE;
  s->opt[OPT_MODE].type = SANE_TYPE_STRING;
  s->opt[OPT_MODE].size = max_string_size (mode_list);
  s->opt[OPT_MODE].constraint_type = SANE_CONSTRAINT_STRING_LIST;
  s->opt[OPT_MODE].constraint.string_list = mode_list;
  s->val[OPT_MODE].w = 2;	/* Color */

  /* bit depth */
  s->opt[OPT_BIT_DEPTH].name = SANE_NAME_BIT_DEPTH;
  s->opt[OPT_BIT_DEPTH].title = SANE_TITLE_BIT_DEPTH;
  s->opt[OPT_BIT_DEPTH].desc = SANE_DESC_BIT_DEPTH;
  s->opt[OPT_BIT_DEPTH].type = SANE_TYPE_INT;
  s->opt[OPT_BIT_DEPTH].unit = SANE_UNIT_NONE;
  s->opt[OPT_BIT_DEPTH].constraint_type = SANE_CONSTRAINT_WORD_LIST;
  s->opt[OPT_BIT_DEPTH].constraint.word_list = bitDepthList;
  s->opt[OPT_BIT_DEPTH].cap |= SANE_CAP_INACTIVE;
  s->val[OPT_BIT_DEPTH].w = bitDepthList[1];	/* the first "real" element is the default */

  if (bitDepthList[0] == 1)	/* only one element in the list -> hide the option */
    s->opt[OPT_BIT_DEPTH].cap |= SANE_CAP_INACTIVE;

  /* halftone */
  s->opt[OPT_HALFTONE].name = SANE_NAME_HALFTONE;
  s->opt[OPT_HALFTONE].title = SANE_TITLE_HALFTONE;
  s->opt[OPT_HALFTONE].desc = SANE_I18N ("Selects the halftone.");

  s->opt[OPT_HALFTONE].type = SANE_TYPE_STRING;
  s->opt[OPT_HALFTONE].size = max_string_size (halftone_list_7);
  s->opt[OPT_HALFTONE].constraint_type = SANE_CONSTRAINT_STRING_LIST;

  if (s->hw->level >= 7)
    s->opt[OPT_HALFTONE].constraint.string_list = halftone_list_7;
  else if (s->hw->level >= 4)
    s->opt[OPT_HALFTONE].constraint.string_list = halftone_list_4;
  else
    s->opt[OPT_HALFTONE].constraint.string_list = halftone_list;

  s->val[OPT_HALFTONE].w = 1;	/* Halftone A */

  if (!s->hw->cmd->set_halftoning)
  {
    s->opt[OPT_HALFTONE].cap |= SANE_CAP_INACTIVE;
  }

  /* dropout */
  s->opt[OPT_DROPOUT].name = "dropout";
  s->opt[OPT_DROPOUT].title = SANE_I18N ("Dropout");
  s->opt[OPT_DROPOUT].desc = SANE_I18N ("Selects the dropout.");

  s->opt[OPT_DROPOUT].type = SANE_TYPE_STRING;
  s->opt[OPT_DROPOUT].size = max_string_size (dropout_list);
  s->opt[OPT_DROPOUT].cap |= SANE_CAP_ADVANCED;
  s->opt[OPT_DROPOUT].constraint_type = SANE_CONSTRAINT_STRING_LIST;
  s->opt[OPT_DROPOUT].constraint.string_list = dropout_list;
  s->val[OPT_DROPOUT].w = 0;	/* None */

  /* brightness algorithm selection */
  s->opt[OPT_BRIGHTNESS_METHOD].name  = "brightness-method";
  s->opt[OPT_BRIGHTNESS_METHOD].title = SANE_I18N ("Brightness Method");
  s->opt[OPT_BRIGHTNESS_METHOD].desc  = SANE_I18N ("Selects a method to change the brightness of the acquired image.");
  s->opt[OPT_BRIGHTNESS_METHOD].type  = SANE_TYPE_STRING;
  s->opt[OPT_BRIGHTNESS_METHOD].unit  = SANE_UNIT_NONE;
  s->opt[OPT_BRIGHTNESS_METHOD].size  = max_string_size (brightness_method_list);
  s->opt[OPT_BRIGHTNESS_METHOD].cap  |= SANE_CAP_ADVANCED;
  s->opt[OPT_BRIGHTNESS_METHOD].constraint_type        = SANE_CONSTRAINT_STRING_LIST;
  s->opt[OPT_BRIGHTNESS_METHOD].constraint.string_list = brightness_method_list;  s->val[OPT_BRIGHTNESS_METHOD].w = 0; /* first supported */

  if (!s->hw->cmd->set_bright)         /* skip "hardware" */
    ++s->opt[OPT_BRIGHTNESS_METHOD].constraint.string_list;

  /* brightness */
  s->opt[OPT_BRIGHTNESS].name  = SANE_NAME_BRIGHTNESS;
  s->opt[OPT_BRIGHTNESS].title = SANE_TITLE_BRIGHTNESS;
  s->opt[OPT_BRIGHTNESS].desc  = SANE_DESC_BRIGHTNESS;
  s->opt[OPT_BRIGHTNESS].type  = SANE_TYPE_INT;
  s->opt[OPT_BRIGHTNESS].unit  = SANE_UNIT_NONE;
  s->opt[OPT_BRIGHTNESS].size  = sizeof (SANE_Int);
  s->opt[OPT_BRIGHTNESS].constraint_type = SANE_CONSTRAINT_RANGE;
  s->val[OPT_BRIGHTNESS].w = 0; /* neutral */

  if (s->hw->cmd->set_bright)
    {
      s->opt[OPT_BRIGHTNESS].constraint.range = &s->hw->cmd->bright_range;
    }
  else
    {
      s->opt[OPT_BRIGHTNESS].cap |= SANE_CAP_EMULATED;
      s->opt[OPT_BRIGHTNESS].constraint.range = &brightness_range;
    }

  /* contrast */
  s->opt[OPT_CONTRAST].name  = SANE_NAME_CONTRAST;
  s->opt[OPT_CONTRAST].title = SANE_TITLE_CONTRAST;
  s->opt[OPT_CONTRAST].desc  = SANE_DESC_CONTRAST;
  s->opt[OPT_CONTRAST].type  = SANE_TYPE_INT;
  s->opt[OPT_CONTRAST].unit  = SANE_UNIT_NONE;
  s->opt[OPT_CONTRAST].size  = sizeof (SANE_Int);
  s->opt[OPT_CONTRAST].cap  |= SANE_CAP_EMULATED;
  s->opt[OPT_CONTRAST].constraint_type  = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_CONTRAST].constraint.range = &contrast_range;
  s->val[OPT_CONTRAST].w = 0;   /* neutral */

  /* sharpness */
  s->opt[OPT_SHARPNESS].name = "sharpness";
  s->opt[OPT_SHARPNESS].title = SANE_I18N ("Sharpness");
  s->opt[OPT_SHARPNESS].desc = "";

  s->opt[OPT_SHARPNESS].type = SANE_TYPE_INT;
  s->opt[OPT_SHARPNESS].unit = SANE_UNIT_NONE;
  s->opt[OPT_SHARPNESS].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_SHARPNESS].constraint.range = &outline_emphasis_range;
  s->val[OPT_SHARPNESS].w = 0;	/* Normal */

  if (!s->hw->cmd->set_outline_emphasis)
  {
    s->opt[OPT_SHARPNESS].cap |= SANE_CAP_INACTIVE;
  }

  /* gamma */
  s->opt[OPT_GAMMA_CORRECTION].name = SANE_NAME_GAMMA_CORRECTION;
  s->opt[OPT_GAMMA_CORRECTION].title = SANE_TITLE_GAMMA_CORRECTION;
  s->opt[OPT_GAMMA_CORRECTION].desc = SANE_DESC_GAMMA_CORRECTION;

  s->opt[OPT_GAMMA_CORRECTION].type = SANE_TYPE_STRING;
  s->opt[OPT_GAMMA_CORRECTION].constraint_type = SANE_CONSTRAINT_STRING_LIST;
  /* 
   * special handling for D1 function level - at this time I'm not
   * testing for D1, I'm just assuming that all D level scanners will
   * behave the same way. This has to be confirmed with the next D-level
   * scanner 
   */
  if (s->hw->cmd->level[0] == 'D')
  {
    s->opt[OPT_GAMMA_CORRECTION].size = max_string_size (gamma_list_d);
    s->opt[OPT_GAMMA_CORRECTION].constraint.string_list = gamma_list_d;
    s->val[OPT_GAMMA_CORRECTION].w = 1;	/* Default */
    s->hw->gamma_user_defined = gamma_userdefined_d;
    s->hw->gamma_type = gamma_params_d;
  }
  else
  {
    s->opt[OPT_GAMMA_CORRECTION].size = max_string_size (gamma_list_ab);
    s->opt[OPT_GAMMA_CORRECTION].constraint.string_list = gamma_list_ab;
    s->val[OPT_GAMMA_CORRECTION].w = 0;	/* Default */
    s->hw->gamma_user_defined = gamma_userdefined_ab;
    s->hw->gamma_type = gamma_params_ab;
  }

  if (!s->hw->cmd->set_gamma)
  {
    s->opt[OPT_GAMMA_CORRECTION].cap |= SANE_CAP_INACTIVE;
  }

  /* red gamma vector */
  s->opt[OPT_GAMMA_VECTOR_R].name = SANE_NAME_GAMMA_VECTOR_R;
  s->opt[OPT_GAMMA_VECTOR_R].title = SANE_TITLE_GAMMA_VECTOR_R;
  s->opt[OPT_GAMMA_VECTOR_R].desc = SANE_DESC_GAMMA_VECTOR_R;

  s->opt[OPT_GAMMA_VECTOR_R].type = SANE_TYPE_INT;
  s->opt[OPT_GAMMA_VECTOR_R].unit = SANE_UNIT_NONE;
  s->opt[OPT_GAMMA_VECTOR_R].size = 256 * sizeof (SANE_Word);
  s->opt[OPT_GAMMA_VECTOR_R].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_GAMMA_VECTOR_R].constraint.range = &u8_range;
  s->val[OPT_GAMMA_VECTOR_R].wa = &s->gamma_table[0][0];

  /* green gamma vector */
  s->opt[OPT_GAMMA_VECTOR_G].name = SANE_NAME_GAMMA_VECTOR_G;
  s->opt[OPT_GAMMA_VECTOR_G].title = SANE_TITLE_GAMMA_VECTOR_G;
  s->opt[OPT_GAMMA_VECTOR_G].desc = SANE_DESC_GAMMA_VECTOR_G;

  s->opt[OPT_GAMMA_VECTOR_G].type = SANE_TYPE_INT;
  s->opt[OPT_GAMMA_VECTOR_G].unit = SANE_UNIT_NONE;
  s->opt[OPT_GAMMA_VECTOR_G].size = 256 * sizeof (SANE_Word);
  s->opt[OPT_GAMMA_VECTOR_G].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_GAMMA_VECTOR_G].constraint.range = &u8_range;
  s->val[OPT_GAMMA_VECTOR_G].wa = &s->gamma_table[1][0];

  /* red gamma vector */
  s->opt[OPT_GAMMA_VECTOR_B].name = SANE_NAME_GAMMA_VECTOR_B;
  s->opt[OPT_GAMMA_VECTOR_B].title = SANE_TITLE_GAMMA_VECTOR_B;
  s->opt[OPT_GAMMA_VECTOR_B].desc = SANE_DESC_GAMMA_VECTOR_B;

  s->opt[OPT_GAMMA_VECTOR_B].type = SANE_TYPE_INT;
  s->opt[OPT_GAMMA_VECTOR_B].unit = SANE_UNIT_NONE;
  s->opt[OPT_GAMMA_VECTOR_B].size = 256 * sizeof (SANE_Word);
  s->opt[OPT_GAMMA_VECTOR_B].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_GAMMA_VECTOR_B].constraint.range = &u8_range;
  s->val[OPT_GAMMA_VECTOR_B].wa = &s->gamma_table[2][0];

  if (s->hw->cmd->set_gamma_table &&
      s->hw->gamma_user_defined[s->val[OPT_GAMMA_CORRECTION].w])
  {
    s->opt[OPT_GAMMA_VECTOR_R].cap &= ~SANE_CAP_INACTIVE;
    s->opt[OPT_GAMMA_VECTOR_G].cap &= ~SANE_CAP_INACTIVE;
    s->opt[OPT_GAMMA_VECTOR_B].cap &= ~SANE_CAP_INACTIVE;
  }
  else
  {
    s->opt[OPT_GAMMA_VECTOR_R].cap |= SANE_CAP_INACTIVE;
    s->opt[OPT_GAMMA_VECTOR_G].cap |= SANE_CAP_INACTIVE;
    s->opt[OPT_GAMMA_VECTOR_B].cap |= SANE_CAP_INACTIVE;
  }

  /* initialize the Gamma tables */
  memset (&s->gamma_table[0], 0, 256 * sizeof (SANE_Word));
  memset (&s->gamma_table[1], 0, 256 * sizeof (SANE_Word));
  memset (&s->gamma_table[2], 0, 256 * sizeof (SANE_Word));
  for (i = 0; i < 256; i++)
  {
    s->gamma_table[0][i] = i;
    s->gamma_table[1][i] = i;
    s->gamma_table[2][i] = i;
  }

  /* color correction */
  s->opt[OPT_COLOR_CORRECTION].name = "color-correction";
  s->opt[OPT_COLOR_CORRECTION].title = SANE_I18N ("Color correction");
  s->opt[OPT_COLOR_CORRECTION].desc =
    SANE_I18N
    ("Sets the color correction table for the selected output device.");

  s->opt[OPT_COLOR_CORRECTION].type = SANE_TYPE_STRING;
  s->opt[OPT_COLOR_CORRECTION].size = 32;
  s->opt[OPT_COLOR_CORRECTION].cap |= SANE_CAP_ADVANCED;
  s->opt[OPT_COLOR_CORRECTION].constraint_type = SANE_CONSTRAINT_STRING_LIST;
  if (s->hw->cmd->level[0] == 'D')
  {
    s->opt[OPT_COLOR_CORRECTION].size = max_string_size (color_list_d);
    s->opt[OPT_COLOR_CORRECTION].constraint.string_list = color_list_d;
    s->val[OPT_COLOR_CORRECTION].w = 0;	/* Default */
    s->hw->color_user_defined = color_userdefined_d;
    s->hw->color_type = color_params_d;
  }
  else
  {
    s->opt[OPT_COLOR_CORRECTION].size = max_string_size (color_list_ab);
    s->opt[OPT_COLOR_CORRECTION].constraint.string_list = color_list_ab;
    s->val[OPT_COLOR_CORRECTION].w = 5;	/* scanner default: CRT monitors */
    s->hw->color_user_defined = color_userdefined_ab;
    s->hw->color_type = color_params_ab;
  }

  if (!s->hw->cmd->set_color_correction)
  {
    s->opt[OPT_COLOR_CORRECTION].cap |= SANE_CAP_INACTIVE;
  }

  /* resolution */
  s->opt[OPT_RESOLUTION].name  = SANE_NAME_SCAN_RESOLUTION;
  s->opt[OPT_RESOLUTION].title = SANE_TITLE_SCAN_RESOLUTION;
  s->opt[OPT_RESOLUTION].desc  = SANE_DESC_SCAN_RESOLUTION;
  s->opt[OPT_RESOLUTION].type  = SANE_TYPE_INT;
  s->opt[OPT_RESOLUTION].unit  = SANE_UNIT_DPI;

  if (s->hw->cmd->level[0] != 'D')
  {
    s->opt[OPT_RESOLUTION].constraint_type  = SANE_CONSTRAINT_RANGE;
    s->opt[OPT_RESOLUTION].constraint.range = &s->hw->dpi_range;
  }
  else
  {
    s->opt[OPT_RESOLUTION].constraint_type      = SANE_CONSTRAINT_WORD_LIST;
    s->opt[OPT_RESOLUTION].constraint.word_list = s->hw->res.list;
  }
  handle_resolution (s, OPT_RESOLUTION, DEFAULT_RESOLUTION);

#undef  SANE_NAME_SCAN_X_RESOLUTION
#define SANE_NAME_SCAN_X_RESOLUTION "x-resolution"

  /* resolution in main scan direction */
  s->opt[OPT_X_RESOLUTION].name  = SANE_NAME_SCAN_X_RESOLUTION;
  s->opt[OPT_X_RESOLUTION].title = SANE_TITLE_SCAN_X_RESOLUTION;
  s->opt[OPT_X_RESOLUTION].desc  = SANE_DESC_SCAN_X_RESOLUTION;
  s->opt[OPT_X_RESOLUTION].type  = SANE_TYPE_INT;
  s->opt[OPT_X_RESOLUTION].unit  = SANE_UNIT_DPI;

  if (s->hw->cmd->level[0] != 'D')
  {
    s->opt[OPT_X_RESOLUTION].constraint_type  = SANE_CONSTRAINT_RANGE;
    s->opt[OPT_X_RESOLUTION].constraint.range = &s->hw->dpi_range;
  }
  else
  {
    s->opt[OPT_X_RESOLUTION].constraint_type      = SANE_CONSTRAINT_WORD_LIST;
    s->opt[OPT_X_RESOLUTION].constraint.word_list = s->hw->res_x.list;
    s->opt[OPT_X_RESOLUTION].cap |= SANE_CAP_ADVANCED;
  }
  handle_resolution (s, OPT_X_RESOLUTION, DEFAULT_X_RESOLUTION);

  /* resolution in sub scan direction */
  s->opt[OPT_Y_RESOLUTION].name  = SANE_NAME_SCAN_Y_RESOLUTION;
  s->opt[OPT_Y_RESOLUTION].title = SANE_TITLE_SCAN_Y_RESOLUTION;
  s->opt[OPT_Y_RESOLUTION].desc  = SANE_DESC_SCAN_Y_RESOLUTION;
  s->opt[OPT_Y_RESOLUTION].type  = SANE_TYPE_INT;
  s->opt[OPT_Y_RESOLUTION].unit  = SANE_UNIT_DPI;

  if (s->hw->cmd->level[0] != 'D')
  {
    s->opt[OPT_Y_RESOLUTION].constraint_type  = SANE_CONSTRAINT_RANGE;
    s->opt[OPT_Y_RESOLUTION].constraint.range = &s->hw->dpi_range;
  }
  else
  {
    s->opt[OPT_Y_RESOLUTION].constraint_type      = SANE_CONSTRAINT_WORD_LIST;
    s->opt[OPT_Y_RESOLUTION].constraint.word_list = s->hw->res_y.list;
    s->opt[OPT_Y_RESOLUTION].cap |= SANE_CAP_ADVANCED;
  }
  handle_resolution (s, OPT_Y_RESOLUTION, DEFAULT_Y_RESOLUTION);

  switch (s->hw->productID)
  {
    /* We already use kludges for a number of models and don't want to
       be burdened with rechecking their functionality.  Really fixing
       their support requires more changes than merely adding main/sub
       resolution lists.
     */
  case 0x0116:
  case 0x0118:
  case 0x0119:
  case 0x012b:
    s->opt[OPT_X_RESOLUTION].cap |= SANE_CAP_INACTIVE;
    s->opt[OPT_Y_RESOLUTION].cap |= SANE_CAP_INACTIVE;
    break;
  default:
    /* use the default capability set at the top of this function */
    ;
  }

  /* threshold */
  s->opt[OPT_THRESHOLD].name = SANE_NAME_THRESHOLD;
  s->opt[OPT_THRESHOLD].title = SANE_TITLE_THRESHOLD;
  s->opt[OPT_THRESHOLD].desc = SANE_DESC_THRESHOLD;

  s->opt[OPT_THRESHOLD].type = SANE_TYPE_INT;
  s->opt[OPT_THRESHOLD].unit = SANE_UNIT_NONE;
  s->opt[OPT_THRESHOLD].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_THRESHOLD].constraint.range = &u8_range;
  s->val[OPT_THRESHOLD].w = 0x80;

  if (!s->hw->cmd->set_threshold)
  {
    s->opt[OPT_THRESHOLD].cap |= SANE_CAP_INACTIVE;
  }

  s->opt[OPT_CCT_GROUP].title = SANE_I18N ("Color correction coefficients");
  s->opt[OPT_CCT_GROUP].desc = SANE_I18N ("Matrix multiplication of RGB");
  s->opt[OPT_CCT_GROUP].type = SANE_TYPE_GROUP;
  s->opt[OPT_CCT_GROUP].cap = SANE_CAP_ADVANCED;

  /* color correction coefficients */
  s->opt[OPT_CCT_1].name = "cct-1";
  s->opt[OPT_CCT_2].name = "cct-2";
  s->opt[OPT_CCT_3].name = "cct-3";
  s->opt[OPT_CCT_4].name = "cct-4";
  s->opt[OPT_CCT_5].name = "cct-5";
  s->opt[OPT_CCT_6].name = "cct-6";
  s->opt[OPT_CCT_7].name = "cct-7";
  s->opt[OPT_CCT_8].name = "cct-8";
  s->opt[OPT_CCT_9].name = "cct-9";

  s->opt[OPT_CCT_1].title = SANE_I18N ("Red");
  s->opt[OPT_CCT_2].title = SANE_I18N ("Shift green to red");
  s->opt[OPT_CCT_3].title = SANE_I18N ("Shift blue to red");
  s->opt[OPT_CCT_4].title = SANE_I18N ("Shift red to green");
  s->opt[OPT_CCT_5].title = SANE_I18N ("Green");
  s->opt[OPT_CCT_6].title = SANE_I18N ("Shift blue to green");
  s->opt[OPT_CCT_7].title = SANE_I18N ("Shift red to blue");
  s->opt[OPT_CCT_8].title = SANE_I18N ("Shift green to blue");
  s->opt[OPT_CCT_9].title = SANE_I18N ("Blue");

  s->opt[OPT_CCT_1].desc = SANE_I18N ("Controls red level");
  s->opt[OPT_CCT_2].desc = SANE_I18N ("Adds to red based on green level");
  s->opt[OPT_CCT_3].desc = SANE_I18N ("Adds to red based on blue level");
  s->opt[OPT_CCT_4].desc = SANE_I18N ("Adds to green based on red level");
  s->opt[OPT_CCT_5].desc = SANE_I18N ("Controls green level");
  s->opt[OPT_CCT_6].desc = SANE_I18N ("Adds to green based on blue level");
  s->opt[OPT_CCT_7].desc = SANE_I18N ("Adds to blue based on red level");
  s->opt[OPT_CCT_8].desc = SANE_I18N ("Adds to blue based on green level");
  s->opt[OPT_CCT_9].desc = SANE_I18N ("Control blue level");

  s->opt[OPT_CCT_1].type = SANE_TYPE_FIXED;
  s->opt[OPT_CCT_2].type = SANE_TYPE_FIXED;
  s->opt[OPT_CCT_3].type = SANE_TYPE_FIXED;
  s->opt[OPT_CCT_4].type = SANE_TYPE_FIXED;
  s->opt[OPT_CCT_5].type = SANE_TYPE_FIXED;
  s->opt[OPT_CCT_6].type = SANE_TYPE_FIXED;
  s->opt[OPT_CCT_7].type = SANE_TYPE_FIXED;
  s->opt[OPT_CCT_8].type = SANE_TYPE_FIXED;
  s->opt[OPT_CCT_9].type = SANE_TYPE_FIXED;

  s->opt[OPT_CCT_1].cap |= SANE_CAP_ADVANCED;
  s->opt[OPT_CCT_2].cap |= SANE_CAP_ADVANCED;
  s->opt[OPT_CCT_3].cap |= SANE_CAP_ADVANCED;
  s->opt[OPT_CCT_4].cap |= SANE_CAP_ADVANCED;
  s->opt[OPT_CCT_5].cap |= SANE_CAP_ADVANCED;
  s->opt[OPT_CCT_6].cap |= SANE_CAP_ADVANCED;
  s->opt[OPT_CCT_7].cap |= SANE_CAP_ADVANCED;
  s->opt[OPT_CCT_8].cap |= SANE_CAP_ADVANCED;
  s->opt[OPT_CCT_9].cap |= SANE_CAP_ADVANCED;

  s->opt[OPT_CCT_1].cap |= SANE_CAP_INACTIVE;
  s->opt[OPT_CCT_2].cap |= SANE_CAP_INACTIVE;
  s->opt[OPT_CCT_3].cap |= SANE_CAP_INACTIVE;
  s->opt[OPT_CCT_4].cap |= SANE_CAP_INACTIVE;
  s->opt[OPT_CCT_5].cap |= SANE_CAP_INACTIVE;
  s->opt[OPT_CCT_6].cap |= SANE_CAP_INACTIVE;
  s->opt[OPT_CCT_7].cap |= SANE_CAP_INACTIVE;
  s->opt[OPT_CCT_8].cap |= SANE_CAP_INACTIVE;
  s->opt[OPT_CCT_9].cap |= SANE_CAP_INACTIVE;

  if (!s->hw->cmd->set_color_correction_coefficients)
    {
      s->opt[OPT_CCT_1].cap |= SANE_CAP_EMULATED;
      s->opt[OPT_CCT_2].cap |= SANE_CAP_EMULATED;
      s->opt[OPT_CCT_3].cap |= SANE_CAP_EMULATED;
      s->opt[OPT_CCT_4].cap |= SANE_CAP_EMULATED;
      s->opt[OPT_CCT_5].cap |= SANE_CAP_EMULATED;
      s->opt[OPT_CCT_6].cap |= SANE_CAP_EMULATED;
      s->opt[OPT_CCT_7].cap |= SANE_CAP_EMULATED;
      s->opt[OPT_CCT_8].cap |= SANE_CAP_EMULATED;
      s->opt[OPT_CCT_9].cap |= SANE_CAP_EMULATED;
    }

  s->opt[OPT_CCT_1].unit = SANE_UNIT_NONE;
  s->opt[OPT_CCT_2].unit = SANE_UNIT_NONE;
  s->opt[OPT_CCT_3].unit = SANE_UNIT_NONE;
  s->opt[OPT_CCT_4].unit = SANE_UNIT_NONE;
  s->opt[OPT_CCT_5].unit = SANE_UNIT_NONE;
  s->opt[OPT_CCT_6].unit = SANE_UNIT_NONE;
  s->opt[OPT_CCT_7].unit = SANE_UNIT_NONE;
  s->opt[OPT_CCT_8].unit = SANE_UNIT_NONE;
  s->opt[OPT_CCT_9].unit = SANE_UNIT_NONE;

  s->opt[OPT_CCT_1].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_CCT_2].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_CCT_3].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_CCT_4].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_CCT_5].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_CCT_6].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_CCT_7].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_CCT_8].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_CCT_9].constraint_type = SANE_CONSTRAINT_RANGE;

  s->hw->matrix_range.min = SANE_FIX (-2.0);
  s->hw->matrix_range.max = SANE_FIX (2.0);
  s->hw->matrix_range.quant = 0;

  s->opt[OPT_CCT_1].constraint.range = &s->hw->matrix_range;
  s->opt[OPT_CCT_2].constraint.range = &s->hw->matrix_range;
  s->opt[OPT_CCT_3].constraint.range = &s->hw->matrix_range;
  s->opt[OPT_CCT_4].constraint.range = &s->hw->matrix_range;
  s->opt[OPT_CCT_5].constraint.range = &s->hw->matrix_range;
  s->opt[OPT_CCT_6].constraint.range = &s->hw->matrix_range;
  s->opt[OPT_CCT_7].constraint.range = &s->hw->matrix_range;
  s->opt[OPT_CCT_8].constraint.range = &s->hw->matrix_range;
  s->opt[OPT_CCT_9].constraint.range = &s->hw->matrix_range;

  s->val[OPT_CCT_1].w = SANE_FIX (1.0);
  s->val[OPT_CCT_2].w = 0;
  s->val[OPT_CCT_3].w = 0;
  s->val[OPT_CCT_4].w = 0;
  s->val[OPT_CCT_5].w = SANE_FIX (1.0);
  s->val[OPT_CCT_6].w = 0;
  s->val[OPT_CCT_7].w = 0;
  s->val[OPT_CCT_8].w = 0;
  s->val[OPT_CCT_9].w = SANE_FIX (1.0);

  /* "Advanced" group: */
  s->opt[OPT_ADVANCED_GROUP].title = SANE_I18N ("Advanced");
  s->opt[OPT_ADVANCED_GROUP].desc = "";
  s->opt[OPT_ADVANCED_GROUP].type = SANE_TYPE_GROUP;
  s->opt[OPT_ADVANCED_GROUP].cap = SANE_CAP_ADVANCED;

  /* mirror */
  s->opt[OPT_MIRROR].name = "mirror";
  s->opt[OPT_MIRROR].title = SANE_I18N ("Mirror image");
  s->opt[OPT_MIRROR].desc = SANE_I18N ("Mirror the image.");

  s->opt[OPT_MIRROR].type = SANE_TYPE_BOOL;
  s->val[OPT_MIRROR].w = SANE_FALSE;

  if (!s->hw->cmd->mirror_image)
  {
    s->opt[OPT_MIRROR].cap |= SANE_CAP_INACTIVE;
  }

  /* speed */
  s->opt[OPT_SPEED].name = SANE_NAME_SCAN_SPEED;
  s->opt[OPT_SPEED].title = SANE_TITLE_SCAN_SPEED;
  s->opt[OPT_SPEED].desc = SANE_DESC_SCAN_SPEED;

  s->opt[OPT_SPEED].type = SANE_TYPE_BOOL;
  s->val[OPT_SPEED].w = SANE_FALSE;

  if (!s->hw->cmd->set_speed)
  {
    s->opt[OPT_SPEED].cap |= SANE_CAP_INACTIVE;
  }

  /* preview speed */
  s->opt[OPT_PREVIEW_SPEED].name = "preview-speed";
  s->opt[OPT_PREVIEW_SPEED].title = SANE_I18N ("Speed");
  s->opt[OPT_PREVIEW_SPEED].desc = "";

  s->opt[OPT_PREVIEW_SPEED].type = SANE_TYPE_BOOL;
  s->val[OPT_PREVIEW_SPEED].w = SANE_FALSE;

  if (!s->hw->cmd->set_speed)
  {
    s->opt[OPT_PREVIEW_SPEED].cap |= SANE_CAP_INACTIVE;
  }

  /* auto area segmentation */
  s->opt[OPT_AAS].name = "auto-area-segmentation";
  s->opt[OPT_AAS].title = SANE_I18N ("Auto area segmentation");
  s->opt[OPT_AAS].desc = "";

  s->opt[OPT_AAS].type = SANE_TYPE_BOOL;
  s->val[OPT_AAS].w = SANE_TRUE;

  if (!s->hw->cmd->control_auto_area_segmentation)
  {
    s->opt[OPT_AAS].cap |= SANE_CAP_INACTIVE;
  }

  /* limit resolution list */
  s->opt[OPT_LIMIT_RESOLUTION].name = "short-resolution";
  s->opt[OPT_LIMIT_RESOLUTION].title = SANE_I18N ("Short resolution list");
  s->opt[OPT_LIMIT_RESOLUTION].desc =
    SANE_I18N ("Display short resolution list");
  s->opt[OPT_LIMIT_RESOLUTION].type = SANE_TYPE_BOOL;
  s->val[OPT_LIMIT_RESOLUTION].w = SANE_FALSE;

  if (SANE_CONSTRAINT_WORD_LIST != s->opt[OPT_RESOLUTION].constraint_type)
  {
    s->opt[OPT_LIMIT_RESOLUTION].cap |= SANE_CAP_INACTIVE;
  }

  /* zoom */
  s->opt[OPT_ZOOM].name = "zoom";
  s->opt[OPT_ZOOM].title = SANE_I18N ("Zoom");
  s->opt[OPT_ZOOM].desc =
    SANE_I18N ("Defines the zoom factor the scanner will use");

  s->opt[OPT_ZOOM].type = SANE_TYPE_INT;
  s->opt[OPT_ZOOM].unit = SANE_UNIT_NONE;
  s->opt[OPT_ZOOM].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_ZOOM].constraint.range = &zoom_range;
  s->val[OPT_ZOOM].w = 100;

  if (s->hw->using_fs) s->hw->cmd->set_zoom = 0;

  if (!s->hw->cmd->set_zoom)
  {
    s->opt[OPT_ZOOM].cap |= SANE_CAP_INACTIVE;
  }

  /* "Preview settings" group: */
  s->opt[OPT_PREVIEW_GROUP].title = SANE_TITLE_PREVIEW;
  s->opt[OPT_PREVIEW_GROUP].desc = "";
  s->opt[OPT_PREVIEW_GROUP].type = SANE_TYPE_GROUP;
  s->opt[OPT_PREVIEW_GROUP].cap = SANE_CAP_ADVANCED;

  /* preview */
  s->opt[OPT_PREVIEW].name = SANE_NAME_PREVIEW;
  s->opt[OPT_PREVIEW].title = SANE_TITLE_PREVIEW;
  s->opt[OPT_PREVIEW].desc = SANE_DESC_PREVIEW;

  s->opt[OPT_PREVIEW].type = SANE_TYPE_BOOL;
  s->val[OPT_PREVIEW].w = SANE_FALSE;

  /* "Geometry" group: */
  s->opt[OPT_GEOMETRY_GROUP].title = SANE_I18N ("Geometry");
  s->opt[OPT_GEOMETRY_GROUP].desc = "";
  s->opt[OPT_GEOMETRY_GROUP].type = SANE_TYPE_GROUP;
  s->opt[OPT_GEOMETRY_GROUP].cap = SANE_CAP_ADVANCED;

  /* media size oriented scan area setting */
  s->opt[OPT_SCAN_AREA].name  = "scan-area";
  s->opt[OPT_SCAN_AREA].title = SANE_I18N ("Scan area");
  s->opt[OPT_SCAN_AREA].desc  =
    SANE_I18N ("Select an area to scan based on well-known media sizes.");
  s->opt[OPT_SCAN_AREA].type = SANE_TYPE_STRING;
  s->opt[OPT_SCAN_AREA].size = 0;
  s->opt[OPT_SCAN_AREA].cap |= SANE_CAP_INACTIVE;
  s->opt[OPT_SCAN_AREA].constraint_type = SANE_CONSTRAINT_STRING_LIST;
  s->opt[OPT_SCAN_AREA].constraint.string_list = NULL;
  s->val[OPT_SCAN_AREA].w = 0;

  /* Quick format */
  s->opt[OPT_QUICK_FORMAT].name = "quick-format";
  s->opt[OPT_QUICK_FORMAT].title = SANE_I18N ("Quick format");
  s->opt[OPT_QUICK_FORMAT].desc =
    SANE_I18N ("Select an area to scan based on well-known media sizes. (DEPRECATED)");
  s->opt[OPT_QUICK_FORMAT].type = SANE_TYPE_STRING;
  s->opt[OPT_QUICK_FORMAT].size = 0;
  s->opt[OPT_QUICK_FORMAT].cap |= SANE_CAP_INACTIVE;
  s->opt[OPT_QUICK_FORMAT].constraint_type = SANE_CONSTRAINT_STRING_LIST;
  s->opt[OPT_QUICK_FORMAT].constraint.string_list = NULL;
  s->val[OPT_QUICK_FORMAT].w = 0;

  handle_scan_area (s, 0);      /* divines device/setting dependent bits */

  /* top-left x */
  s->opt[OPT_TL_X].name = SANE_NAME_SCAN_TL_X;
  s->opt[OPT_TL_X].title = SANE_TITLE_SCAN_TL_X;
  s->opt[OPT_TL_X].desc = SANE_DESC_SCAN_TL_X;

  s->opt[OPT_TL_X].type = SANE_TYPE_FIXED;
  s->opt[OPT_TL_X].unit = SANE_UNIT_MM;
  s->opt[OPT_TL_X].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_TL_X].constraint.range = &(s->hw->src->x_range);
  s->val[OPT_TL_X].w = 0;

  /* top-left y */
  s->opt[OPT_TL_Y].name = SANE_NAME_SCAN_TL_Y;
  s->opt[OPT_TL_Y].title = SANE_TITLE_SCAN_TL_Y;
  s->opt[OPT_TL_Y].desc = SANE_DESC_SCAN_TL_Y;

  s->opt[OPT_TL_Y].type = SANE_TYPE_FIXED;
  s->opt[OPT_TL_Y].unit = SANE_UNIT_MM;
  s->opt[OPT_TL_Y].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_TL_Y].constraint.range = &(s->hw->src->y_range);
  s->val[OPT_TL_Y].w = 0;

  /* bottom-right x */
  s->opt[OPT_BR_X].name = SANE_NAME_SCAN_BR_X;
  s->opt[OPT_BR_X].title = SANE_TITLE_SCAN_BR_X;
  s->opt[OPT_BR_X].desc = SANE_DESC_SCAN_BR_X;

  s->opt[OPT_BR_X].type = SANE_TYPE_FIXED;
  s->opt[OPT_BR_X].unit = SANE_UNIT_MM;
  s->opt[OPT_BR_X].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_BR_X].constraint.range = &(s->hw->src->x_range);
  s->val[OPT_BR_X].w = s->hw->src->x_range.max;

  /* bottom-right y */
  s->opt[OPT_BR_Y].name = SANE_NAME_SCAN_BR_Y;
  s->opt[OPT_BR_Y].title = SANE_TITLE_SCAN_BR_Y;
  s->opt[OPT_BR_Y].desc = SANE_DESC_SCAN_BR_Y;

  s->opt[OPT_BR_Y].type = SANE_TYPE_FIXED;
  s->opt[OPT_BR_Y].unit = SANE_UNIT_MM;
  s->opt[OPT_BR_Y].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_BR_Y].constraint.range = &(s->hw->src->y_range);
  s->val[OPT_BR_Y].w = s->hw->src->y_range.max;

  /* "Optional equipment" group: */
  s->opt[OPT_EQU_GROUP].title = SANE_I18N ("Optional equipment");
  s->opt[OPT_EQU_GROUP].desc = "";
  s->opt[OPT_EQU_GROUP].type = SANE_TYPE_GROUP;
  s->opt[OPT_EQU_GROUP].cap = SANE_CAP_ADVANCED;

  /* source */
  s->opt[OPT_SOURCE].name = SANE_NAME_SCAN_SOURCE;
  s->opt[OPT_SOURCE].title = SANE_TITLE_SCAN_SOURCE;
  s->opt[OPT_SOURCE].desc = SANE_DESC_SCAN_SOURCE;

  s->opt[OPT_SOURCE].type = SANE_TYPE_STRING;
  s->opt[OPT_SOURCE].size = max_string_size (s->hw->sources);

  s->opt[OPT_SOURCE].constraint_type = SANE_CONSTRAINT_STRING_LIST;
  s->opt[OPT_SOURCE].constraint.string_list = s->hw->sources;

  if (!s->hw->sources[1])        /* two or more scan sources */
  {
    s->opt[OPT_SOURCE].cap |= SANE_CAP_INACTIVE;
  }
  s->val[OPT_SOURCE].w = 0;


  /* film type */
  s->opt[OPT_FILM_TYPE].name = "film-type";
  s->opt[OPT_FILM_TYPE].title = SANE_I18N ("Film type");
  s->opt[OPT_FILM_TYPE].desc = "";

  s->opt[OPT_FILM_TYPE].type = SANE_TYPE_STRING;
  s->opt[OPT_FILM_TYPE].size = max_string_size (film_list);

  s->opt[OPT_FILM_TYPE].constraint_type = SANE_CONSTRAINT_STRING_LIST;
  s->opt[OPT_FILM_TYPE].constraint.string_list = film_list;

  s->val[OPT_FILM_TYPE].w = 0;

  deactivateOption (s, OPT_FILM_TYPE, &dummy);	/* default is inactive */

  /* focus position */
  s->opt[OPT_FOCUS].name = SANE_EPSON_FOCUS_NAME;
  s->opt[OPT_FOCUS].title = SANE_EPSON_FOCUS_TITLE;
  s->opt[OPT_FOCUS].desc = SANE_EPSON_FOCUS_DESC;
  s->opt[OPT_FOCUS].type = SANE_TYPE_STRING;
  s->opt[OPT_FOCUS].size = max_string_size (focus_list);
  s->opt[OPT_FOCUS].constraint_type = SANE_CONSTRAINT_STRING_LIST;
  s->opt[OPT_FOCUS].constraint.string_list = focus_list;
  s->val[OPT_FOCUS].w = 0;

  s->opt[OPT_FOCUS].cap |= SANE_CAP_ADVANCED;
  if (s->hw->tpu && s->hw->tpu->has_focus)
  {
    s->opt[OPT_FOCUS].cap &= ~SANE_CAP_INACTIVE;
  }
  else
  {
    s->opt[OPT_FOCUS].cap |= SANE_CAP_INACTIVE;
  }

  /* forward feed / eject */
  s->opt[OPT_EJECT].name = "eject";
  s->opt[OPT_EJECT].title = SANE_I18N ("Eject");
  s->opt[OPT_EJECT].desc = SANE_I18N ("Eject the sheet in the ADF");

  s->opt[OPT_EJECT].type = SANE_TYPE_BUTTON;

  if ((!s->hw->adf) && (!s->hw->cmd->set_bay))
  {				/* Hack: Using set_bay to indicate. */
    s->opt[OPT_EJECT].cap |= SANE_CAP_INACTIVE;
  }

  /* auto forward feed / eject */
  s->opt[OPT_AUTO_EJECT].name = "auto-eject";
  s->opt[OPT_AUTO_EJECT].title = SANE_I18N ("Auto eject");
  s->opt[OPT_AUTO_EJECT].desc = SANE_I18N ("Eject document after scanning");

  s->opt[OPT_AUTO_EJECT].type = SANE_TYPE_BOOL;
  s->val[OPT_AUTO_EJECT].w = SANE_TRUE;

  if (s->hw->adf) s->hw->adf->auto_eject = SANE_TRUE;

  if (!s->hw->adf)
  {
    s->opt[OPT_AUTO_EJECT].cap |= SANE_CAP_INACTIVE;
  }

  s->opt[OPT_ADF_MODE].name = "adf-mode";
  s->opt[OPT_ADF_MODE].title = SANE_I18N ("ADF Mode");
  s->opt[OPT_ADF_MODE].desc =
    SANE_I18N ("Selects the ADF mode (simplex/duplex)");
  s->opt[OPT_ADF_MODE].type = SANE_TYPE_STRING;
  s->opt[OPT_ADF_MODE].size = max_string_size (adf_mode_list);
  s->opt[OPT_ADF_MODE].constraint_type = SANE_CONSTRAINT_STRING_LIST;
  s->opt[OPT_ADF_MODE].constraint.string_list = adf_mode_list;
  s->val[OPT_ADF_MODE].w = 0;	/* simplex */

  if (!(using (s->hw, adf) && (EXT_STATUS_ADFS & s->hw->ext_status)))
  {
    s->opt[OPT_ADF_MODE].cap |= SANE_CAP_INACTIVE;
  }

  /* select bay */
  s->opt[OPT_BAY].name = "bay";
  s->opt[OPT_BAY].title = SANE_I18N ("Bay");
  s->opt[OPT_BAY].desc = SANE_I18N ("Select bay to scan");

  s->opt[OPT_BAY].type = SANE_TYPE_STRING;
  s->opt[OPT_BAY].size = max_string_size (bay_list);
  s->opt[OPT_BAY].constraint_type = SANE_CONSTRAINT_STRING_LIST;
  s->opt[OPT_BAY].constraint.string_list = bay_list;
  s->val[OPT_BAY].w = 0;	/* Bay 1 */

  if (!s->hw->cmd->set_bay)
  {
    s->opt[OPT_BAY].cap |= SANE_CAP_INACTIVE;
  }

  s->opt[OPT_WAIT_FOR_BUTTON].name = SANE_EPSON_WAIT_FOR_BUTTON_NAME;
  s->opt[OPT_WAIT_FOR_BUTTON].title = SANE_EPSON_WAIT_FOR_BUTTON_TITLE;
  s->opt[OPT_WAIT_FOR_BUTTON].desc = SANE_EPSON_WAIT_FOR_BUTTON_DESC;

  s->opt[OPT_WAIT_FOR_BUTTON].type = SANE_TYPE_BOOL;
  s->opt[OPT_WAIT_FOR_BUTTON].unit = SANE_UNIT_NONE;
  s->opt[OPT_WAIT_FOR_BUTTON].constraint_type = SANE_CONSTRAINT_NONE;
  s->opt[OPT_WAIT_FOR_BUTTON].constraint.range = NULL;
  s->opt[OPT_WAIT_FOR_BUTTON].cap |= SANE_CAP_ADVANCED;
  s->val[OPT_WAIT_FOR_BUTTON].w = SANE_FALSE;

  if (!s->hw->cmd->request_push_button_status
      || push_button_is_black_listed (s->hw))
  {
    s->opt[OPT_WAIT_FOR_BUTTON].cap |= SANE_CAP_INACTIVE;
  }
  else if (push_button_needs_polling (s->hw))
  {
    s->val[OPT_WAIT_FOR_BUTTON].w = SANE_TRUE;
  }

  s->opt[OPT_MONITOR_BUTTON].name = SANE_EPSON_MONITOR_BUTTON_NAME;
  s->opt[OPT_MONITOR_BUTTON].title = SANE_EPSON_MONITOR_BUTTON_TITLE;
  s->opt[OPT_MONITOR_BUTTON].desc = SANE_EPSON_MONITOR_BUTTON_DESC;

  s->opt[OPT_MONITOR_BUTTON].type = SANE_TYPE_BOOL;
  s->opt[OPT_MONITOR_BUTTON].unit = SANE_UNIT_NONE;
  s->opt[OPT_MONITOR_BUTTON].constraint_type = SANE_CONSTRAINT_NONE;
  s->opt[OPT_MONITOR_BUTTON].constraint.range = NULL;
  s->opt[OPT_MONITOR_BUTTON].cap |=  SANE_CAP_ADVANCED;
  s->opt[OPT_MONITOR_BUTTON].cap &= ~SANE_CAP_SOFT_SELECT;
  s->val[OPT_MONITOR_BUTTON].w = SANE_FALSE;

  if (!s->hw->cmd->request_push_button_status
      || push_button_is_black_listed (s->hw))
  {
    s->opt[OPT_MONITOR_BUTTON].cap |= SANE_CAP_INACTIVE;
  }

  s->opt[OPT_POLLING_TIME].name = SANE_EPSON_POLLING_TIME_NAME;
  s->opt[OPT_POLLING_TIME].title = SANE_EPSON_POLLING_TIME_TITLE;
  s->opt[OPT_POLLING_TIME].desc = SANE_EPSON_POLLING_TIME_DESC;

  s->opt[OPT_POLLING_TIME].type = SANE_TYPE_INT;
  s->opt[OPT_POLLING_TIME].unit = SANE_UNIT_MICROSECOND;
  s->opt[OPT_POLLING_TIME].constraint_type = SANE_CONSTRAINT_NONE;
  s->opt[OPT_POLLING_TIME].cap |=  SANE_CAP_ADVANCED;
  s->opt[OPT_POLLING_TIME].cap &= ~SANE_CAP_SOFT_SELECT;
  s->val[OPT_POLLING_TIME].w = s->hw->polling_time;

  if (!s->hw->cmd->request_push_button_status
      || push_button_is_black_listed (s->hw))
  {
    s->opt[OPT_POLLING_TIME].cap |= SANE_CAP_INACTIVE;
  }

  s->opt[OPT_NEEDS_POLLING].name = SANE_EPSON_NEEDS_POLLING_NAME;
  s->opt[OPT_NEEDS_POLLING].title = SANE_EPSON_NEEDS_POLLING_TITLE;
  s->opt[OPT_NEEDS_POLLING].desc = SANE_EPSON_NEEDS_POLLING_DESC;

  s->opt[OPT_NEEDS_POLLING].type = SANE_TYPE_BOOL;
  s->opt[OPT_NEEDS_POLLING].unit = SANE_UNIT_NONE;
  s->opt[OPT_NEEDS_POLLING].constraint_type = SANE_CONSTRAINT_NONE;
  s->opt[OPT_NEEDS_POLLING].cap |=  SANE_CAP_ADVANCED;
  s->opt[OPT_NEEDS_POLLING].cap &= ~SANE_CAP_SOFT_SELECT;
  s->val[OPT_NEEDS_POLLING].w = SANE_FALSE;

  if (!s->hw->cmd->request_push_button_status
      || push_button_is_black_listed (s->hw))
  {
    s->opt[OPT_NEEDS_POLLING].cap |= SANE_CAP_INACTIVE;
  }
  else if (push_button_needs_polling (s->hw))
  {
    s->val[OPT_NEEDS_POLLING].w = SANE_TRUE;
  }

  s->opt[OPT_DETECT_DOC_SIZE].name = SANE_EPSON_DETECT_DOC_SIZE_NAME;
  s->opt[OPT_DETECT_DOC_SIZE].title = SANE_EPSON_DETECT_DOC_SIZE_TITLE;
  s->opt[OPT_DETECT_DOC_SIZE].desc = SANE_EPSON_DETECT_DOC_SIZE_DESC;

  s->opt[OPT_DETECT_DOC_SIZE].type = SANE_TYPE_BOOL;
  s->opt[OPT_DETECT_DOC_SIZE].unit = SANE_UNIT_NONE;
  s->opt[OPT_DETECT_DOC_SIZE].constraint_type = SANE_CONSTRAINT_NONE;
  s->opt[OPT_DETECT_DOC_SIZE].constraint.range = NULL;
  s->opt[OPT_DETECT_DOC_SIZE].cap |= SANE_CAP_ADVANCED;
  s->val[OPT_DETECT_DOC_SIZE].w = SANE_FALSE;

  if (!has_size_check_support (s->hw->src))
  {
    s->opt[OPT_DETECT_DOC_SIZE].cap |= SANE_CAP_INACTIVE;
  }

  s->opt[OPT_SCAN_AREA_IS_VALID].name = SANE_EPSON_SCAN_AREA_IS_VALID_NAME;
  s->opt[OPT_SCAN_AREA_IS_VALID].title = SANE_EPSON_SCAN_AREA_IS_VALID_TITLE;
  s->opt[OPT_SCAN_AREA_IS_VALID].desc = SANE_EPSON_SCAN_AREA_IS_VALID_DESC;

  s->opt[OPT_SCAN_AREA_IS_VALID].type = SANE_TYPE_BOOL;
  s->opt[OPT_SCAN_AREA_IS_VALID].unit = SANE_UNIT_NONE;
  s->opt[OPT_SCAN_AREA_IS_VALID].constraint_type = SANE_CONSTRAINT_NONE;
  s->opt[OPT_SCAN_AREA_IS_VALID].constraint.range = NULL;
  s->opt[OPT_SCAN_AREA_IS_VALID].cap = SANE_CAP_SOFT_DETECT | SANE_CAP_ADVANCED;
  s->val[OPT_SCAN_AREA_IS_VALID].w = SANE_FALSE;

  s->opt[OPT_ADF_DUPLEX_DIRECTION_MATCHES].name =
          SANE_EPSON_ADF_DUPLEX_DIRECTION_MATCHES_NAME;
  s->opt[OPT_ADF_DUPLEX_DIRECTION_MATCHES].title =
          SANE_EPSON_ADF_DUPLEX_DIRECTION_MATCHES_TITLE;
  s->opt[OPT_ADF_DUPLEX_DIRECTION_MATCHES].desc =
          SANE_EPSON_ADF_DUPLEX_DIRECTION_MATCHES_DESC;

  s->opt[OPT_ADF_DUPLEX_DIRECTION_MATCHES].type = SANE_TYPE_BOOL;
  s->opt[OPT_ADF_DUPLEX_DIRECTION_MATCHES].unit = SANE_UNIT_NONE;
  s->opt[OPT_ADF_DUPLEX_DIRECTION_MATCHES].constraint_type =
          SANE_CONSTRAINT_NONE;
  s->opt[OPT_ADF_DUPLEX_DIRECTION_MATCHES].constraint.range = NULL;
  s->opt[OPT_ADF_DUPLEX_DIRECTION_MATCHES].cap =
          SANE_CAP_SOFT_DETECT | SANE_CAP_ADVANCED;
  s->val[OPT_ADF_DUPLEX_DIRECTION_MATCHES].w = SANE_FALSE;

  if (!(using (s->hw, adf) && (EXT_STATUS_ADFS & s->hw->ext_status)))
  {
    s->opt[OPT_ADF_DUPLEX_DIRECTION_MATCHES].cap |= SANE_CAP_INACTIVE;
  }

  s->opt[OPT_ADF_AUTO_SCAN].name  = SANE_EPSON_ADF_AUTO_SCAN_NAME;
  s->opt[OPT_ADF_AUTO_SCAN].title = SANE_EPSON_ADF_AUTO_SCAN_TITLE;
  s->opt[OPT_ADF_AUTO_SCAN].desc  = SANE_EPSON_ADF_AUTO_SCAN_DESC;

  s->opt[OPT_ADF_AUTO_SCAN].type = SANE_TYPE_BOOL;
  s->opt[OPT_ADF_AUTO_SCAN].unit = SANE_UNIT_NONE;
  s->opt[OPT_ADF_AUTO_SCAN].constraint_type = SANE_CONSTRAINT_NONE;
  s->opt[OPT_ADF_AUTO_SCAN].constraint.range = NULL;
  s->opt[OPT_ADF_AUTO_SCAN].cap |= SANE_CAP_ADVANCED;
  s->val[OPT_ADF_AUTO_SCAN].w = SANE_FALSE;

  if (!using (s->hw, adf) || !(FSI_CAP_ADFAS & s->hw->fsi_cap_2))
    {
      s->opt[OPT_ADF_AUTO_SCAN].cap |= SANE_CAP_INACTIVE;
    }

  s->opt[OPT_ADF_DFD_SENSITIVITY].name  = SANE_EPSON_ADF_DFD_SENSITIVITY_NAME;
  s->opt[OPT_ADF_DFD_SENSITIVITY].title = SANE_EPSON_ADF_DFD_SENSITIVITY_TITLE;
  s->opt[OPT_ADF_DFD_SENSITIVITY].desc  = SANE_EPSON_ADF_DFD_SENSITIVITY_DESC;

  s->opt[OPT_ADF_DFD_SENSITIVITY].type = SANE_TYPE_STRING;
  s->opt[OPT_ADF_DFD_SENSITIVITY].unit = SANE_UNIT_NONE;
  s->opt[OPT_ADF_DFD_SENSITIVITY].size = max_string_size (dfd_sensitivity);
  s->opt[OPT_ADF_DFD_SENSITIVITY].constraint_type = SANE_CONSTRAINT_STRING_LIST;
  s->opt[OPT_ADF_DFD_SENSITIVITY].constraint.string_list = dfd_sensitivity;
  s->opt[OPT_ADF_DFD_SENSITIVITY].cap |= SANE_CAP_ADVANCED;
  s->val[OPT_ADF_DFD_SENSITIVITY].w = 0;

  if (!using (s->hw, adf) || !(FSI_CAP_DFD & s->hw->fsi_cap_2))
    {
      s->opt[OPT_ADF_DFD_SENSITIVITY].cap |= SANE_CAP_INACTIVE;
    }

  s->opt[OPT_EXT_SANE_STATUS].name  = "ext-sane-status";
  s->opt[OPT_EXT_SANE_STATUS].title = "Extended SANE Status";
  s->opt[OPT_EXT_SANE_STATUS].desc  = "Ugly kludge to provide additional status message strings to a frontend.";

  s->opt[OPT_EXT_SANE_STATUS].type = SANE_TYPE_INT;
  s->opt[OPT_EXT_SANE_STATUS].unit = SANE_UNIT_NONE;
  s->opt[OPT_EXT_SANE_STATUS].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_EXT_SANE_STATUS].constraint.range = &ext_sane_status;
  s->opt[OPT_EXT_SANE_STATUS].cap = SANE_CAP_SOFT_DETECT | SANE_CAP_ADVANCED;
  s->val[OPT_EXT_SANE_STATUS].w = 0;

  /* deskew */
  s->opt[OPT_DESKEW].name  = "deskew";
  s->opt[OPT_DESKEW].title = SANE_I18N ("Deskew");
  s->opt[OPT_DESKEW].desc  = SANE_I18N ("Rotate image so it appears upright.");
  s->opt[OPT_DESKEW].type  = SANE_TYPE_BOOL;
  s->opt[OPT_DESKEW].unit  = SANE_UNIT_NONE;
  s->opt[OPT_DESKEW].size  = sizeof (SANE_Bool);
  s->opt[OPT_DESKEW].cap  |= SANE_CAP_EMULATED;
  s->opt[OPT_DESKEW].cap  |= SANE_CAP_ADVANCED;
  if ( dip_has_deskew (s->dip, s->hw) )
    {
      s->opt[OPT_DESKEW].cap &= ~SANE_CAP_INACTIVE;
    }
  else
    {
      s->opt[OPT_DESKEW].cap  |= SANE_CAP_INACTIVE;
    }
  s->opt[OPT_DESKEW].constraint_type  = SANE_CONSTRAINT_NONE;
  s->val[OPT_DESKEW].w = SANE_FALSE;

  /* auto-crop */
  s->opt[OPT_AUTOCROP].name  = "autocrop";
  s->opt[OPT_AUTOCROP].title = SANE_I18N ("Trim image to paper size");
  s->opt[OPT_AUTOCROP].desc  = SANE_I18N ("Determines empty margins in the scanned image and removes them.  This normally reduces the image to the size of the original document but may remove more.");
  s->opt[OPT_AUTOCROP].type  = SANE_TYPE_BOOL;
  s->opt[OPT_AUTOCROP].unit  = SANE_UNIT_NONE;
  s->opt[OPT_AUTOCROP].size  = sizeof (SANE_Bool);
  s->opt[OPT_AUTOCROP].cap  |= SANE_CAP_EMULATED;
  s->opt[OPT_AUTOCROP].cap  |= SANE_CAP_ADVANCED;
  if ( dip_has_autocrop (s->dip, s->hw) )
    {
      s->opt[OPT_AUTOCROP].cap &= ~SANE_CAP_INACTIVE;
    }
  else
    {
      s->opt[OPT_AUTOCROP].cap  |= SANE_CAP_INACTIVE;
    }
  s->opt[OPT_AUTOCROP].constraint_type  = SANE_CONSTRAINT_NONE;
  s->val[OPT_AUTOCROP].w = SANE_FALSE;

  s->opt[OPT_CALIBRATE].name  = SANE_EPSON_CALIBRATE_NAME;
  s->opt[OPT_CALIBRATE].title = SANE_EPSON_CALIBRATE_TITLE;
  s->opt[OPT_CALIBRATE].desc  = SANE_EPSON_CALIBRATE_DESC;
  s->opt[OPT_CALIBRATE].type  = SANE_TYPE_BUTTON;
  s->opt[OPT_CALIBRATE].unit  = SANE_UNIT_NONE;
  s->opt[OPT_CALIBRATE].cap  |= SANE_CAP_ADVANCED;
  s->opt[OPT_CALIBRATE].constraint_type  = SANE_CONSTRAINT_NONE;
  if (!maintenance_is_supported (s->hw))
  {
    s->opt[OPT_CALIBRATE].cap  |= SANE_CAP_INACTIVE;
  }

  s->opt[OPT_CLEAN].name  = SANE_EPSON_CLEAN_NAME;
  s->opt[OPT_CLEAN].title = SANE_EPSON_CLEAN_TITLE;
  s->opt[OPT_CLEAN].desc  = SANE_EPSON_CLEAN_DESC;
  s->opt[OPT_CLEAN].type  = SANE_TYPE_BUTTON;
  s->opt[OPT_CLEAN].unit  = SANE_UNIT_NONE;
  s->opt[OPT_CLEAN].cap  |= SANE_CAP_ADVANCED;
  s->opt[OPT_CLEAN].constraint_type  = SANE_CONSTRAINT_NONE;
  if (!maintenance_is_supported (s->hw))
  {
    s->opt[OPT_CLEAN].cap  |= SANE_CAP_INACTIVE;
  }

  handle_mode (s, s->val[OPT_MODE].w, &reload);
  change_profile_matrix (s);

  /* adjust default settings based on configuration options */
  {
    void *cfg = cfg_init (NULL, NULL);
    if (cfg_has_value (cfg, CFG_KEY_OPTION, "prefer-adf"))
      {
        const char *found = NULL;
        int i = 0;
        while ((found = s->hw->sources[i])
               && 0 != strcmp_c (ADF_STR, found))
          {
            ++i;
          }
        if (found) handle_source (s, i, ADF_STR);
      }
  }

  return SANE_STATUS_GOOD;
}


SANE_Status
epkowa_open (const char *name, SANE_Handle *handle, const void *dip)
{
  SANE_Status status = SANE_STATUS_GOOD;
  Epson_Scanner *s = NULL;

  status = create_sane_handle (&s, name, dip);
  if (SANE_STATUS_GOOD != status) return status;

  /* insert newly opened handle into list of open handles */
  s->next = first_handle;
  first_handle = s;

  *handle = (SANE_Handle) s;

  return SANE_STATUS_GOOD;
}


void
sane_close (SANE_Handle handle)
{
  Epson_Scanner *s, *prev;
  size_t i;

  /* Test if there is still data pending from the scanner. If so, then
   * do a cancel.
   */

  log_call ();

  s = (Epson_Scanner *) handle;

  /* remove handle from list of open handles */
  prev = 0;
  for (s = first_handle; s; s = s->next)
  {
    if (s == handle)
      break;
    prev = s;
  }

  if (!s)
  {
    err_fatal ("invalid handle (0x%p)", handle);
    return;
  }

  if (prev)
    prev->next = s->next;
  else
    first_handle = s->next;

  s->hw = dev_dtor (s->hw);

  const_delete (s->opt[OPT_BIT_DEPTH].constraint.word_list, SANE_Word *);
  const_delete (s->opt[OPT_SCAN_AREA].constraint.string_list,
                SANE_String_Const *);

  /* image data acquisition related resources */
  delete (s->raw.buf);
  delete (s->img.buf);
  for (i = 0; i < LINES_SHUFFLE_MAX; ++i)
    delete (s->line_buffer[i]);

  dip_destroy_LUT (s->dip, s->lut);

  delete (s);
}


const SANE_Option_Descriptor *
sane_get_option_descriptor (SANE_Handle handle, SANE_Int option)
{
  Epson_Scanner *s = (Epson_Scanner *) handle;

  if (option < 0 || option >= NUM_OPTIONS)
    {
      log_call ("(%d)", option);
      return NULL;
    }

  log_call ("(%s)", s->opt[option].name);
  return (s->opt + option);
}


static const SANE_String_Const *
search_string_list (const SANE_String_Const * list, SANE_String value)
{
  log_call ("(%s)", value);

  while (*list != NULL && strcmp_c (value, *list) != 0)
  {
    ++list;
  }

  return ((*list == NULL) ? NULL : list);
}


/*  Activate, deactivate an option.  Subroutines so we can add
    debugging info if we want.  The change flag is set to TRUE
    if we changed an option.  If we did not change an option,
    then the value of the changed flag is not modified.
 */
static void
activateOption (Epson_Scanner * s, SANE_Int option, SANE_Bool * change)
{
  log_call ("(%s)", s->opt[option].name);
  if (!SANE_OPTION_IS_ACTIVE (s->opt[option].cap))
  {
    s->opt[option].cap &= ~SANE_CAP_INACTIVE;
    *change = SANE_TRUE;
  }
}

static void
deactivateOption (Epson_Scanner * s, SANE_Int option, SANE_Bool * change)
{
  log_call ("(%s)", s->opt[option].name);
  if (SANE_OPTION_IS_ACTIVE (s->opt[option].cap))
  {
    s->opt[option].cap |= SANE_CAP_INACTIVE;
    *change = SANE_TRUE;
  }
}

static void
setOptionState (Epson_Scanner * s, SANE_Bool state,
		SANE_Int option, SANE_Bool * change)
{
  if (state)
  {
    activateOption (s, option, change);
  }
  else
  {
    deactivateOption (s, option, change);
  }
}

static void
calculate_scan_area_offset (const Option_Value *v, int *left, int *top)
{
  *left =
    SANE_UNFIX (v[OPT_TL_X].w) / MM_PER_INCH *
    v[OPT_X_RESOLUTION].w * v[OPT_ZOOM].w / 100 + 0.5;

  *top =
    SANE_UNFIX (v[OPT_TL_Y].w) / MM_PER_INCH *
    v[OPT_Y_RESOLUTION].w * v[OPT_ZOOM].w / 100 + 0.5;
}

static void
calculate_scan_area_max (const Epson_Scanner *s, int *x, int *y)
{
  /* Cast first value to double to force the whole computation to be
     done in double.  Works around integer overflows.
   */
  *x = ((double) s->hw->src->max_x * s->val[OPT_X_RESOLUTION].w *
        s->val[OPT_ZOOM].w / (s->hw->base_res * 100));
  *y = ((double) s->hw->src->max_y * s->val[OPT_Y_RESOLUTION].w *
        s->val[OPT_ZOOM].w / (s->hw->base_res * 100));
}

static bool
scan_area_is_valid (Epson_Scanner *s)
{
  int left = 0;
  int top = 0;
  int max_x = 0;
  int max_y = 0;

  bool rv = true;

  /* finalize parameters before we validate*/
  estimate_parameters (s, NULL);

  calculate_scan_area_max (s, &max_x, &max_y);
  calculate_scan_area_offset (s->val, &left, &top);

  if ( s->raw.ctx.pixels_per_line > max_x)         rv = false;
  if ((left + s->raw.ctx.pixels_per_line) > max_x) rv = false;

  if (!need_autocrop_override (s))
    {
      if ( s->raw.ctx.lines > max_y )       rv = false;
      if ((top + s->raw.ctx.lines) > max_y) rv = false;
    }

  /* check physical channel limitations */
  {
    size_t max_req = s->hw->channel->max_request_size (s->hw->channel);
    if (s->raw.ctx.bytes_per_line > max_req) rv = false;
  }

  if (s->hw->using_fs)
    {
      if (s->raw.ctx.pixels_per_line > s->hw->scan_width_limit) rv = false;
      return rv;
    }

  if (SANE_FRAME_RGB == s->raw.ctx.format)    /* max x according to to spec */
    if (s->raw.ctx.pixels_per_line > 21840) rv = false;
  if (top > 65530) rv = false;
  if (left > 65530) rv = false;

  return rv;
}

static SANE_Status
getvalue (Epson_Scanner *s, SANE_Int option, void *value)
{
  SANE_Option_Descriptor *sopt = &(s->opt[option]);
  Option_Value *sval = &(s->val[option]);

  log_call ("(%s)", sopt->name);

  switch (option)
  {
  case OPT_GAMMA_VECTOR_R:
  case OPT_GAMMA_VECTOR_G:
  case OPT_GAMMA_VECTOR_B:
    memcpy (value, sval->wa, sopt->size);
    break;

  case OPT_NUM_OPTS:
  case OPT_RESOLUTION:
  case OPT_X_RESOLUTION:
  case OPT_Y_RESOLUTION:
  case OPT_TL_X:
  case OPT_TL_Y:
  case OPT_BR_X:
  case OPT_BR_Y:
  case OPT_MIRROR:
  case OPT_SPEED:
  case OPT_PREVIEW_SPEED:
  case OPT_AAS:
  case OPT_PREVIEW:
  case OPT_BRIGHTNESS:
  case OPT_CONTRAST:
  case OPT_SHARPNESS:
  case OPT_AUTO_EJECT:
  case OPT_CCT_1:
  case OPT_CCT_2:
  case OPT_CCT_3:
  case OPT_CCT_4:
  case OPT_CCT_5:
  case OPT_CCT_6:
  case OPT_CCT_7:
  case OPT_CCT_8:
  case OPT_CCT_9:
  case OPT_THRESHOLD:
  case OPT_ZOOM:
  case OPT_BIT_DEPTH:
  case OPT_WAIT_FOR_BUTTON:
  case OPT_DETECT_DOC_SIZE:
  case OPT_LIMIT_RESOLUTION:
  case OPT_ADF_AUTO_SCAN:
  case OPT_DESKEW:
  case OPT_AUTOCROP:
  case OPT_NEEDS_POLLING:
    *((SANE_Word *) value) = sval->w;
    break;
  case OPT_MODE:
  case OPT_ADF_MODE:
  case OPT_HALFTONE:
  case OPT_DROPOUT:
  case OPT_BRIGHTNESS_METHOD:
  case OPT_SCAN_AREA:
  case OPT_SOURCE:
  case OPT_FILM_TYPE:
  case OPT_GAMMA_CORRECTION:
  case OPT_COLOR_CORRECTION:
  case OPT_BAY:
  case OPT_FOCUS:
  case OPT_ADF_DFD_SENSITIVITY:
    strcpy ((char *) value, sopt->constraint.string_list[sval->w]);
    break;

  case OPT_QUICK_FORMAT:
    getvalue (s, OPT_SCAN_AREA, value);
    break;

  case OPT_EXT_SANE_STATUS:
    if (using (s->hw, adf)
        && (ADF_EXT_STATUS_DFE & s->hw->adf->ext_status))
      sval->w = EXT_SANE_STATUS_MULTI_FEED;
    if (using (s->hw, adf)
        && (ADF_EXT_STATUS_TR_OPN & s->hw->adf->ext_status))
      sval->w = EXT_SANE_STATUS_TRAY_CLOSED;
    *((SANE_Word *) value) = sval->w;
    sval->w = 0;
    break;

  case OPT_MONITOR_BUTTON:
    if (SANE_OPTION_IS_ACTIVE (option))
    {
      SANE_Bool pressed;
      SANE_Status status = SANE_STATUS_GOOD;
      if (SANE_STATUS_GOOD == status)
      {
	status = get_push_button_status (s->hw, &pressed);
	if (SANE_STATUS_GOOD == status)
	{
	  *((SANE_Bool *) value) = pressed;
	}
      }
      return status;
    }
    else
    {
      return SANE_STATUS_UNSUPPORTED;
    }
    break;
  case OPT_POLLING_TIME:
    *((SANE_Word *) value) = sval->w;
    break;
  case OPT_SCAN_AREA_IS_VALID:
    {
      sval->w = scan_area_is_valid (s);
      *((SANE_Word *) value) = sval->w;
    }
    break;
  case OPT_ADF_DUPLEX_DIRECTION_MATCHES:
    {
      sval->w = adf_duplex_direction_matches (s->hw);
      *((SANE_Word *) value) = sval->w;
    }
    break;
  default:
    return SANE_STATUS_INVAL;
  }

  return SANE_STATUS_GOOD;
}


static void
handle_mode (Epson_Scanner * s, SANE_Int optindex, SANE_Bool * reload)
{
  SANE_Bool dropout, aas, halftone, threshold, cct;
  SANE_Bool brightness, contrast;

  log_call ();

  *reload = SANE_FALSE;

  switch (optindex)
  {
  case 0:			/* b & w */
    dropout = SANE_TRUE;
    aas = SANE_TRUE;
    halftone = SANE_TRUE;
    threshold = SANE_TRUE;
    cct = SANE_FALSE;
    brightness = SANE_FALSE;
    contrast = SANE_FALSE;
    break;
  case 1:			/* gray */
    dropout = SANE_TRUE;
    aas = SANE_FALSE;
    halftone = SANE_FALSE;
    threshold = SANE_FALSE;
    cct = SANE_FALSE;
    brightness = SANE_TRUE;
    contrast = SANE_TRUE;
    break;
  case 2:			/* color */
    dropout = SANE_FALSE;
    aas = SANE_FALSE;
    halftone = SANE_FALSE;
    threshold = SANE_FALSE;
    cct = SANE_TRUE;
    brightness = SANE_TRUE;
    contrast = SANE_TRUE;
    break;
  default:
    return;
  }

  if (s->hw->cmd->level[0] == 'D')
  {
    dropout = SANE_FALSE;
    aas = SANE_FALSE;
    halftone = SANE_FALSE;
  }

  setOptionState (s, dropout, OPT_DROPOUT, reload);
  s->val[OPT_DROPOUT].w = 0;
  setOptionState (s, halftone, OPT_HALFTONE, reload);
  s->val[OPT_HALFTONE].w = 0;
  setOptionState (s, aas, OPT_AAS, reload);
  s->val[OPT_AAS].w = SANE_FALSE;

  setOptionState (s, threshold, OPT_THRESHOLD, reload);

  setOptionState (s, brightness, OPT_BRIGHTNESS, reload);
  setOptionState (s, contrast, OPT_CONTRAST, reload);

  setOptionState (s, cct, OPT_CCT_1, reload);
  setOptionState (s, cct, OPT_CCT_2, reload);
  setOptionState (s, cct, OPT_CCT_3, reload);
  setOptionState (s, cct, OPT_CCT_4, reload);
  setOptionState (s, cct, OPT_CCT_5, reload);
  setOptionState (s, cct, OPT_CCT_6, reload);
  setOptionState (s, cct, OPT_CCT_7, reload);
  setOptionState (s, cct, OPT_CCT_8, reload);
  setOptionState (s, cct, OPT_CCT_9, reload);

  /* if binary, then disable the bit depth selection */
  if (optindex == 0)
  {
    s->opt[OPT_BIT_DEPTH].cap |= SANE_CAP_INACTIVE;
  }
  else
  {
    if (bitDepthList[0] == 1)
      s->opt[OPT_BIT_DEPTH].cap |= SANE_CAP_INACTIVE;
    else
    {
      s->opt[OPT_BIT_DEPTH].cap &= ~SANE_CAP_INACTIVE;
      s->val[OPT_BIT_DEPTH].w = mode_params[optindex].depth;
    }
  }

  if (optindex == 0)		/* b & w */
    handle_depth_halftone (s, 0, reload);

  *reload = SANE_TRUE;
}


static void
change_profile_matrix (Epson_Scanner * s)
{
  int index = 0;

  log_call ();

  require (s->hw->scan_hard);

  if (using (s->hw, tpu))       /* TPU */
  {
    if (s->val[OPT_FILM_TYPE].w == 0)	/* posi */
      index = 3;
    else
      index = 1;
  }
  else				/* Flatbed or ADF */
  {
    index = 0;
  }

  s->val[OPT_CCT_1].w = SANE_FIX (s->hw->scan_hard->color_profile[index][0]);
  s->val[OPT_CCT_2].w = SANE_FIX (s->hw->scan_hard->color_profile[index][1]);
  s->val[OPT_CCT_3].w = SANE_FIX (s->hw->scan_hard->color_profile[index][2]);
  s->val[OPT_CCT_4].w = SANE_FIX (s->hw->scan_hard->color_profile[index][3]);
  s->val[OPT_CCT_5].w = SANE_FIX (s->hw->scan_hard->color_profile[index][4]);
  s->val[OPT_CCT_6].w = SANE_FIX (s->hw->scan_hard->color_profile[index][5]);
  s->val[OPT_CCT_7].w = SANE_FIX (s->hw->scan_hard->color_profile[index][6]);
  s->val[OPT_CCT_8].w = SANE_FIX (s->hw->scan_hard->color_profile[index][7]);
  s->val[OPT_CCT_9].w = SANE_FIX (s->hw->scan_hard->color_profile[index][8]);
}


static unsigned char
int2cpt (int val)
{
  if (val >= 0)
  {
    if (val > 127)
      val = 127;
    return (unsigned char) val;
  }
  else
  {
    val = -val;
    if (val > 127)
      val = 127;
    return (unsigned char) (0x80 | val);
  }
}


static void
get_colorcoeff_from_profile (double *profile, unsigned char *color_coeff)
{
  int cc_idx[] = { 4, 1, 7, 3, 0, 6, 5, 2, 8 };
  int color_table[9];
  int i;

  round_cct (profile, color_table);

  for (i = 0; i < 9; i++)
    color_coeff[i] = int2cpt (color_table[cc_idx[i]]);
}


static void
round_cct (double org_cct[], int rnd_cct[])
{
  int i, j, index;
  double mult_cct[9], frac[9];
  int sum[3];
  int loop;

  for (i = 0; i < 9; i++)
    mult_cct[i] = org_cct[i] * 32;

  for (i = 0; i < 9; i++)
    rnd_cct[i] = (int) floor (org_cct[i] * 32 + 0.5);

  loop = 0;

  do
  {
    for (i = 0; i < 3; i++)
    {
      if ((rnd_cct[i * 3] == 11) &&
	  (rnd_cct[i * 3] == rnd_cct[i * 3 + 1]) &&
	  (rnd_cct[i * 3] == rnd_cct[i * 3 + 2]))
      {
	rnd_cct[i * 3 + i]--;
	mult_cct[i * 3 + i] = rnd_cct[i * 3 + i];
      }
    }

    for (i = 0; i < 3; i++)
    {
      sum[i] = 0;
      for (j = 0; j < 3; j++)
	sum[i] += rnd_cct[i * 3 + j];
    }

    for (i = 0; i < 9; i++)
      frac[i] = mult_cct[i] - rnd_cct[i];

    for (i = 0; i < 3; i++)
    {
      if (sum[i] < 32)
      {
	index = get_roundup_index (&frac[i * 3], 3);
	if (index != -1)
	{
	  rnd_cct[i * 3 + index]++;
	  mult_cct[i * 3 + index] = rnd_cct[i * 3 + index];
	  sum[i]++;
	}
      }
      else if (sum[i] > 32)
      {
	index = get_rounddown_index (&frac[i * 3], 3);
	if (index != -1)
	{
	  rnd_cct[i * 3 + index]--;
	  mult_cct[i * 3 + index] = rnd_cct[i * 3 + index];
	  sum[i]--;
	}
      }
    }
  }
  while ((++loop < 2)
	 && ((sum[0] != 32) || (sum[1] != 32) || (sum[2] != 32)));
}


static int
get_roundup_index (double frac[], int n)
{
  int i, index = -1;
  double max_val = 0.0;

  for (i = 0; i < n; i++)
  {
    if (frac[i] < 0)
      continue;
    if (max_val < frac[i])
    {
      index = i;
      max_val = frac[i];
    }
  }

  return index;
}


static int
get_rounddown_index (double frac[], int n)
{
  int i, index = -1;
  double min_val = 1.0;

  for (i = 0; i < n; i++)
  {
    if (frac[i] > 0)
      continue;
    if (min_val > frac[i])
    {
      index = i;
      min_val = frac[i];
    }
  }

  return index;
}


/*  This routine handles common options between OPT_MODE and
    OPT_HALFTONE.  These options are TET (a HALFTONE mode), AAS
    - auto area segmentation, and threshold.  Apparently AAS
    is some method to differentiate between text and photos.
    Or something like that.

    AAS is available when the scan color depth is 1 and the
    halftone method is not TET.

    Threshold is available when halftone is NONE, and depth is 1.
*/
static void
handle_depth_halftone (Epson_Scanner * s, SANE_Int optindex,
		       SANE_Bool * reload)
{
  SANE_Bool threshold, aas, dropout;

  log_call ();

  *reload = SANE_FALSE;

  switch (halftone_params[optindex])
  {
  case HALFTONE_NONE:
    threshold = SANE_TRUE;
    aas = SANE_TRUE;
    dropout = SANE_TRUE;
    break;
  case HALFTONE_TET:
    threshold = SANE_FALSE;
    aas = SANE_FALSE;
    dropout = SANE_FALSE;
    break;
  default:
    threshold = SANE_FALSE;
    aas = SANE_TRUE;
    dropout = SANE_TRUE;
  }

  setOptionState (s, threshold, OPT_THRESHOLD, reload);
  setOptionState (s, aas, OPT_AAS, reload);
  setOptionState (s, dropout, OPT_DROPOUT, reload);

  *reload = SANE_TRUE;
}


static void
handle_resolution (Epson_Scanner * s, SANE_Int option, SANE_Word value)
{
  SANE_Int *last = NULL;

  SANE_Int   size = 0;
  SANE_Word *list = NULL;

  int f = 0;
  int k = 0;
  int n = 0;

  log_call ("(%s, %d)", s->opt[option].name, value);

  switch (option)
    {
    case OPT_RESOLUTION:
      last = &s->hw->res.last;
      size =  s->hw->res.size;
      list =  s->hw->res.list;
      break;
    case OPT_X_RESOLUTION:
      last = &s->hw->res_x.last;
      size =  s->hw->res_x.size;
      list =  s->hw->res_x.list;
      break;
    case OPT_Y_RESOLUTION:
      last = &s->hw->res_y.last;
      size =  s->hw->res_y.size;
      list =  s->hw->res_y.list;
      break;
    default:
      err_fatal ("%s", strerror (EINVAL));
      exit (EXIT_FAILURE);
    }

  if (SANE_CONSTRAINT_RANGE == s->opt[option].constraint_type)
  {
    sanei_constrain_value (&(s->opt[option]), &value, NULL);
    s->val[option].w = value;
  }
  else
  {
    SANE_Int best = list[size];
    int min_d = INT_MAX;

    /* find supported resolution closest to that requested */
    for (n = 1; n <= size; n++)
    {
      int d = abs (value - list[n]);

      if (d < min_d)
      {
	min_d = d;
	k = n;
	best = list[n];
      }
    }

    /* FIXME? what's this trying to do?  Use a resolution close to the
              last one used if best is far away?  Why???  */
    if ((value != best) && *last)
    {
      for (f = 1; f <= size; f++)
	if (*last == list[f])
	  break;

      if (f != k && f != k - 1 && f != k + 1)
      {
	if (k > f)
	  best = list[f + 1];
	else if (k < f)
	  best = list[f - 1];
      }
    }

    *last = best;
    s->val[option].w = (SANE_Word) best;
  }

  if (OPT_RESOLUTION == option)
    {
      s->val[OPT_X_RESOLUTION].w = s->val[option].w;
      s->val[OPT_Y_RESOLUTION].w = s->val[option].w;

      s->hw->res_x.last = s->hw->res.last;
      s->hw->res_y.last = s->hw->res.last;
    }
  {
    SANE_Bool dummy;
    handle_deskew (s, NULL, &dummy);
  }
}

static void
limit_adf_res (Epson_Scanner * s)
{
  SANE_Constraint_Type type = s->opt[OPT_RESOLUTION].constraint_type;
  int limit = large_res_kills_adf_scan (s->hw);

  if (using (s->hw, adf))
    {
      dev_limit_res (s->hw, type, limit);

      /* constrain the current values to the new limit */
      handle_resolution (s, OPT_RESOLUTION, s->val[OPT_RESOLUTION].w);
      handle_resolution (s, OPT_X_RESOLUTION, s->val[OPT_X_RESOLUTION].w);
      handle_resolution (s, OPT_Y_RESOLUTION, s->val[OPT_Y_RESOLUTION].w);
    }
  else
    {
      dev_restore_res (s->hw, type);
    }
}


/*
    Handles setting the source (flatbed, transparency adapter (TPU),
    or auto document feeder (ADF)).

    For newer scanners it also sets the focus according to the 
    glass / TPU settings.
*/
static SANE_Status
handle_source (Epson_Scanner * s, SANE_Int optindex, char *value)
{
  SANE_Bool dummy;
  SANE_Status status = SANE_STATUS_GOOD;

  log_call ("(%s)", value);

  if (s->val[OPT_SOURCE].w == optindex)
    return SANE_STATUS_GOOD;

  if (s->hw->adf && strcmp_c (ADF_STR, value) == 0)
  {
    s->val[OPT_SOURCE].w = optindex;
    s->hw->src = (const extension *) s->hw->adf;
    deactivateOption (s, OPT_FILM_TYPE, &dummy);
    s->val[OPT_FOCUS].w = 0;
    if (EXT_STATUS_ADFS & s->hw->ext_status)
    {
      activateOption (s, OPT_ADF_MODE, &dummy);
      activateOption (s, OPT_ADF_DUPLEX_DIRECTION_MATCHES, &dummy);
    }
    else
    {
      deactivateOption (s, OPT_ADF_MODE, &dummy);
      s->val[OPT_ADF_MODE].w = 0;
      deactivateOption (s, OPT_ADF_DUPLEX_DIRECTION_MATCHES, &dummy);
    }
    if (FSI_CAP_ADFAS & s->hw->fsi_cap_2)
      {
        activateOption (s, OPT_ADF_AUTO_SCAN, &dummy);
      }
    if (FSI_CAP_DFD & s->hw->fsi_cap_2)
      {
        activateOption (s, OPT_ADF_DFD_SENSITIVITY, &dummy);
      }
    else
      {
        deactivateOption (s, OPT_ADF_DFD_SENSITIVITY, &dummy);
        s->val[OPT_ADF_DFD_SENSITIVITY].w = 0;
      }
  }
  else if (s->hw->tpu && strcmp_c (TPU_STR, value) == 0)
  {
    s->val[OPT_SOURCE].w = optindex;
    s->hw->src = (const extension *) s->hw->tpu;
    deactivateOption (s, OPT_ADF_MODE, &dummy);
    deactivateOption (s, OPT_ADF_AUTO_SCAN, &dummy);
    deactivateOption (s, OPT_ADF_DFD_SENSITIVITY, &dummy);
    deactivateOption (s, OPT_EJECT, &dummy);
    deactivateOption (s, OPT_AUTO_EJECT, &dummy);
    deactivateOption (s, OPT_ADF_DUPLEX_DIRECTION_MATCHES, &dummy);
  }
  else if (s->hw->fbf)
  {
    s->val[OPT_SOURCE].w = optindex;
    s->hw->src = (const extension *) s->hw->fbf;
    s->val[OPT_FOCUS].w = 0;
    deactivateOption (s, OPT_ADF_MODE, &dummy);
    deactivateOption (s, OPT_ADF_AUTO_SCAN, &dummy);
    deactivateOption (s, OPT_ADF_DFD_SENSITIVITY, &dummy);
    deactivateOption (s, OPT_ADF_DUPLEX_DIRECTION_MATCHES, &dummy);
  }
  else
    {
      err_fatal ("internal inconsistency");
      return SANE_STATUS_INVAL;
    }

  /* reset the scanner when we are changing the source setting - 
     this is necessary for the Perfection 1650 */
  if (s->hw->need_reset_on_source_change)
    initialize (s->hw);

  handle_detect_doc_size (s, NULL, &dummy);

  /*change*/
  status = handle_scan_area(s, s->val[OPT_ADF_MODE].w);

  change_profile_matrix (s);

  setOptionState (s, using (s->hw, tpu), OPT_FILM_TYPE, &dummy);
  setOptionState (s, using (s->hw, adf), OPT_AUTO_EJECT, &dummy);
  setOptionState (s, using (s->hw, adf), OPT_EJECT, &dummy);

  if (s->hw->cmd->set_focus_position)
  {
    if (using (s->hw, tpu))
    {
      s->val[OPT_FOCUS].w = 1;
      setOptionState (s, SANE_TRUE, OPT_FOCUS, &dummy);
    }
    else if (using (s->hw, adf))
    {
      s->val[OPT_FOCUS].w = 0;
      setOptionState (s, SANE_FALSE, OPT_FOCUS, &dummy);
    }
    else
    {
      s->val[OPT_FOCUS].w = 0;
      setOptionState (s, SANE_TRUE, OPT_FOCUS, &dummy);
    }
  }

  status = get_resolution_constraints (s->hw, s);
  if (SANE_STATUS_GOOD != status)
    {
      return status;
    }

  if (s->hw->adf)
    {
      if (large_res_kills_adf_scan (s->hw)) limit_adf_res (s);

      if (zoom_kills_adf_scan (s->hw))
        {
          if (using (s->hw, adf))
            {
              s->val[OPT_ZOOM].w = 100;
              deactivateOption (s, OPT_ZOOM, &dummy);
            }
          else
            {
              if (s->hw->cmd->set_zoom)
                activateOption (s, OPT_ZOOM, &dummy);
            }
        }
    }

  return status;
}


static void
handle_filmtype (Epson_Scanner * s, SANE_Int optindex, char *value)
{
  log_call ();

  value = value;

  if (!s->hw->tpu || s->val[OPT_FILM_TYPE].w == optindex)
    return;

  s->val[OPT_FILM_TYPE].w = optindex;

  require (s->hw->src == (extension *) s->hw->tpu);

  s->val[OPT_TL_X].w = 0;
  s->val[OPT_TL_Y].w = 0;
  s->val[OPT_BR_X].w = s->hw->src->x_range.max;
  s->val[OPT_BR_Y].w = s->hw->src->y_range.max;

  s->opt[OPT_TL_X].constraint.range = &(s->hw->src->x_range);
  s->opt[OPT_TL_Y].constraint.range = &(s->hw->src->y_range);
  s->opt[OPT_BR_X].constraint.range = &(s->hw->src->x_range);
  s->opt[OPT_BR_Y].constraint.range = &(s->hw->src->y_range);

  change_profile_matrix (s);
}

static void
handle_autocrop (Epson_Scanner *s, SANE_Bool *value, SANE_Bool *reload)
{
  *reload = SANE_FALSE;

  if ( dip_has_autocrop (s->dip, s->hw)
       && ( ! SANE_OPTION_IS_ACTIVE (s->opt[OPT_DESKEW].cap)
            || ! s->val[OPT_DESKEW].b )
       && !s->val[OPT_PREVIEW].b )
    {
      activateOption (s, OPT_AUTOCROP, reload);
      if (value)
        {
          SANE_Bool dummy;
          s->val[OPT_AUTOCROP].b = *value;
          handle_deskew (s, NULL, &dummy);
          *reload = SANE_TRUE;
        }
    }
  else
    {
      deactivateOption (s, OPT_AUTOCROP, reload);
    }
}

static void
handle_deskew (Epson_Scanner *s, SANE_Bool *value, SANE_Bool *reload)
{
  *reload = SANE_FALSE;

  if ( dip_has_deskew (s->dip, s->hw)
       && ( ! SANE_OPTION_IS_ACTIVE (s->opt[OPT_AUTOCROP].cap)
            || ! s->val[OPT_AUTOCROP].b )
       && ( ! SANE_OPTION_IS_ACTIVE (s->opt[OPT_BIT_DEPTH].cap)
            || 8 == s->val[OPT_BIT_DEPTH].w)
       && 600 >= s->val[OPT_RESOLUTION].w
       && 600 >= s->val[OPT_X_RESOLUTION].w
       && 600 >= s->val[OPT_Y_RESOLUTION].w
       && !s->val[OPT_PREVIEW].b )
    {
      activateOption (s, OPT_DESKEW , reload);
      if (value)
        {
          SANE_Bool dummy;
          s->val[OPT_DESKEW].b = *value;
          handle_autocrop (s, NULL, &dummy);
          *reload = SANE_TRUE;
        }
    }
  else
    {
      deactivateOption (s, OPT_DESKEW, reload);
    }
}

static void
handle_detect_doc_size (Epson_Scanner *s, SANE_Bool *value, SANE_Bool *reload)
{
  *reload = SANE_FALSE;

  if (has_size_check_support (s->hw->src)
      && !s->val[OPT_PREVIEW].b)
    {
      activateOption (s, OPT_DETECT_DOC_SIZE, reload);
      if (value)
        {
          s->val[OPT_DETECT_DOC_SIZE].b = *value;
          *reload = SANE_TRUE;
        }
    }
  else
    {
      deactivateOption (s, OPT_DETECT_DOC_SIZE, reload);
    }
}

static void
handle_preview (Epson_Scanner *s, SANE_Bool *value, SANE_Bool *reload)
{
  SANE_Bool dummy;

  if (value)
    {
      s->val[OPT_PREVIEW].b = *value;
      handle_detect_doc_size (s, &s->val[OPT_DETECT_DOC_SIZE].b, &dummy);
      handle_autocrop (s, &s->val[OPT_AUTOCROP].b, &dummy);
      handle_deskew (s, &s->val[OPT_DESKEW].b, &dummy);
      *reload = SANE_TRUE;
    }
}

static SANE_Status
setvalue (Epson_Scanner *s, SANE_Int option, void *value, SANE_Int * info)
{
  SANE_Option_Descriptor *sopt = &(s->opt[option]);
  Option_Value *sval = &(s->val[option]);

  SANE_Status status;
  const SANE_String_Const *optval;
  int optindex;
  SANE_Bool reload = SANE_FALSE;

  log_call ("(%s, value @%p)", sopt->name, value);

  status = sanei_constrain_value (sopt, value, info);

  if (status != SANE_STATUS_GOOD)
    return status;

  optval = NULL;
  optindex = 0;

  if (sopt->constraint_type == SANE_CONSTRAINT_STRING_LIST)
  {
    optval = search_string_list (sopt->constraint.string_list,
				 (char *) value);

    if (optval == NULL)
      return SANE_STATUS_INVAL;
    optindex = optval - sopt->constraint.string_list;
  }

  switch (option)
  {
  case OPT_GAMMA_VECTOR_R:
  case OPT_GAMMA_VECTOR_G:
  case OPT_GAMMA_VECTOR_B:
    memcpy (sval->wa, value, sopt->size);	/* Word arrays */
    break;

  case OPT_CCT_1:
  case OPT_CCT_2:
  case OPT_CCT_3:
  case OPT_CCT_4:
  case OPT_CCT_5:
  case OPT_CCT_6:
  case OPT_CCT_7:
  case OPT_CCT_8:
  case OPT_CCT_9:
    sval->w = *((SANE_Word *) value);	/* Simple values */
    break;

  case OPT_FILM_TYPE:
    handle_filmtype (s, optindex, (char *) value);
    reload = SANE_TRUE;
    break;

  case OPT_DROPOUT:
  case OPT_BAY:
  case OPT_FOCUS:
    sval->w = optindex;		/* Simple lists */
    break;

  case OPT_EJECT:
    dev_eject_paper (s->hw);
    break;

  case OPT_RESOLUTION:
  case OPT_X_RESOLUTION:
  case OPT_Y_RESOLUTION:
    handle_resolution (s, option, *((SANE_Word *) value));
    reload = SANE_TRUE;
    break;

  case OPT_TL_X:
  case OPT_TL_Y:
  case OPT_BR_X:
  case OPT_BR_Y:
    sval->w = *((SANE_Word *) value);
    log_info ("set = %f", SANE_UNFIX (sval->w));
    if (NULL != info)
      *info |= SANE_INFO_RELOAD_PARAMS;
    break;

  case OPT_SOURCE:
    status = handle_source (s, optindex, (char *) value);
    reload = SANE_TRUE;
    break;

  case OPT_MODE:
    {
      if (sval->w == optindex)
	break;

      sval->w = optindex;

      handle_mode (s, optindex, &reload);

      break;
    }

  case OPT_ADF_MODE:
    status = handle_scan_area(s, optindex);
    reload = SANE_TRUE;
    /* through intentionally */
  case OPT_ADF_DFD_SENSITIVITY:
    sval->w = optindex;
    break;

  case OPT_BIT_DEPTH:
    {
      SANE_Bool dummy;
      sval->w = *((SANE_Word *) value);
      mode_params[s->val[OPT_MODE].w].depth = sval->w;
      handle_deskew (s, NULL, &dummy);
      reload = SANE_TRUE;
      break;
    }
  case OPT_HALFTONE:
    if (sval->w == optindex)
      break;
    sval->w = optindex;
    handle_depth_halftone (s, optindex, &reload);
    break;

  case OPT_BRIGHTNESS_METHOD:
    {
      const SANE_Range *r;
      SANE_Bool f
        = s->hw->gamma_user_defined[s->val[OPT_GAMMA_CORRECTION].w];

      if (sval->w == optindex) break;

      r = s->opt[OPT_BRIGHTNESS].constraint.range;

      if (0 == strcmp_c (brightness_method_list[0], /* "hardware" */
                         sopt->constraint.string_list[optindex]))
        {
          s->opt[OPT_BRIGHTNESS].cap &= ~SANE_CAP_EMULATED;
          s->opt[OPT_BRIGHTNESS].constraint.range = &s->hw->cmd->bright_range;
          setOptionState (s, !f, OPT_BRIGHTNESS, &reload);
        }
      else
        {
          s->opt[OPT_BRIGHTNESS].cap |= SANE_CAP_EMULATED;
          s->opt[OPT_BRIGHTNESS].constraint.range = &brightness_range;
          setOptionState (s, SANE_TRUE, OPT_BRIGHTNESS, &reload);
        }

      if (r != s->opt[OPT_BRIGHTNESS].constraint.range)
        {
          double v = s->val[OPT_BRIGHTNESS].w;

          /**/ if (0 < v)
            {
              require (0 < r->max);

              v /= r->max;
              r = s->opt[OPT_BRIGHTNESS].constraint.range;
              v *= r->max;
              v += 0.5;

              reload = SANE_TRUE;
            }
          else if (0 > v)
            {
              require (0 > r->min);

              v /= r->min;
              r = s->opt[OPT_BRIGHTNESS].constraint.range;
              v *= r->min;
              v -= 0.5;

              reload = SANE_TRUE;
            }
          else                  /* 0 == v */
            {}

          s->val[OPT_BRIGHTNESS].w = (SANE_Int) v;
        }

      sval->w = optindex;

      break;
    }

  case OPT_COLOR_CORRECTION:
    {
      SANE_Bool f = s->hw->color_user_defined[optindex];

      sval->w = optindex;
      setOptionState (s, f, OPT_CCT_1, &reload);
      setOptionState (s, f, OPT_CCT_2, &reload);
      setOptionState (s, f, OPT_CCT_3, &reload);
      setOptionState (s, f, OPT_CCT_4, &reload);
      setOptionState (s, f, OPT_CCT_5, &reload);
      setOptionState (s, f, OPT_CCT_6, &reload);
      setOptionState (s, f, OPT_CCT_7, &reload);
      setOptionState (s, f, OPT_CCT_8, &reload);
      setOptionState (s, f, OPT_CCT_9, &reload);

      break;
    }

  case OPT_GAMMA_CORRECTION:
    {
      SANE_Bool f = s->hw->gamma_user_defined[optindex];

      sval->w = optindex;
      setOptionState (s, f, OPT_GAMMA_VECTOR_R, &reload);
      setOptionState (s, f, OPT_GAMMA_VECTOR_G, &reload);
      setOptionState (s, f, OPT_GAMMA_VECTOR_B, &reload);

      if (0 == strcmp_c (brightness_method_list[0], /* "hardware" */
                         s->opt[OPT_BRIGHTNESS_METHOD].constraint
                         .string_list[s->val[OPT_BRIGHTNESS_METHOD].w]))
        {
          setOptionState (s, !f, OPT_BRIGHTNESS, &reload);
        }

      break;
    }

  case OPT_AUTO_EJECT:
    sval->w = *((SANE_Word *) value);
    if (s->hw && s->hw->adf) s->hw->adf->auto_eject = sval->w;
    break;
  case OPT_MIRROR:
  case OPT_SPEED:
  case OPT_PREVIEW_SPEED:
  case OPT_AAS:
  case OPT_BRIGHTNESS:
  case OPT_CONTRAST:
  case OPT_SHARPNESS:
  case OPT_THRESHOLD:
  case OPT_ZOOM:
  case OPT_WAIT_FOR_BUTTON:
  case OPT_ADF_AUTO_SCAN:
    sval->w = *((SANE_Word *) value);
    break;

  case OPT_DETECT_DOC_SIZE:
    handle_detect_doc_size (s, (SANE_Word *) value, &reload);
    break;

  case OPT_PREVIEW:
    handle_preview (s, (SANE_Word *) value, &reload);
    break;

  case OPT_DESKEW:
    handle_deskew (s, (SANE_Word *) value, &reload);
    break;

  case OPT_AUTOCROP:
    handle_autocrop (s, (SANE_Word *) value, &reload);
    break;

  case OPT_LIMIT_RESOLUTION:
    sval->w = *((SANE_Word *) value);
    filter_resolution_list (s);
    reload = SANE_TRUE;
    break;

  case OPT_SCAN_AREA:
    {
      sval->w = optindex;

      /**/ if (0 == strcmp_c (sopt->constraint.string_list[sval->w],
                              media_maximum))
        {
          s->val[OPT_TL_X].w = SANE_FIX (0.0);
          s->val[OPT_TL_Y].w = SANE_FIX (0.0);
          s->val[OPT_BR_X].w = s->hw->src->x_range.max;
          s->val[OPT_BR_Y].w = s->hw->src->y_range.max;
        }
      else if (0 == strcmp_c (sopt->constraint.string_list[sval->w],
                              media_automatic))
        {
          SANE_Bool yes = SANE_TRUE;

          if (SANE_OPTION_IS_ACTIVE (s->opt[OPT_DETECT_DOC_SIZE].cap))
            {
              setvalue (s, OPT_DETECT_DOC_SIZE, &yes, NULL);
            }
        }
      else
        {
          size_t i = 0;

          while (i < num_of (media_list)
                 && 0 != strcmp_c (sopt->constraint.string_list[sval->w],
                                   media_list[i].name))
            ++i;

          require (i < num_of (media_list));

          s->val[OPT_TL_X].w = SANE_FIX (0.0);
          s->val[OPT_TL_Y].w = SANE_FIX (0.0);
          s->val[OPT_BR_X].w = SANE_FIX (media_list[i].width);
          s->val[OPT_BR_Y].w = SANE_FIX (media_list[i].height);

          adf_handle_manual_centering (s, SANE_FALSE);
        }

      reload = SANE_TRUE;
      break;
    }
  case OPT_QUICK_FORMAT:
    setvalue (s, OPT_SCAN_AREA,  value, info);
    sval->w = s->val[OPT_SCAN_AREA].w;
    break;

  case OPT_CALIBRATE:
    status = dev_calibrate (s->hw);
    break;

  case OPT_CLEAN:
    status = dev_clean (s->hw);
    break;

  case OPT_MONITOR_BUTTON:
  default:
    return SANE_STATUS_INVAL;
  }

  if (reload && info != NULL)
  {
    *info |= SANE_INFO_RELOAD_OPTIONS | SANE_INFO_RELOAD_PARAMS;
  }

  return status;
}


SANE_Status
sane_control_option (SANE_Handle handle,
		     SANE_Int option,
		     SANE_Action action, void *value, SANE_Int * info)
{
  log_call ();

  if (option < 0 || option >= NUM_OPTIONS)
    return SANE_STATUS_INVAL;

  if (info != NULL)
    *info = 0;

  switch (action)
  {
  case SANE_ACTION_GET_VALUE:
    return (getvalue (handle, option, value));

  case SANE_ACTION_SET_VALUE:
    return (setvalue (handle, option, value, info));
  default:
    return SANE_STATUS_INVAL;
  }

  return SANE_STATUS_GOOD;
}

/* This function is part of the SANE API and gets called when the front end
 * requests information aobut the scan configuration (e.g. color depth, mode,
 * bytes and pixels per line, number of lines. This information is returned
 * in the SANE_Parameters structure.
 *
 * Once a scan was started, this routine has to report the correct values, if
 * it is called before the scan is actually started, the values are based on
 * the current settings.
 *
 */
SANE_Status
estimate_parameters (Epson_Scanner *s, SANE_Parameters * params)
{
  int zoom, max_x, max_y;
  int bytes_per_pixel;

  int x_dpi = 0;
  int y_dpi = 0;

  SANE_Int max_y_orig = -1;

  log_call ();

  memset (&s->raw.ctx, 0, sizeof (SANE_Parameters));

  x_dpi = s->val[OPT_X_RESOLUTION].w;
  y_dpi = s->val[OPT_Y_RESOLUTION].w;

  zoom = s->val[OPT_ZOOM].w;

  if (need_autocrop_override (s))
  {                           /* yucky changes to be reverted below */
    max_y_orig = s->hw->src->max_y;
    ((extension *) s->hw->src)->max_y = autocrop_max_y (s->hw);
    update_ranges (s->hw, s->hw->src);
    s->val[OPT_BR_Y].w = s->hw->src->y_range.max;
  }

  calculate_scan_area_max (s, &max_x, &max_y);

  s->raw.ctx.pixels_per_line =
    SANE_UNFIX (s->val[OPT_BR_X].w -
		s->val[OPT_TL_X].w) / MM_PER_INCH * x_dpi * zoom / 100;
  s->raw.ctx.lines =
    SANE_UNFIX (s->val[OPT_BR_Y].w -
		s->val[OPT_TL_Y].w) / MM_PER_INCH * y_dpi * zoom / 100;

  log_data ("max x:%d y:%d [in pixels]", max_x, max_y);

  if (max_x != 0 && max_y != 0)
  {
    if (max_x < s->raw.ctx.pixels_per_line)
      s->raw.ctx.pixels_per_line = max_x;
    if (max_y < s->raw.ctx.lines)
      s->raw.ctx.lines = max_y;
  }

  if (s->raw.ctx.pixels_per_line < 8)
    s->raw.ctx.pixels_per_line = 8;
  if (s->raw.ctx.lines < 1)
    s->raw.ctx.lines = 1;

  log_data ("Preview = %d", s->val[OPT_PREVIEW].w);
  log_data ("X Resolution = %d", s->val[OPT_X_RESOLUTION].w);
  log_data ("Y Resolution = %d", s->val[OPT_Y_RESOLUTION].w);

  log_data ("Scan area: TL (%.2f, %.2f) -- BR (%.2f, %.2f) [in mm]",
            SANE_UNFIX (s->val[OPT_TL_X].w),
            SANE_UNFIX (s->val[OPT_TL_Y].w),
            SANE_UNFIX (s->val[OPT_BR_X].w),
            SANE_UNFIX (s->val[OPT_BR_Y].w));

  /* Calculate bytes_per_pixel and bytes_per_line for any color
   * depths.  The default color depth is stored in mode_params.depth.
   */
  if (mode_params[s->val[OPT_MODE].w].depth == 1)
  {
    s->raw.ctx.depth = 1;
  }
  else
  {
    s->raw.ctx.depth = s->val[OPT_BIT_DEPTH].w;
  }

  if (s->raw.ctx.depth > 8)
  { 
    /* The frontends can only handle 8 or 16 bits for gray or color -
     * so if it's more than 8, it gets automatically set to 16. This
     * works as long as EPSON does not come out with a scanner that
     * can handle more than 16 bits per color channel.
     */
    s->raw.ctx.depth = 16; 
  }

  bytes_per_pixel = s->raw.ctx.depth / 8;	/* this works because it can only be set to 1, 8 or 16 */
  if (s->raw.ctx.depth % 8)	/* just in case ... */
  {
    bytes_per_pixel++;
  }

  /* All models require alignment on a multiple of 8 pixels per line.
     However, some models require multiples of 32 pixels instead of 8
     when scanning in monochrome mode.
     We use the largest multiple that is not larger than the original
     value.
   */
  s->raw.ctx.pixels_per_line &= ~7;
  if (1 == s->raw.ctx.depth)
  {
    s->raw.ctx.pixels_per_line &= ~31;
  }

  s->raw.ctx.last_frame = SANE_TRUE;

  if (mode_params[s->val[OPT_MODE].w].color)
  {
    s->raw.ctx.format = SANE_FRAME_RGB;
    s->raw.ctx.bytes_per_line =
      3 * s->raw.ctx.pixels_per_line * bytes_per_pixel;
  }
  else
  {
    s->raw.ctx.format = SANE_FRAME_GRAY;
    s->raw.ctx.bytes_per_line =
      s->raw.ctx.pixels_per_line * s->raw.ctx.depth / 8;
  }

  if (NULL != params)
    memcpy (params, &s->raw.ctx, sizeof (SANE_Parameters));

  print_params (s->raw.ctx);

  if (need_autocrop_override (s))
  {                             /* revert yucky changes made above */
    ((extension *) s->hw->src)->max_y = max_y_orig;
    update_ranges (s->hw, s->hw->src);
    s->val[OPT_BR_Y].w = s->hw->src->y_range.max;
  }

  return SANE_STATUS_GOOD;
}

static SANE_Status
device_init (Epson_Scanner *s)
{
  SANE_Status status = SANE_STATUS_GOOD;

  log_call ();

  status = initialize (s->hw);
  if (SANE_STATUS_GOOD != status)
  {
    return status;
  }

/* There is some undocumented special behavior with the TPU enable/disable.
 *      TPU power	ESC e		status
 *	on		0		NAK
 *	on		1		ACK
 *	off		0		ACK
 *	off		1		NAK
 *
 * It makes no sense to scan with TPU powered on and source flatbed, because 
 * light will come from both sides.
 */

  if (s->hw->adf || s->hw->tpu)
  {
    /* if it is previewing now, disable duplex */
    SANE_Bool adf_duplex = ((1 == s->val[OPT_ADF_MODE].w)
                            && !s->val[OPT_PREVIEW].b);
    status = control_option_unit (s->hw, adf_duplex);

    if (SANE_STATUS_GOOD != status)
    {
      if (s->hw->tpu)
        err_major ("You may have to power %s your TPU",
                   (using (s->hw, tpu) ? "on" : "off"));

      err_major ("You may have to restart the SANE frontend.");
      return status;
    }

    if (s->hw->cmd->request_extended_status != 0)
    {
      status = check_ext_status (s->hw);

      if (SANE_STATUS_GOOD != status && SANE_STATUS_DEVICE_BUSY != status)
      {
	return status;
      }
    }
  }

  if (s->hw->using_fs)
    {
      cmd_request_scanning_parameter (s->hw);
      s->hw->param_buf[39] =  s->val[OPT_ADF_DFD_SENSITIVITY].w;
      s->hw->param_buf[40] = (s->val[OPT_ADF_AUTO_SCAN].w ? 0xFF : 0x00);
    }

  return status;
}

static SANE_Status
device_set_focus (Epson_Scanner *s)
{
  SANE_Status status = SANE_STATUS_GOOD;

  log_call ();

  if (!s->hw->tpu || !using (s->hw, tpu)) return status;

   /* set the focus position according to the extension used:
    * if the TPU is selected, then focus 2.5mm above the glass,
    * otherwise focus on the glass. Scanners that don't support
    * this feature, will just ignore these calls.
    */

  if (s->hw->adf || s->hw->tpu)
  {
    if (s->hw->tpu && s->hw->tpu->has_focus)
    {
      if (s->val[OPT_FOCUS].w == 0)
      {
        log_info ("Setting focus to glass surface");
	status = set_focus_position (s->hw, 0x40);
      }
      else
      {
	log_info ("Setting focus to 2.5mm above glass");
	status = set_focus_position (s->hw, 0x59);
      }

      if (SANE_STATUS_GOOD != status)
      {
	return status;
      }
    }
  }
  return status;
}

static SANE_Status
set_scan_parameters (Epson_Scanner *s, int *x_res, int *y_res)
{
  SANE_Status status = SANE_STATUS_GOOD;

  const struct mode_param *mparam;
  int x_dpi, y_dpi;

  log_call ();

  mparam = mode_params + s->val[OPT_MODE].w;
  log_data ("setting data format to %d bits", mparam->depth);
  status = set_data_format (s->hw, mparam->depth);

  if (SANE_STATUS_GOOD != status)
  {
    err_fatal ("set_data_format failed (%s)",
               sane_strstatus (status));
    return status;
  }

  /* The byte sequence mode was introduced in B5, for B[34] we need
     line sequence mode
   */
  if ((s->hw->cmd->level[0] == 'D' ||
       (s->hw->cmd->level[0] == 'B' && s->hw->level >= 5)) &&
      mparam->mode_flags == 0x02)
  {
    status = set_color_mode (s->hw, 0x13);
  }
  else
  {
    status = set_color_mode (s->hw, (mparam->mode_flags
				 | (mparam->dropout_mask
				    & dropout_params[s->val[OPT_DROPOUT].w])));
  }

  if (SANE_STATUS_GOOD != status)
  {
    err_fatal ("set_color_mode failed (%s)",
               sane_strstatus (status));
    return status;
  }

  if (s->hw->cmd->set_halftoning &&
      SANE_OPTION_IS_ACTIVE (s->opt[OPT_HALFTONE].cap))
  {
    status = set_halftoning (s->hw, halftone_params[s->val[OPT_HALFTONE].w]);

    if (SANE_STATUS_GOOD != status)
    {
      err_fatal ("set_halftoning failed (%s)",
	   sane_strstatus (status));
      return status;
    }
  }

  s->brightness = 0;
  if (SANE_OPTION_IS_ACTIVE (s->opt[OPT_BRIGHTNESS].cap))
  {
    SANE_Option_Descriptor *sopt = &(s->opt[OPT_BRIGHTNESS_METHOD]);
    Option_Value *sval = &(s->val[OPT_BRIGHTNESS_METHOD]);

    if (0 == strcmp_c (brightness_method_list[0], /* "hardware" */
                       sopt->constraint.string_list[sval->w]))
      {
        status = set_bright (s->hw, s->val[OPT_BRIGHTNESS].w);

        if (SANE_STATUS_GOOD != status)
          {
            err_fatal ("set_bright failed (%s)",
                       sane_strstatus (status));
            return status;
          }
      }
    else                        /* software emulation */
      {
        s->brightness  = s->val[OPT_BRIGHTNESS].w;
        s->brightness /= s->opt[OPT_BRIGHTNESS].constraint.range->max;
      }
  }

  s->contrast = 0;
  if (SANE_OPTION_IS_ACTIVE (s->opt[OPT_CONTRAST].cap))
  {
    s->contrast  = s->val[OPT_CONTRAST].w;
    s->contrast /= s->opt[OPT_CONTRAST].constraint.range->max;
  }

  s->lut = dip_destroy_LUT (s->dip, s->lut);
  if (0 != s->brightness || 0 != s->contrast)   /* non-linear LUT */
    {
      SANE_Option_Descriptor *sopt = &(s->opt[OPT_BRIGHTNESS_METHOD]);
      Option_Value *sval = &(s->val[OPT_BRIGHTNESS_METHOD]);

      if (0 == strcmp_c (brightness_method_list[2], /* "gimp" */
                         sopt->constraint.string_list[sval->w]))
        s->lut = dip_gimp_BC_LUT (s->dip, s->raw.ctx.depth,
                                  s->brightness, s->contrast);
      else
        /* We use "iscan" for contrast by default.  If "hardware" was
         * given then s->brightness is 0 and doesn't produce knock-on
         * effects with this method.
         */
        s->lut = dip_iscan_BC_LUT (s->dip, s->raw.ctx.depth,
                                   s->brightness, s->contrast);

      if (!s->lut)
        status = SANE_STATUS_NO_MEM;

      if (SANE_STATUS_GOOD != status)
        {
          err_fatal ("set_contrast failed (%s)",
                     sane_strstatus (status));
          return status;
        }
    }

  if (SANE_OPTION_IS_ACTIVE (s->opt[OPT_MIRROR].cap))
  {
    status = mirror_image (s->hw, mirror_params[s->val[OPT_MIRROR].w]);

    if (SANE_STATUS_GOOD != status)
    {
      err_fatal ("mirror_image failed (%s)",
                 sane_strstatus (status));
      return status;
    }
  }

  if (SANE_OPTION_IS_ACTIVE (s->opt[OPT_SPEED].cap))
  {

    if (s->val[OPT_PREVIEW].w)
      status = set_speed (s->hw, speed_params[s->val[OPT_PREVIEW_SPEED].w]);
    else
      status = set_speed (s->hw, speed_params[s->val[OPT_SPEED].w]);

    if (SANE_STATUS_GOOD != status)
    {
      err_fatal ("set_speed failed (%s)", sane_strstatus (status));
      return status;
    }
  }

  /*  use of speed_params is ok here since they are false and true.
   *  NOTE: I think I should throw that "params" stuff as long w is
   *  already the value.
   */

  s->invert_image = SANE_FALSE;	/* default: to not inverting the image */

  if (SANE_OPTION_IS_ACTIVE (s->opt[OPT_FILM_TYPE].cap))
  {
    status = set_film_type (s->hw, film_params[s->val[OPT_FILM_TYPE].w]);
    if (SANE_STATUS_GOOD != status)
    {
      err_fatal ("set_film_type failed (%s)",
                 sane_strstatus (status));
      return status;
    }
  }

  if (SANE_OPTION_IS_ACTIVE (s->opt[OPT_BAY].cap))
  {
    status = set_bay (s->hw, s->val[OPT_BAY].w);

    if (SANE_STATUS_GOOD != status)
    {
      err_fatal ("set_bay failed (%s)", sane_strstatus (status));
      return status;
    }
  }

  if (SANE_OPTION_IS_ACTIVE (s->opt[OPT_SHARPNESS].cap))
  {

    status = set_outline_emphasis (s->hw, s->val[OPT_SHARPNESS].w);

    if (SANE_STATUS_GOOD != status)
    {
      err_fatal ("set_outline_emphasis failed (%s)",
                 sane_strstatus (status));
      return status;
    }
  }

  if (s->hw->cmd->set_gamma &&
      SANE_OPTION_IS_ACTIVE (s->opt[OPT_GAMMA_CORRECTION].cap))
  {
    int val;
    if (s->hw->cmd->level[0] == 'D')
    {
      /* The D1 level has only the two user defined gamma settings.
       */
      val = s->hw->gamma_type[s->val[OPT_GAMMA_CORRECTION].w];
    }
    else
    {
      val = s->hw->gamma_type[s->val[OPT_GAMMA_CORRECTION].w];

      /* If "Default" is selected then determine the actual value to
       * send to the scanner: If bilevel mode, just send the value
       * from the table (0x01), for grayscale or color mode add one
       * and send 0x02.
       */
      if (s->val[OPT_GAMMA_CORRECTION].w == 0)
      {
	val += mparam->depth == 1 ? 0 : 1;
      }
    }

    log_info ("set_gamma (s, 0x%x)", val);
    status = set_gamma (s->hw, val);

    if (SANE_STATUS_GOOD != status)
    {
      err_fatal ("set_gamma failed (%s)", sane_strstatus (status));
      return status;
    }
  }

  if (s->hw->cmd->set_gamma_table &&
      s->hw->gamma_user_defined[s->val[OPT_GAMMA_CORRECTION].w])
  {				/* user defined. */
    status = set_gamma_table (s->hw, s);

    if (SANE_STATUS_GOOD != status)
    {
      err_fatal ("set_gamma_table failed (%s)",
                 sane_strstatus (status));
      return status;
    }
  }

  if (s->hw->cmd->set_color_correction)
  {
    int val = s->hw->color_type[s->val[OPT_COLOR_CORRECTION].w];

    log_data ("set_color_correction (s, 0x%x)", val);
    status = set_color_correction (s->hw, val);

    if (SANE_STATUS_GOOD != status)
    {
      err_fatal ("set_color_correction failed (%s)",
                 sane_strstatus (status));
      return status;
    }
  }

  if (s->hw->color_user_defined[s->val[OPT_COLOR_CORRECTION].w])
  {
    size_t i;

    status = set_color_correction_coefficients (s->hw, s);

    if (SANE_STATUS_GOOD != status)
    {
      err_fatal ("set_color_correction_coefficients failed (%s)",
                 sane_strstatus (status));
      return status;
    }

    log_info ("Color correction coefficients");
    for (i = 0; i < 9; ++i)
      log_info ("cct[%zd] = %f", i, s->cct[i]);
  }

  if (s->hw->cmd->set_threshold != 0
      && SANE_OPTION_IS_ACTIVE (s->opt[OPT_THRESHOLD].cap))
  {
    status = set_threshold (s->hw, s->val[OPT_THRESHOLD].w);

    if (SANE_STATUS_GOOD != status)
    {
      err_fatal ("set_threshold failed (%s)",
                 sane_strstatus (status));
      return status;
    }
  }

  x_dpi = s->val[OPT_X_RESOLUTION].w;
  y_dpi = s->val[OPT_Y_RESOLUTION].w;

  if (s->hw->using_fs)
    status = dev_set_scanning_resolution (s->hw, x_dpi, y_dpi);
  else
    status = set_resolution (s->hw, x_dpi, y_dpi);

  *x_res = x_dpi;
  *y_res = y_dpi;

  if (SANE_STATUS_GOOD != status)
  {
    err_fatal ("set_resolution(%d, %d) failed (%s)",
               x_dpi, y_dpi, sane_strstatus (status));
    return status;
  }

  if (s->hw->cmd->set_zoom != 0)
  {
    status = set_zoom (s->hw, s->val[OPT_ZOOM].w, s->val[OPT_ZOOM].w);
    if (status != SANE_STATUS_GOOD)
    {
      err_fatal ("set_zoom(%d) failed (%s)",
                 s->val[OPT_ZOOM].w, sane_strstatus (status));
      return status;
    }
  }
  return status;
}

static void
wait_for_button (Epson_Scanner *s)
{
  SANE_Bool button_status;

  log_call ();

  /* If WAIT_FOR_BUTTON is active, then do just that: Wait until the
   * button is pressed. If the button was already pressed, then we
   * will get the button Pressed event right away.
   */
  if (s->val[OPT_WAIT_FOR_BUTTON].w == SANE_TRUE)
  {
    s->hw->wait_for_button = SANE_TRUE;

    while (s->hw->wait_for_button == SANE_TRUE)
    {
      if (s->raw.cancel_requested)
      {
	s->hw->wait_for_button = SANE_FALSE;
      }
      /* get the button status from the scanner */
      else if (get_push_button_status (s->hw, &button_status) ==
	       SANE_STATUS_GOOD)
      {
	if (button_status == SANE_TRUE)
	{
	  s->hw->wait_for_button = SANE_FALSE;
	}
	else
	{
          microsleep (s->hw->polling_time);
	}
      }
      else
      {
	/* we run into an eror condition, just continue */
	s->hw->wait_for_button = SANE_FALSE;
      }
    }
  }
}

static SANE_Status
set_line_count (Epson_Scanner *s)
{
  SANE_Status status = SANE_STATUS_GOOD;

  int lcount = 1;
  s->hw->block_mode = SANE_FALSE;

  log_call ();

  /* The set line count commands needs to be sent for certain scanners
   * in color mode. The D1 level requires it, we are however only
   * testing for 'D' and not for the actual numeric level.
   */
  if (((s->hw->cmd->level[0] == 'B') &&
       ((s->hw->level >= 5) || ((s->hw->level >= 4) &&
				(!mode_params[s->val[OPT_MODE].w].color))))
      || (s->hw->cmd->level[0] == 'D'))
  {
    channel *ch = s->hw->channel; /* for the sake of brevity */
    s->hw->block_mode = SANE_TRUE;
    lcount = ch->max_request_size (ch) / s->raw.ctx.bytes_per_line;

    if (0 >= lcount) lcount = 1;

    if (lcount > 255)
    {
      lcount = 255;
    }

    if (using (s->hw, tpu) && lcount > 32)
    {
      lcount = 32;
    }

    /* The D1 series of scanners only allow an even line number
     * for bi-level scanning. If a bit depth of 1 is selected, then
     * make sure the next lower even number is selected.
     */
    {
      if (3 < lcount && lcount % 2)
      {
	lcount -= 1;
      }
    }
    s->line_count = lcount;

    if (lcount == 0)
    {
      err_fatal ("can not set zero line count");
      return SANE_STATUS_NO_MEM;
    }

    status = set_lcount (s->hw, lcount);

    if (SANE_STATUS_GOOD != status)
    {
      err_fatal ("set_lcount(%d) failed (%s)",
                 lcount, sane_strstatus (status));
      return status;
    }
  }
  return status;
}

static SANE_Status
request_command_parameters (Epson_Scanner *s)
{
  SANE_Status status = SANE_STATUS_GOOD;

  log_call ();

  if (s->hw->cmd->request_condition != 0)
  {
    u_char params[2];
    u_char result[4];
    u_char *buf;
    size_t len;

    params[0] = ESC;
    params[1] = s->hw->cmd->request_condition;

    /* send request condition */
    channel_send (s->hw->channel, params, 2, &status);

    if (SANE_STATUS_GOOD != status)
    {
      return status;
    }

    len = 4;
    channel_recv (s->hw->channel, result, len, &status);

    if (SANE_STATUS_GOOD != status)
    {
      return status;
    }
    s->hw->status = result[1];

    len = result[3] << 8 | result[2];
    buf = t_alloca (len, u_char);
    channel_recv (s->hw->channel, buf, len, &status);

    if (SANE_STATUS_GOOD != status)
    {
      return status;
    }

    log_info ("SANE_START: Color: %d", (int) buf[1]);
    log_info ("SANE_START: Resolution (x, y): (%d, %d)",
              (int) (buf[4] << 8 | buf[3]), (int) (buf[6] << 8 | buf[5]));
    log_info ("SANE_START: Scan offset (x, y): (%d, %d)",
              (int) (buf[9] << 8 | buf[8]), (int) (buf[11] << 8 | buf[10]));
    log_info ("SANE_START: Scan size (w, h): (%d, %d)",
              (int) (buf[13] << 8 | buf[12]), (int) (buf[15] << 8 | buf[14]));
    log_info ("SANE_START: Data format: %d", (int) buf[17]);
    log_info ("SANE_START: Halftone: %d", (int) buf[19]);
    log_info ("SANE_START: Brightness: %d", (int) buf[21]);
    log_info ("SANE_START: Gamma: %d", (int) buf[23]);
    log_info ("SANE_START: Zoom (x, y): (%d, %d)", (int) buf[26],
              (int) buf[25]);
    log_info ("SANE_START: Color correction: %d", (int) buf[28]);
    log_info ("SANE_START: Sharpness control: %d", (int) buf[30]);
    log_info ("SANE_START: Scanning mode: %d", (int) buf[32]);
    log_info ("SANE_START: Mirroring: %d", (int) buf[34]);
    log_info ("SANE_START: Auto area segmentation: %d", (int) buf[36]);
    log_info ("SANE_START: Threshold: %d", (int) buf[38]);
    log_info ("SANE_START: Line counter: %d", (int) buf[40]);
    log_info ("SANE_START: Option unit control: %d", (int) buf[42]);
    log_info ("SANE_START: Film type: %d", (int) buf[44]);
  }
  return status;
}

static SANE_Status
device_sheet_setup (Epson_Scanner *s)
{
  SANE_Status status = SANE_STATUS_GOOD;

  int left, top;

  int x_dpi = 0;
  int y_dpi = 0;

  const struct mode_param *mparam = NULL;

  status = dev_load_paper (s->hw);
  if (SANE_STATUS_GOOD != status && SANE_STATUS_DEVICE_BUSY != status)
    {
      return status;
    }
  status = device_set_focus (s);
  if (SANE_STATUS_GOOD != status && SANE_STATUS_DEVICE_BUSY != status)
    {
      return status;
    }
  status = dev_request_extended_status (s->hw);
  if (SANE_STATUS_GOOD != status && SANE_STATUS_DEVICE_BUSY != status)
  {
    return status;
  }

  if (SANE_OPTION_IS_ACTIVE (s->opt[OPT_AUTOCROP].cap)
      && s->val[OPT_AUTOCROP].b)
  {
    s->val[OPT_TL_X].w = SANE_FIX (0);
    s->val[OPT_TL_Y].w = SANE_FIX (0);
    s->val[OPT_BR_X].w = s->hw->src->x_range.max;
    s->val[OPT_BR_Y].w = s->hw->src->y_range.max;
  }
  else if (has_size_check_support (s->hw->src)
           && SANE_OPTION_IS_ACTIVE (s->opt[OPT_DETECT_DOC_SIZE].cap)
           && s->val[OPT_DETECT_DOC_SIZE].w)
  {
    s->val[OPT_TL_X].w = SANE_FIX (0);
    s->val[OPT_TL_Y].w = SANE_FIX (0);
    s->val[OPT_BR_X].w = SANE_FIX (s->hw->src->doc_x);
    s->val[OPT_BR_Y].w = SANE_FIX (s->hw->src->doc_y);
  }

  adf_handle_manual_centering (s, SANE_TRUE);

  status = set_scan_parameters (s, &x_dpi, &y_dpi);
  if (SANE_STATUS_GOOD != status)
  {
    return status;
  }

  calculate_scan_area_offset (s->val, &left, &top);

  /* Calculate correction for line_distance in D1 scanner: Start
   * line_distance lines earlier and add line_distance lines at the
   * end.
   *
   * Because the actual line_distance is not yet calculated we have to
   * do this first.
   */

  s->hw->color_shuffle = SANE_FALSE;
  s->current_output_line = 0;
  s->color_shuffle_line = 0;

  mparam = mode_params + s->val[OPT_MODE].w;
  if ((s->hw->optical_res != 0) && (mparam->depth == 8)
      && (mparam->mode_flags != 0))
  {
    s->line_distance = s->hw->max_line_distance * x_dpi / s->hw->optical_res;
    if (s->line_distance != 0)
    {
      s->hw->color_shuffle = SANE_TRUE;
    }
    else
      s->hw->color_shuffle = SANE_FALSE;
  }

  estimate_parameters (s, NULL);
  {                             /* finalise the number of scanlines */
    int lines = s->raw.ctx.lines;
    int max_lines = lines;

    if (!need_autocrop_override (s))
      {
        max_lines = (SANE_UNFIX(s->hw->src->y_range.max)
                     * s->val[OPT_Y_RESOLUTION].w * s->val[OPT_ZOOM].w / 100
                     / MM_PER_INCH) + 0.5;
      }
    /* add extra lines at the top and bottom to minimise the loss of
       scanlines due to colour shuffling */
    if (SANE_TRUE == s->hw->color_shuffle)
    {
      top   -= 1 * s->line_distance;
      lines += 2 * s->line_distance;
    }

    /* make sure values are within range
       In the worst case, this chomps 2 * s->line_distance lines from
       the area the user wanted to scan.  C'est la vie.  */
    top   = max (0, top);
    lines = min (lines, max_lines - top);

    if (s->hw->using_fs)
      {
        status = dev_set_scanning_area (s->hw, left, top,
                                        s->raw.ctx.pixels_per_line, lines);
      }
    else
      {
        status = set_scan_area (s->hw, left, top,
                                s->raw.ctx.pixels_per_line, lines);
      }

    /* substract the additional lines needed for colour shuffling so
       the frontend can know how many lines to expect */
    if (SANE_TRUE == s->hw->color_shuffle)
    {
      lines -= 2 * s->line_distance;
    }
    s->raw.ctx.lines = lines;

    if (SANE_STATUS_GOOD != status)
    {
      err_fatal ("set_scan_area failed (%s)",
                 sane_strstatus (status));
      return status;
    }
  }

  status = set_line_count (s);
  if (SANE_STATUS_GOOD != status)
  {
    return status;
  }
  
  if (s->hw->using_fs)
    {
      /* if it is previewing now, disable duplex */
      byte adf_duplex = ((1 == s->val[OPT_ADF_MODE].w)
                         && !s->val[OPT_PREVIEW].b) ? 0x01 : 0x00;
      dev_set_option_unit (s->hw, adf_duplex);
      status = cmd_set_scanning_parameter (s->hw);
    }

  if (SANE_STATUS_GOOD != status)
  {
    return status;
  }

  if (s->hw->using_fs)
    {
      status = cmd_request_scanning_parameter (s->hw);
      dev_log_scanning_parameter (s->hw);
    }
  else
    status = request_command_parameters (s);

  if (SANE_STATUS_GOOD != status)
  {
    return status;
  }

  if (s->hw->channel->interpreter)
  {
    status = s->hw->channel->interpreter->ftor1 (s->hw->channel, &s->raw.ctx,
                                                 mparam->depth, left, x_dpi,
                                                 s->hw->optical_res);
    if (SANE_STATUS_GOOD != status)
    {
      return status;
    }
  }

  wait_for_button (s);

  return status;
}

/*! \brief  Implements image data buffer resizing policy.
 *
 *  The implementation basically reuses previously acquired image data
 *  acquisition buffers as much as possible.  This mitigates the risk
 *  of a (consecutive) scan aborting someway halfway through because
 *  memory allocation failed.
 */
static
bool
resize_warranted (size_t needed, size_t capacity)
{
  return needed > capacity;
}

SANE_Status
sane_start (SANE_Handle handle)
{
  Epson_Scanner *s = (Epson_Scanner *) handle;
  SANE_Status status;
  u_char params[4];
  size_t len_raw;

  log_call ();

  s->raw.cancel_requested = false;
  s->raw.all_data_fetched = false;
  s->raw.transfer_started = false;
  s->raw.transfer_stopped = false;

  s->img.cancel_requested = false;
  s->img.all_data_fetched = false;
  s->img.transfer_started = false;
  s->img.transfer_stopped = false;

  s->src = &s->raw;

  if (0 == s->frame_count)
  {
    /* Back up original SANE options.
     * \todo string and word array have to copy individually.
     */
    memcpy (s->val_bak, s->val, (sizeof (s->val[0]) * NUM_OPTIONS));
  }
  else if (!s->val[OPT_ADF_AUTO_SCAN].w)
  {
    /* Restore original SANE options.
     * \todo string and word array have to copy individually.
     */
    memcpy (s->val, s->val_bak, (sizeof (s->val[0]) * NUM_OPTIONS));
  }

  if (!scan_area_is_valid (s))
    {
      err_fatal ("The image data resulting from the combination of the "
                 "specified scan area and resolution is too large.");
      return SANE_STATUS_INVAL;
    }

  /* for AFF devices, check paper status here instead of in scan_finish
   * to avoid blocking there in dev_request_extended_status()
   */
  if (0 < s->frame_count
      && using (s->hw, adf)
      && adf_has_auto_form_feed (s->hw))
    adf_handle_out_of_paper (s);

  if (0 == s->frame_count)
  {
    status = device_init (s);
    if (SANE_STATUS_GOOD != status && SANE_STATUS_DEVICE_BUSY != status)
    {
      return status;
    }
  }

  if (ENABLE_TIMING && 0 == time_pass_count)
    {
      time_clear ();
      time_stamp (time_scan, start);
    }

  if (!s->val[OPT_ADF_AUTO_SCAN].w || 0 == s->frame_count)
    {
      status = device_sheet_setup (s);
      if (SANE_STATUS_GOOD != status)
        {
          return status;
        }
    }

  if (SANE_STATUS_GOOD != (status = check_warmup (s->hw))
      && !(SANE_STATUS_NO_DOCS == status
           && using (s->hw, adf)))
  {
    return status;
  }

  status = dev_lock (s->hw);
  if (SANE_STATUS_GOOD != status)
  {
    return status;
  }

  params[0] = ESC;
  if (s->hw->using_fs) params[0] = FS;
  params[1] = s->hw->cmd->start_scanning;

  if (ENABLE_TIMING && TIME_PASS_MAX > time_pass_count)
    time_stamp (time_pass[time_pass_count], start);

  channel_send (s->hw->channel, params, 2, &status);

  if (SANE_STATUS_GOOD != status)
  {
    err_fatal ("start failed (%s)", sane_strstatus (status));
    return status;
  }

  if (s->hw->color_shuffle == SANE_TRUE)
  {
    size_t len_line_buffer = s->raw.ctx.bytes_per_line;

    if (resize_warranted (len_line_buffer, s->cap_line_buffer))
      {
        size_t i;

        for (i = 0; i < 2 * s->line_distance + 1; ++i)
          delete (s->line_buffer[i]);
        s->cap_line_buffer = 0;

        for (i = 0; i < 2 * s->line_distance + 1; ++i)
          {
            SANE_Byte *p = t_malloc (len_line_buffer, SANE_Byte);

            if (p)
              {
                s->line_buffer[i] = p;
              }
            else                /* clean up and bail */
              {
                size_t j;

                for (j = 0; j < i; ++i)
                  delete (s->line_buffer[i]);

                s->cap_line_buffer = 0;

                return SANE_STATUS_NO_MEM;
              }
            s->cap_line_buffer = len_line_buffer;
          }
      }
  }

  if (s->hw->using_fs)
    {
      s->hw->block_mode = SANE_TRUE;
      status = read_image_info_block (s->hw);
      s->raw.transfer_started = (s->hw->block_total
                                 * s->hw->image_block_size
                                 + s->hw->final_block_size
                                 > 0);
      if (SANE_STATUS_GOOD != status) return status;

      len_raw = s->hw->image_block_size + 1; /* for error code */
    }
  else
    {
      len_raw = s->line_count * s->raw.ctx.bytes_per_line;
    }

  if (resize_warranted (len_raw, s->raw.cap))
    {
      delete (s->raw.buf);
      s->raw.cap = 0;

      if (!(s->raw.buf = t_malloc (len_raw, SANE_Byte)))
        return SANE_STATUS_NO_MEM;

      s->raw.cap = len_raw;
    }
  s->raw.ptr = s->raw.end = s->raw.buf;
  s->raw.transfer_started = true;

  /* This here will block sane_start() until the whole image has been
   * scanned and pre-processed.  The assumption made here is that the
   * pre-processing can not be done in place and that the resulting
   * image is no larger than the image acquired.
   */
  if (dip_needs_whole_image (s->dip, s->val, s->opt))
    {
      SANE_Int len_img = s->raw.ctx.bytes_per_line * s->raw.ctx.lines;
      SANE_Int max = len_img;
      SANE_Int len = 0;

      log_info ("buffering image before returning from sane_start()");

      if (resize_warranted (len_img, s->img.cap))
        {
          delete (s->img.buf);
          s->img.cap = 0;

          if (!(s->img.buf = t_malloc (len_img, SANE_Byte)))
            return SANE_STATUS_NO_MEM;

          s->img.cap = len_img;
        }

      s->img.ptr = s->img.buf;
      do                        /* note: non-blocking I/O not supported */
        {
          s->img.ptr += len;
          max -= len;
          status = fetch_image_data (s, s->img.ptr, max, &len);
        }
      while (SANE_STATUS_GOOD == status);

      if (SANE_STATUS_EOF != status)
        return status;

      if (0 != max)
        return SANE_STATUS_IO_ERROR;

      s->img.ptr = s->img.buf;
      s->img.end = s->img.buf + len_img;
      memcpy (&s->img.ctx, &s->raw.ctx, sizeof (s->raw.ctx));

      if (SANE_OPTION_IS_ACTIVE (s->opt[OPT_DESKEW].cap)
          && s->val[OPT_DESKEW].b)
        {
          dip_deskew (s->dip, s->hw, s->frame_count, &s->img, s->val);
        }

      if (SANE_OPTION_IS_ACTIVE (s->opt[OPT_AUTOCROP].cap)
          && s->val[OPT_AUTOCROP].b)
        {
          dip_autocrop (s->dip, s->hw, s->frame_count, &s->img, s->val);
        }

      s->img.all_data_fetched = true;
      s->img.transfer_started = true;
      s->src = &s->img;
    }

  return SANE_STATUS_GOOD;
}

/*! Retrieves an image info block and computes image data block info.
 */
static SANE_Status
read_image_info_block (device *hw)
{
  SANE_Status status = SANE_STATUS_GOOD;

  u_char buf[14];               /* largest image handshake "info" block */
  int    buf_size = num_of (buf);

  const int limit = 30;
        int ticks =  0;

  log_call ();

  if (!hw->using_fs)
    buf_size = (hw->block_mode ? 6 : 4);

  channel_recv_all_retry (hw->channel, buf, buf_size,
                          MAX_READ_ATTEMPTS, &status);

  /* Prevent reporting of stale values if we bail early.
   */
  hw->image_block_size = 0;
  hw->final_block_size = 0;
  if (hw->using_fs)
    {
      hw->block_total = 0;
      hw->block_count = 0;
    }

  if (SANE_STATUS_GOOD != status)
    return status;

  if (STX != buf[0])
  {
    log_data ("code   %02x", buf[0]);
    log_data ("error, expected STX");

    return SANE_STATUS_INVAL;
  }
  hw->status = buf[1];

  /* Update values here so they are also available in case we bail in
     the while loop below.
   */
  if (hw->using_fs)
    {
      hw->image_block_size = buf_to_uint32 (buf +  2);
      hw->final_block_size = buf_to_uint32 (buf + 10);
      hw->block_total = buf_to_uint32 (buf + 6);
      hw->block_count = 0;
    }
  else
    {
      hw->image_block_size = buf[3] << 8 | buf[2];
      if (hw->block_mode)
        hw->image_block_size *= buf[5] << 8 | buf[4];
    }

  /* Although spec compliant scanners have their warming up bit set
     when they are getting ready for a scan, the world is less than
     perfect :-(

     We hack around non-compliant behaviour by looping until 'ESC G'
     succeeds or our patience runs out.  In the latter case we just
     claim that the device is busy.

     However, when the ADF function is used, and the last paper is
     scanned, we do not need looping any more.
   */
  while (hw->status & (STATUS_FATAL_ERROR | STATUS_NOT_READY)
         && !(using (hw, adf)
              && (hw->status & STATUS_AREA_END))
         && ticks++ < limit)
    {
      u_char cmd[2];

      err_fatal ("fatal error - Status = %02x", hw->status);

      if (SANE_STATUS_GOOD != (status = check_warmup (hw)))
        return status;

      cmd[0] = (hw->using_fs ? FS : ESC);
      cmd[1] = 'G';

      channel_send (hw->channel, cmd, 2, &status);
      sleep (1);
      channel_recv_all_retry (hw->channel, buf, buf_size,
                              MAX_READ_ATTEMPTS, &status);
      hw->status = buf[1];

      if (hw->using_fs)
        {
          hw->image_block_size = buf_to_uint32 (buf +  2);
          hw->final_block_size = buf_to_uint32 (buf + 10);
          hw->block_total = buf_to_uint32 (buf + 6);
          hw->block_count = 0;
        }
      else
        {
          hw->image_block_size = buf[3] << 8 | buf[2];
          if (hw->block_mode)
            hw->image_block_size *= buf[5] << 8 | buf[4];
        }
    }
  return (ticks < limit
	  ? status
	  : SANE_STATUS_DEVICE_BUSY);
}


static void
scan_finish (Epson_Scanner * s)
{
  log_call ();

  s->raw.transfer_stopped = true;

  if (s->hw->channel->interpreter)	/* FIXME: do we really need this?  */
  {
    s->hw->channel->interpreter->free (s->hw->channel);
  }

  s->frame_count++;

  if (!using (s->hw, adf)) return;      /* we're done */

  if (!(EXT_STATUS_ADFT & s->hw->ext_status)    /* sheet feed */
      && !adf_has_auto_form_feed (s->hw))
    {
      dev_eject_paper (s->hw);
    }

  /* avoid blocking here on dev_request_extended_status()
   * with AFF devices, unless canceling
   */
  if (!adf_has_auto_form_feed (s->hw) || s->raw.cancel_requested)
    adf_handle_out_of_paper (s);
}

/* part of the scan sequence logic and should not be used generally */
static void
adf_handle_out_of_paper (Epson_Scanner * s)
{
  if (!using (s->hw, adf)) return;

  /* if we're finishing an ADF scan and the ADF unit has been disabled,
   * enable it again in order to get accurate status information
   */
  if (using (s->hw, adf) && !(ADF_STATUS_EN & s->hw->adf->status))
    {
      /* if it is previewing now, disable duplex */
      byte value = ((s->hw->adf->using_duplex && !s->val[OPT_PREVIEW].b)
                    ? 0x02 : 0x01);
      cmd_control_option_unit (s->hw, value);
    }

  dev_request_extended_status (s->hw);
  if (!(ADF_STATUS_PE & s->hw->adf->status)) return;

  log_info ("ADF: out of paper, %s mode, %d sheets",
            ((ADF_STATUS_PAG & s->hw->adf->status) ? "duplex" : "simplex"),
            s->hw->adf->sheet_count);
  if (!(ADF_STATUS_PAG & s->hw->adf->status)
      || 0 == (s->hw->adf->sheet_count % 2))
    {
      dev_eject_paper (s->hw);
      s->frame_count = 0;
      /* Restore original SANE options.
       * \todo string and word array have to copy individually.
       */
      memcpy (s->val, s->val_bak, (sizeof (s->val[0]) * NUM_OPTIONS));
    }
  else
    log_info ("ADF: scanning reverse side");
}

static void
adf_handle_manual_centering (Epson_Scanner *s, SANE_Bool finalize)
{
  scan_area_t adf_scan_area;
  double shift = 0.0;

  if (!using (s->hw, adf)) return;
  if (!adf_needs_manual_centering (s->hw)) return;

  adf_scan_area =
    get_model_info_max_scan_area (s->hw, s->val[OPT_ADF_MODE].w);

  if(SANE_UNFIX (adf_scan_area.width) < 0
     && SANE_UNFIX (adf_scan_area.height) < 0)
  {
    adf_scan_area.width = s->hw->src->x_range.max;
    adf_scan_area.height = s->hw->src->y_range.max;
  }

  /* scan area setting or no marquee */
  if (!finalize ||
      (adf_scan_area.width     == (s->val[OPT_BR_X].w - s->val[OPT_TL_X].w)
       && adf_scan_area.height == (s->val[OPT_BR_Y].w - s->val[OPT_TL_Y].w)))
  {
    double scan_width = (SANE_UNFIX (s->val[OPT_BR_X].w)
                         - SANE_UNFIX (s->val[OPT_TL_X].w));

    shift = ((SANE_UNFIX (s->hw->src->x_range.max) - scan_width) / 2);

    s->val[OPT_TL_X].w = SANE_FIX (shift + 0);
    s->val[OPT_BR_X].w = SANE_FIX (shift + scan_width);
  }
  /* uses auto-detect document size */
  else if (has_size_check_support (s->hw->src)
           && SANE_OPTION_IS_ACTIVE (s->opt[OPT_DETECT_DOC_SIZE].cap)
           && s->val[OPT_DETECT_DOC_SIZE].w)
  {
    shift = ((SANE_UNFIX (s->hw->src->x_range.max) - s->hw->src->doc_x) / 2);

    s->val[OPT_TL_X].w = SANE_FIX (shift + 0);
    s->val[OPT_BR_X].w = SANE_FIX (shift + s->hw->src->doc_x);
  }
  /* has marquee and adf-duplex scan area differ from adf-simplex scan area */
  else if (!(adf_scan_area.width     == s->hw->src->x_range.max
             && adf_scan_area.height == s->hw->src->y_range.max))
  {
    shift = SANE_UNFIX (s->hw->src->x_range.max - adf_scan_area.width) / 2;

    s->val[OPT_TL_X].w += SANE_FIX (shift);
    s->val[OPT_BR_X].w += SANE_FIX (shift);
  }

  log_info ("shifting scan area offset by %.2f mm", shift);
}

#define GET_COLOR(x)	(((x) >> 2) & 0x03)

SANE_Status
fetch_image_data (Epson_Scanner *s, SANE_Byte * data, SANE_Int max_length,
             SANE_Int * length)
{
  SANE_Status status;
  int index = 0;
  SANE_Bool reorder = SANE_FALSE;
  SANE_Bool needStrangeReorder = SANE_FALSE;

  log_call ();

  if (s->raw.transfer_stopped && s->raw.cancel_requested)
    return SANE_STATUS_CANCELLED;

START_READ:
  if (s->raw.ptr == s->raw.end)
  {
    size_t buf_len;

    if (s->raw.all_data_fetched)
    {
      *length = 0;
      return SANE_STATUS_EOF;
    }

    if (!s->hw->using_fs)
      {
        status = read_image_info_block (s->hw);
        if (SANE_STATUS_GOOD != status)
          {
            *length = 0;
            scan_finish (s);
            return status;
          }
        buf_len = s->hw->image_block_size;
      }
    else
      {
        buf_len = s->hw->image_block_size;
        if (s->hw->block_count >= s->hw->block_total)
          buf_len = s->hw->final_block_size;
        ++buf_len; /* include error byte */
      }

    if (!s->hw->block_mode && SANE_FRAME_RGB == s->raw.ctx.format)
    {
      /* Read color data in line mode */

      /* read the first color line - the number of bytes to read
       * is already known (from last call to read_image_info_block()
       * We determine where to write the line from the color information
       * in the data block. At the end we want the order RGB, but the
       * way the data is delivered does not guarantee this - actually it's
       * most likely that the order is GRB if it's not RGB!
       */
      switch (GET_COLOR (s->hw->status))
      {
      case 1:
	index = 1;
	break;
      case 2:
	index = 0;
	break;
      case 3:
	index = 2;
	break;
      }

      channel_recv (s->hw->channel, s->raw.buf + index * s->raw.ctx.pixels_per_line,
                    buf_len, &status);

      if (SANE_STATUS_GOOD != status)
      {
	*length = 0;
	scan_finish (s);
	return status;
      }

      /* send the ACK signal to the scanner in order to make
       * it ready for the next image info block.
       */
      channel_send (s->hw->channel, S_ACK, 1, &status);

      /* ... and request the next image info block
       */
      if (SANE_STATUS_GOOD != (status = read_image_info_block (s->hw)))
      {
	*length = 0;
	scan_finish (s);
	return status;
      }

      buf_len = s->hw->image_block_size;

      switch (GET_COLOR (s->hw->status))
      {
      case 1:
	index = 1;
	break;
      case 2:
	index = 0;
	break;
      case 3:
	index = 2;
	break;
      }

      channel_recv (s->hw->channel, s->raw.buf + index * s->raw.ctx.pixels_per_line,
                    buf_len, &status);

      if (SANE_STATUS_GOOD != status)
      {
	*length = 0;
	scan_finish (s);
	return status;
      }

      channel_send (s->hw->channel, S_ACK, 1, &status);

      /* ... and the last image info block
       */
      if (SANE_STATUS_GOOD != (status = read_image_info_block (s->hw)))
      {
	*length = 0;
	scan_finish (s);
	return status;
      }

      buf_len = s->hw->image_block_size;

      switch (GET_COLOR (s->hw->status))
      {
      case 1:
	index = 1;
	break;
      case 2:
	index = 0;
	break;
      case 3:
	index = 2;
	break;
      }

      channel_recv (s->hw->channel, s->raw.buf + index * s->raw.ctx.pixels_per_line,
                    buf_len, &status);

      if (SANE_STATUS_GOOD != status)
      {
	*length = 0;
	scan_finish (s);
	return status;
      }
    }
    else
    {
      /* Read image data in block mode */

      /* do we have to reorder the image data ? */
      if (GET_COLOR (s->hw->status) == 0x01)
      {
	reorder = SANE_TRUE;
      }

      channel_recv_all_retry (s->hw->channel, s->raw.buf, buf_len,
                              MAX_READ_ATTEMPTS, &status);

      if (SANE_STATUS_GOOD != status)
      {
	*length = 0;
	scan_finish (s);
	return status;
      }
    }

    if (s->hw->using_fs)
      {
        u_char err = 0;

        if (s->hw->block_count >= s->hw->block_total)
          s->raw.all_data_fetched = true;
        ++(s->hw->block_count);
        log_info ("read image block %u/%u",
                  s->hw->block_count, s->hw->block_total + 1);

        err = s->raw.buf[--buf_len]; /* drop error byte */
        log_info ("image block error byte: %x", err);

        if ((FSG_FATAL_ERROR | FSG_NOT_READY) & err)
          {
	    *length = 0;
	    scan_finish (s);
	    return check_ext_status (s->hw);
          }
        else if (FSG_CANCEL_REQUEST & err)
          {
            s->raw.cancel_requested = true;
          }
        else if (FSG_PAGE_END & err)
          {
            if (FSI_CAP_PED & s->hw->fsi_cap_2)
              {
                log_info ("paper end flag raised");
              }
            else
              {
                err_minor ("invalid paper end flag raised");
              }
          }
        else if (0 != err)
          {
            log_info ("unknown error flag(s) raised");
            *length = 0;
            scan_finish (s);
            return check_ext_status (s->hw);
          }
      }
    else
      {
        s->raw.all_data_fetched = (STATUS_AREA_END & s->hw->status);
      }

    if (s->raw.all_data_fetched
        && ENABLE_TIMING && TIME_PASS_MAX > time_pass_count)
      {
        time_stamp (time_pass[time_pass_count], stop);
        ++time_pass_count;
      }

    if (!s->raw.all_data_fetched)
    {
      if (s->raw.cancel_requested)
      {
	channel_send (s->hw->channel, S_CAN, 1, &status);
	if (SANE_STATUS_GOOD != status) return status;

	status = expect_ack (s->hw);
	if (SANE_STATUS_GOOD != status) return status;

	*length = 0;

	scan_finish (s);

	return SANE_STATUS_CANCELLED;
      }
      else
      {
	channel_send (s->hw->channel, S_ACK, 1, &status);
	if (SANE_STATUS_GOOD != status) return status;
      }
    }

    s->raw.end = s->raw.buf + buf_len;
    s->raw.ptr = s->raw.buf;

    /* if we have to re-order the color components (GRB->RGB) we
     * are doing this here:
     */

    /* Some scanners (e.g. the Perfection 1640 and GT-2200) seem
     * to have the R and G channels swapped.
     * The GT-8700 is the Asian version of the Perfection1640.
     * If the scanner name is one of these, and the scan mode is
     * RGB then swap the colors.
     */

    needStrangeReorder = ((0 == strcmp_c (s->hw->fw_name, "GT-2200")
                           || 0 == strcmp_c (s->hw->fw_name, "Perfection1640")
                           || 0 == strcmp_c (s->hw->fw_name, "GT-8700"))
                          && s->raw.ctx.format == SANE_FRAME_RGB);

    /* Certain Perfection 1650 also need this re-ordering of the two 
     * color channels. These scanners are identified by the problem 
     * with the half vertical scanning area. When we corrected this, 
     * we also set the variable s->hw->need_color_reorder
     */
    if (s->hw->need_color_reorder)
    {
      needStrangeReorder = SANE_TRUE;
    }

    if (needStrangeReorder)
      reorder = SANE_FALSE;	/* reordering once is enough */

    if (s->raw.ctx.format != SANE_FRAME_RGB)
      reorder = SANE_FALSE;	/* don't reorder for BW or gray */

    if (reorder)
    {
      s->raw.ptr = s->raw.buf;
      dip_change_GRB_to_RGB (s->dip, &s->raw);
    }

    /* Do the color_shuffle if everything else is correct - at this
     * time most of the stuff is hardcoded for the Perfection 610
     */
    if (s->hw->color_shuffle)
    {
      int new_length = 0;

      status = color_shuffle (s, &new_length);

      /* If no bytes are returned, check if the scanner is already
       * done, if so, we'll probably just return, but if there is more
       * data to process get the next batch.
       */

      if (new_length == 0 && s->raw.end != s->raw.ptr)
      {
	goto START_READ;
      }

      s->raw.end = s->raw.buf + new_length;
      s->raw.ptr = s->raw.buf;
    }

    if ((SANE_CAP_EMULATED & s->opt[OPT_CCT_1].cap)
        && s->hw->color_user_defined[s->val[OPT_COLOR_CORRECTION].w]
        && SANE_FRAME_RGB == s->raw.ctx.format)
    {
      dip_apply_color_profile (s->dip, &s->raw, s->cct);
    }

    if (s->hw->channel->interpreter)
    {
      s->hw->channel->interpreter->ftor0 (s->hw->channel,
                                         &s->raw.ctx,
                                          s->raw.ptr, s->raw.end);
    }

    if (s->lut)
      {
        dip_apply_LUT (s->dip, &s->raw, s->lut);
      }

    /* WARNING: The SANE specification normally uses zero to indicate
     * minimum intensity.  However, SANE_FRAME_GRAY images with a bit
     * depth of one use zero to indicate *maximum* intensity.
     * The device always uses zero for minimum intensity, irrespective
     * of the color mode and bit depth.
     */
    if (1 == s->raw.ctx.depth && SANE_FRAME_GRAY == s->raw.ctx.format)
      {
        if (!s->invert_image)
          dip_flip_bits (s->dip, &s->raw);
      }
    else
      {
        if (s->invert_image)
          dip_flip_bits (s->dip, &s->raw);
      }
  }

  /* copy the image data to the data memory area
   */
  if (!s->hw->block_mode && SANE_FRAME_RGB == s->raw.ctx.format)
  {
    max_length /= 3;

    if (max_length > s->raw.end - s->raw.ptr)
      max_length = s->raw.end - s->raw.ptr;

    *length = 3 * max_length;

    if (s->raw.cancel_requested)
      s->raw.ptr += max_length;
    else
      {
        while (max_length-- != 0)
          {
            *data++ = s->raw.ptr[0];
            *data++ = s->raw.ptr[s->raw.ctx.pixels_per_line];
            *data++ = s->raw.ptr[2 * s->raw.ctx.pixels_per_line];
            ++s->raw.ptr;
          }
      }
  }
  else
  {
    if (max_length > s->raw.end - s->raw.ptr)
      max_length = s->raw.end - s->raw.ptr;

    *length = max_length;

    if (s->raw.cancel_requested)
      s->raw.ptr += max_length;
    else
      {
        memcpy (data, s->raw.ptr, max_length);
        s->raw.ptr += max_length;
      }
  }

  if (s->raw.ptr == s->raw.end && s->raw.all_data_fetched)
  {
    scan_finish (s);
    if (0 == strcmp_c (s->hw->fw_name, "DS-30"))
    {
      status = check_ext_status (s->hw);
      /* Ignore ADF paper empty */
      s->hw->adf->status &= ~ADF_STATUS_PE;
      if (   SANE_STATUS_GOOD != status
          && SANE_STATUS_NO_DOCS != status)
      {
        return status;
      }
    }
  }
  log_call ("exit");

  return SANE_STATUS_GOOD;
}


/*! Puts raw scan data into the correct scan line.

    When scanning with a non-zero line distance, the RGB data is \e
    not on a single line in the raw scan data.  The RGB channels are
    separated by a number of lines that depends on the line distance
    as reported by the ESC i command and the resolution of the scan.

    This function reorganises raw scan data so that the RGB channels
    are no longer separated and all data is on the same scan line.

    \note
    It seems that the Perfection 610 and 640U both report a maximum
    scan area with the two line distances included (based on 11.7"
    at 600dpi = 7020, whereas the scan area is reportedly 7036 with
    both line distances equal to 8).
 */
static SANE_Status
color_shuffle (Epson_Scanner *s, int *new_length)
{
  SANE_Byte *buf = s->raw.buf;
  int length = s->raw.end - s->raw.buf;

  log_call ();

  if (s->hw->color_shuffle == SANE_TRUE)
  {
    SANE_Byte *data_ptr;	/* ptr to data to process */
    SANE_Byte *data_end;	/* ptr to end of processed data */
    SANE_Byte *out_data_ptr;	/* ptr to memory when writing data */
    int i;			/* loop counter */

    log_call ();

    /* Initialize the variables we are going to use for the 
     * copying of the data. data_ptr is the pointer to
     * the currently worked on scan line. data_end is the
     * end of the data area as calculated from adding *length 
     * to the start of data.
     * out_data_ptr is used when writing out the processed data
     * and always points to the beginning of the next line to
     * write.
     */

    data_ptr = out_data_ptr = buf;
    data_end = data_ptr + length;

    /* The image data is in *raw.buf, we know that the buffer contains
     * s->raw.end - s->raw.buf ( = length) bytes of data. The width of
     * one line is in s->raw.ctx.bytes_per_line
     */

    /* The buffer area is supposed to have a number of full scan
     * lines, let's test if this is the case. 
     */

    if (length % s->raw.ctx.bytes_per_line != 0)
    {
      err_major ("ERROR in size of buffer: %d / %d",
                 length, s->raw.ctx.bytes_per_line);
      return SANE_STATUS_INVAL;
    }

    while (data_ptr < data_end)
    {
      SANE_Byte *source_ptr, *dest_ptr;
      int loop;

      /* copy the green information into the current line */

      source_ptr = data_ptr + 1;
      dest_ptr = s->line_buffer[s->color_shuffle_line] + 1;

      for (i = 0; i < s->raw.ctx.bytes_per_line / 3; i++)
      {
	*dest_ptr = *source_ptr;
	dest_ptr += 3;
	source_ptr += 3;
      }

      /* copy the red information n lines back */

      if (s->color_shuffle_line >= s->line_distance)
      {
	source_ptr = data_ptr + 2;
	dest_ptr =
	  s->line_buffer[s->color_shuffle_line - s->line_distance] + 2;

	for (loop = 0; loop < s->raw.ctx.bytes_per_line / 3; loop++)
	{
	  *dest_ptr = *source_ptr;
	  dest_ptr += 3;
	  source_ptr += 3;
	}
      }

      /* copy the blue information n lines forward */

      source_ptr = data_ptr;
      dest_ptr = s->line_buffer[s->color_shuffle_line + s->line_distance];

      for (loop = 0; loop < s->raw.ctx.bytes_per_line / 3; loop++)
      {
	*dest_ptr = *source_ptr;
	dest_ptr += 3;
	source_ptr += 3;
      }

      data_ptr   += s->raw.ctx.bytes_per_line;
      s->raw.ptr += s->raw.ctx.bytes_per_line;

      if (s->color_shuffle_line == s->line_distance)
      {
        /* We just finished shuffling the line in line_buffer[0] -
	 * write the RGB image data in it to the output buffer and
	 * cyclically shift the line_buffers up by one.
	 */

        SANE_Byte *first;

	if ((s->current_output_line >= s->line_distance) &&
	    (s->current_output_line < s->raw.ctx.lines + s->line_distance))
	{
	  memcpy (out_data_ptr, s->line_buffer[0], s->raw.ctx.bytes_per_line);
	  out_data_ptr += s->raw.ctx.bytes_per_line;
	}

	s->current_output_line++;

        first = s->line_buffer[0];
        for (i = 0; i < 2 * s->line_distance; ++i)
        {
          s->line_buffer[i] = s->line_buffer[i + 1];
        }
        s->line_buffer[2 * s->line_distance] = first;
      }
      else
      {
	s->color_shuffle_line++;	/* increase the buffer number */
      }
    }

    /* At this time we've used up all the new data from the scanner,
     * some of it is still in the line_buffers, but we are ready to
     * return some of it to the front end software. To do so we have
     * to adjust the size of the data area and the *new_length
     * variable.
     */

    *new_length = out_data_ptr - buf;
  }

  return SANE_STATUS_GOOD;
}


static SANE_Status
get_identity_information (device *hw)
{
  SANE_Status status = SANE_STATUS_GOOD;

  log_call ();

  if (!hw->cmd->request_identity)
    return SANE_STATUS_INVAL;

  status = cmd_request_identity (hw);
  if (SANE_STATUS_GOOD != status)
    {
      return SANE_STATUS_INVAL;
    }
  log_data ("detected command level %s", hw->cmd_lvl);

  {
    char *force = getenv ("SANE_EPSON_CMD_LVL");

    if (force)
    {
      hw->cmd_lvl[0] = force[0];
      hw->cmd_lvl[1] = force[1];
      log_info ("forced command level %s", hw->cmd_lvl);
    }
  }

  /* check if option equipment is installed */

  if (hw->status & STATUS_OPTION)
  {
    log_info ("option equipment is installed");
  }

  delete (hw->tpu);             /* FIXME: is this necessary?? */
  delete (hw->adf);

  /* set command type and level */
  {
    int n = 0;

    while (num_of (epson_cmd) > n
           && (0 != strcmp_c (hw->cmd_lvl, epson_cmd[n].level)))
      ++n;

    if (num_of (epson_cmd) > n)
      hw->cmd = &epson_cmd[n];
    else
      err_minor ("unknown command level %s, using %s instead",
                 hw->cmd_lvl, hw->cmd->level);

    hw->level = hw->cmd->level[1] - '0';
  }				/* set comand type and level */

  if (hw->using_fs && 'D' != hw->cmd->level[0]) return SANE_STATUS_GOOD;

  /* Setting available resolutions and xy ranges for sane frontend. */
  {
    hw->dpi_range.min = hw->res.list[1];
    hw->dpi_range.max = hw->res.list[hw->res.size];
    if (!hw->using_fs) hw->base_res = hw->res.list[hw->res.size];
    hw->dpi_range.quant = 0;

    if (hw->fbf)
      {
        update_ranges (hw, hw->fbf);

        log_info ("FBF: TL (%.2f, %.2f) -- BR (%.2f, %.2f) [in mm]",
                  SANE_UNFIX (hw->fbf->x_range.min),
                  SANE_UNFIX (hw->fbf->y_range.min),
                  SANE_UNFIX (hw->fbf->x_range.max),
                  SANE_UNFIX (hw->fbf->y_range.max));
      }
  }

  copy_resolution_info (&hw->res_x, &hw->res, SANE_FALSE);
  copy_resolution_info (&hw->res_y, &hw->res, SANE_FALSE);

  return SANE_STATUS_GOOD;
}				/* request identity */


static SANE_Status
get_hardware_property (device *hw)
{
  SANE_Status status = SANE_STATUS_GOOD;

  log_call ();

  if (hw->cmd->request_identity2 == 0)
    return SANE_STATUS_UNSUPPORTED;

  status = cmd_request_hardware_property (hw);
  if (SANE_STATUS_GOOD != status)
    {
      return status;
    }

  log_info ("optical resolution: %ddpi", hw->optical_res);

  if (hw->line_dist_x != hw->line_dist_y)
  {
    return SANE_STATUS_INVAL;
  }
  hw->max_line_distance = hw->line_dist_y;
  if (hw->fbf)
    {
      hw->fbf->y_range.max = SANE_FIX ((hw->fbf->max_y -
                                        2 * (hw->max_line_distance))
                                       * MM_PER_INCH / hw->base_res);
    }

  return SANE_STATUS_GOOD;
}

static SANE_Status
get_identity2_information (device *hw, Epson_Scanner *s)
{
  SANE_Status status = SANE_STATUS_GOOD;

  require (s && (hw == s->hw));

  status = get_hardware_property (hw);
  if (SANE_STATUS_GOOD != status)
    return status;

  /* reset constraints because we are pointing to different lists now
     FIXME: Only needs to be done the first time we (successfully)
     send the ESC i command.  This should be done when constructing
     the device and is therefore done by the time we construct a
     scanner.  While the content and size of the lists may vary
     depending on the selected option, the list nature is constant.
   */
  s->opt[OPT_X_RESOLUTION].constraint_type      = SANE_CONSTRAINT_WORD_LIST;
  s->opt[OPT_X_RESOLUTION].constraint.word_list = hw->res_x.list;
  s->opt[OPT_Y_RESOLUTION].constraint_type      = SANE_CONSTRAINT_WORD_LIST;
  s->opt[OPT_Y_RESOLUTION].constraint.word_list = hw->res_y.list;

  return SANE_STATUS_GOOD;
}


void
sane_cancel (SANE_Handle handle)
{
  Epson_Scanner *s = (Epson_Scanner *) handle;

  log_call ();

  s->img.cancel_requested = true;

  if (s->src->transfer_started && !s->src->transfer_stopped)
  {
    SANE_Byte dumpster[1024];
    int len;

    if (s->src == &s->raw)
    {
      s->raw.cancel_requested = true;
      do
        {
          fetch_image_data (s, dumpster, num_of (dumpster), &len);
        }
      while (!s->raw.transfer_stopped);
    }

    if (using (s->hw, adf) && 0 != s->hw->adf->sheet_count
        && ((EXT_STATUS_ADFT & s->hw->ext_status) /* page type */
            || adf_has_auto_form_feed (s->hw)))
      dev_eject_paper (s->hw);
  }
  if (!s->raw.cancel_requested && s->raw.all_data_fetched
      && s->hw->using_fs && s->val[OPT_ADF_AUTO_SCAN].w
      && SANE_STATUS_NO_DOCS != check_ext_status (s->hw))
  {
    s->raw.cancel_requested = dev_force_cancel (s->hw);
  }
  dev_unlock (s->hw);

  if (ENABLE_TIMING)
    {
      time_stamp (time_scan, stop);
      time_stats (time_pass_count);
      time_pass_count = 0;
    }

  s->frame_count = 0;

  /* Restore original SANE options.
   * \todo string and word array have to copy individually.
   */
  if (s->src->transfer_started)
    {
      memcpy (s->val, s->val_bak, (sizeof (s->val[0]) * NUM_OPTIONS));
    }

  /* release resource hogs between scan sequences */
  delete (s->img.buf);
  s->img.cap = 0;
}

/* Request the push button status returns SANE_TRUE if the button was
 * pressed and SANE_FALSE if the button was not pressed.  It also
 * returns SANE_TRUE in case of an error.  This is necessary so that a
 * process that waits for the button does not block indefinitely.
 */
static SANE_Status
get_push_button_status (device *hw, SANE_Bool *button_pushed)
{
  SANE_Status status;
  int len;
  u_char param[3];
  u_char result[4];
  u_char *buf;

  log_call ();

  if (hw->cmd->request_push_button_status == 0)
  {
    log_info ("push button status unsupported");
    return SANE_STATUS_UNSUPPORTED;
  }

  param[0] = ESC;
  param[1] = hw->cmd->request_push_button_status;
  param[2] = '\0';

  channel_send (hw->channel, param, 2, &status);

  if (SANE_STATUS_GOOD != status)
  {
    err_minor ("error sending command");
    return status;
  }

  len = 4;			/* get info block */
  channel_recv (hw->channel, result, len, &status);
  if (SANE_STATUS_GOOD != status)
    return status;

  /* get button status */
  hw->status = result[1];
  len = result[3] << 8 | result[2];
  buf = t_alloca (len, u_char);
  channel_recv (hw->channel, buf, len, &status);

  log_info ("Push button status = %d", buf[0] & 0x01);
  *button_pushed = ((buf[0] & 0x01) != 0);

  return (SANE_STATUS_GOOD);
}


static void
filter_resolution_list (Epson_Scanner * s)
{
  log_call ();

  if (SANE_TRUE == s->val[OPT_LIMIT_RESOLUTION].w)
  {				/* trim list */
    SANE_Int i;

    s->hw->res.size = 0;
    for (i = 1; i <= s->hw->resolution.size; i++)
    {
      SANE_Word res = s->hw->resolution.list[i];

      if ((res < 100) || (0 == (res % 300)) || (0 == (res % 400)))
      {
	s->hw->res.size++;
	s->hw->res.list[s->hw->res.size] = res;
      }
    }
  }
  else
  {				/* restore full list */
    SANE_Int i;

    for (i = 1; i <= s->hw->resolution.size; i++)
    {
      s->hw->res.list[i] = s->hw->resolution.list[i];
    }
    s->hw->res.size = s->hw->resolution.size;
  }
  s->hw->res.list[0] = s->hw->res.size;

  /*  We have just created a filtered resolution list on the *full*
   *  list of resolutions.  Now apply ADF capping if necessary.  No
   *  need to call dev_restore_res as we've already (sorta) done so
   *  above.
   */
  if (using (s->hw, adf)
      && 0 < large_res_kills_adf_scan (s->hw))
  {
    dev_limit_res (s->hw, s->opt[OPT_RESOLUTION].constraint_type,
		   large_res_kills_adf_scan (s->hw));
  }
  handle_resolution (s, OPT_RESOLUTION, s->val[OPT_RESOLUTION].w);
}


SANE_Status
sane_set_io_mode (SANE_Handle handle, SANE_Bool non_blocking)
{
  log_call ("(%s-blocking)", (non_blocking ? "non" : ""));

  /* get rid of compiler warning */
  handle = handle;

  return SANE_STATUS_UNSUPPORTED;
}


SANE_Status
sane_get_select_fd (SANE_Handle handle, SANE_Int * fd)
{
  log_call ();

  /* get rid of compiler warnings */
  handle = handle;
  fd = fd;

  return SANE_STATUS_UNSUPPORTED;
}


SANE_Status
control_option_unit (device *hw, SANE_Bool use_duplex)
{
  SANE_Status status = SANE_STATUS_GOOD;
  u_char      value  = 0;

  log_call ();

  if (!hw)	             return SANE_STATUS_INVAL;
  if (!(hw->adf || hw->tpu)) return SANE_STATUS_GOOD;

  value = using (hw, adf) || using (hw, tpu);
  if (value && use_duplex)
    {
      value = 2;
      hw->adf->using_duplex = SANE_TRUE;
    }

  status = set_cmd (hw, hw->cmd->control_an_extension, value);

  if (using (hw, adf)
      && (0 == strcmp_c ("ES-10000G", hw->fw_name)
	  || 0 == strcmp_c ("Expression10000", hw->fw_name)))
    {
      u_char *buf;
      u_char params[2];
      EpsonHdr head;
      int failures_allowed = 5;

      params[0] = ESC;
      params[1] = hw->cmd->request_extended_status;

      head = (EpsonHdr) command (hw, params, 2, &status);
      buf = &head->buf[0];

      while (!(buf[1] & ADF_STATUS_EN))
	{
	  sleep (1);
	  status = set_cmd (hw, hw->cmd->control_an_extension, value);

	  head = (EpsonHdr) command (hw, params, 2, &status);
	  if (SANE_STATUS_GOOD != status)
	    {
	      --failures_allowed;
	      if (!failures_allowed)
		return status;
	    }
	  buf = &head->buf[0];
	}
    }
  return status;
}

/* Fetches the main/sub resolution lists for the device.
 */
static SANE_Status
get_resolution_constraints (device *hw, Epson_Scanner *s)
{
  SANE_Status status      = SANE_STATUS_GOOD;
  SANE_Bool adf_duplex;

  log_call ();

  if (!hw->cmd->request_identity2)
    return SANE_STATUS_GOOD;

  /* if it is previewing now, disable duplex */
  adf_duplex = ((1 == s->val[OPT_ADF_MODE].w)
                && !s->val[OPT_PREVIEW].b);
  status = control_option_unit (hw, adf_duplex);
  status = get_identity2_information (hw, s);

  handle_resolution (s, OPT_X_RESOLUTION, DEFAULT_X_RESOLUTION);
  handle_resolution (s, OPT_Y_RESOLUTION, DEFAULT_Y_RESOLUTION);

  return status;
}


static scan_area_t get_model_info_max_scan_area (device *hw, int adf_mode)
{
  SANE_Status status      = SANE_STATUS_GOOD;
  scan_area_t scan_area;

  scan_area.width = SANE_FIX(-1);
  scan_area.height = SANE_FIX(-1);

  if(hw->adf){
    const void *info = model_info_cache_get_info (hw->fw_name, &status);
    if (SANE_STATUS_GOOD == status && info){
      const char *mode = (1 == adf_mode ? "duplex": "simplex");

      scan_area = model_info_max_scan_area(info, "adf", mode);
    }
  }
  return scan_area;
}


static SANE_Status handle_scan_area(Epson_Scanner *s, int adf_mode)
{
  SANE_Status status = SANE_STATUS_GOOD;
  scan_area_t scan_area;

  if(s->hw->adf){
    scan_area = get_model_info_max_scan_area (s->hw, adf_mode);

    if(SANE_UNFIX (scan_area.width) >= 0
       && SANE_UNFIX (scan_area.height) >= 0){
      s->hw->adf->x_range.max = scan_area.width;
      s->hw->adf->y_range.max = scan_area.height;
    }else{
      err_minor ("failure getting model info (%s)", sane_strstatus (status));
    }
  }

  {
    SANE_Option_Descriptor *sopt = &(s->opt[OPT_SCAN_AREA]);
    Option_Value           *sval = &(s->val[OPT_SCAN_AREA]);

    SANE_String_Const  area = NULL;
    SANE_String_Const *p
      = (SANE_String_Const *) sopt->constraint.string_list;

    if (!p)
      {
        s->opt[OPT_SCAN_AREA].cap |=  SANE_CAP_INACTIVE;

        /* media_list + media_maximum, media_automatic, and NULL terminator */
        p = t_calloc (num_of (media_list) + 3, SANE_String_Const);

        if (!p)                 /* do without the option */
        {
          s->opt[OPT_QUICK_FORMAT].cap = s->opt[OPT_SCAN_AREA].cap;
          return status;
        }

        sopt->constraint.string_list = p;
      }
    area = p[sval->w];

    memset (p, 0, (num_of (media_list) + 3) * sizeof (SANE_String_Const));
    sopt->size = 0;

    *p = media_maximum;
    sopt->size = max (strlen (*p), sopt->size);
    ++p;

    if (using (s->hw, tpu))
      {
        s->opt[OPT_SCAN_AREA].cap |=  SANE_CAP_INACTIVE;
      }
    else
      {
        size_t i;

        if (has_size_check_support (s->hw->src))
          {
            *p = media_automatic;
            sopt->size = max (strlen (*p) + 1, sopt->size);
            ++p;
            s->opt[OPT_SCAN_AREA].cap |= SANE_CAP_AUTOMATIC;
          }

        for (i = 0; i < num_of (media_list); ++i)
          {
            SANE_Fixed w = SANE_FIX (media_list[i].width);
            SANE_Fixed h = SANE_FIX (media_list[i].height);

            if (   w <= s->hw->src->x_range.max
                && h <= s->hw->src->y_range.max)
              {
                *p = media_list[i].name;
                sopt->size = max (strlen (*p) + 1, sopt->size);
                ++p;
                if (strstr (media_list[i].name, "Portrait"))
                  {
                    ++i;
                  }
              }
            else if (strstr (media_list[i].name, "Landscape"))
              {                 /* no need to list "Portrait" */
                ++i;
              }
          }

        s->opt[OPT_SCAN_AREA].cap &= ~SANE_CAP_INACTIVE;
      }

    {              // recompute index of selected area in the new list
      size_t i = 0;
      p = (SANE_String_Const *) sopt->constraint.string_list;
      while (*p && 0 != strcmp_c (*p, area))
        ++i, ++p;
      sval->w = (*p ? i : 0);
    }
  }

  s->val[OPT_BR_X].w = s->hw->src->x_range.max;
  s->val[OPT_BR_Y].w = s->hw->src->y_range.max;
  s->val[OPT_TL_X].w = 0;
  s->val[OPT_TL_Y].w = 0;

  s->opt[OPT_TL_X].constraint.range = &(s->hw->src->x_range);
  s->opt[OPT_TL_Y].constraint.range = &(s->hw->src->y_range);
  s->opt[OPT_BR_X].constraint.range = &(s->hw->src->x_range);
  s->opt[OPT_BR_Y].constraint.range = &(s->hw->src->y_range);

  /* copy OPT_SCAN_AREA to OPT_QUICK_FORMAT */
  s->opt[OPT_QUICK_FORMAT].size = s->opt[OPT_SCAN_AREA].size;
  s->opt[OPT_QUICK_FORMAT].cap = s->opt[OPT_SCAN_AREA].cap;
  s->opt[OPT_QUICK_FORMAT].constraint.string_list
    = s->opt[OPT_SCAN_AREA].constraint.string_list;
  s->val[OPT_QUICK_FORMAT].w = s->val[OPT_SCAN_AREA].w;

  return status;
}
