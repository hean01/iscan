/* epkowa.h - SANE backend for EPSON flatbed scanners
   	      (Image Scan! version)

   Based on the SANE Epson backend (originally from sane-1.0.3)
   - updated to sane-backends-1.0.6
   - renamed from epson to epkowa to avoid confusion
   - updated to sane-backends-1.0.10
   - updated to sane-backends-1.0.15

   Based on Kazuhiro Sasayama previous
   Work on epson.[ch] file from the SANE package.

   Original code taken from sane-0.71
   Copyright (C) 1997 Hypercore Software Design, Ltd.

   modifications
   Copyright (C) 1998-1999 Christian Bucher <bucher@vernetzt.at>
   Copyright (C) 1998-1999 Kling & Hautzinger GmbH
   Copyright (C) 1999 Norihiko Sawa <sawa@yb3.so-net.ne.jp>
   Copyright (C) 2000 Karl Heinz Kremer <khk@khk.net>
   Copyright (C) 2001-2009 SEIKO EPSON CORPORATION

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
   If you do not wish that, delete this exception notice.  */

#ifndef epkowa_h
#define epkowa_h

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "device.h"

typedef struct
{
  SANE_Byte *lut;
  int depth;

} LUT;


/* convenience union to access option values given to the backend
 */
typedef union
{
  SANE_Bool b;
  SANE_Word w;
  SANE_Word *wa;		/* word array */
  SANE_String s;
}
Option_Value;

/* string constants for GUI elements that are not defined SANE-wide */

#define SANE_NAME_GAMMA_CORRECTION "gamma-correction"
#define SANE_TITLE_GAMMA_CORRECTION SANE_I18N("Gamma Correction")
#define SANE_DESC_GAMMA_CORRECTION SANE_I18N("Selects the gamma correction value from a list of pre-defined devices or the user defined table, which can be downloaded to the scanner")

#define SANE_EPSON_FOCUS_NAME "focus-position"
#define SANE_EPSON_FOCUS_TITLE SANE_I18N("Focus Position")
#define SANE_EPSON_FOCUS_DESC SANE_I18N("Sets the focus position to either the glass or 2.5mm above the glass")
#define SANE_EPSON_WAIT_FOR_BUTTON_NAME "wait-for-button"
#define SANE_EPSON_WAIT_FOR_BUTTON_TITLE SANE_I18N("Wait for Button")
#define SANE_EPSON_WAIT_FOR_BUTTON_DESC SANE_I18N("After sending the scan command, wait until the button on the scanner is pressed to actually start the scan process.");
#define SANE_EPSON_MONITOR_BUTTON_NAME "monitor-button"
#define SANE_EPSON_MONITOR_BUTTON_TITLE SANE_I18N("Monitor Button")
#define SANE_EPSON_MONITOR_BUTTON_DESC SANE_I18N("Indicates whether a button on the scanner has been pressed.");
#define SANE_EPSON_DETECT_DOC_SIZE_NAME "detect-doc-size"
#define SANE_EPSON_DETECT_DOC_SIZE_TITLE SANE_I18N("Auto-detect document size")
#define SANE_EPSON_DETECT_DOC_SIZE_DESC SANE_I18N("Activates document size auto-detection.  The scan area will be set to match the detected document size.")
#define SANE_EPSON_SCAN_AREA_IS_VALID_NAME "scan-area-is-valid"
#define SANE_EPSON_SCAN_AREA_IS_VALID_TITLE SANE_I18N("Scan Area Is Valid")
#define SANE_EPSON_SCAN_AREA_IS_VALID_DESC SANE_I18N("Indicates whether the current scan area settings are valid.")

#define SANE_EPSON_ADF_AUTO_SCAN_NAME  "adf-auto-scan"
#define SANE_EPSON_ADF_AUTO_SCAN_TITLE SANE_I18N("ADF Auto Scan")
#define SANE_EPSON_ADF_AUTO_SCAN_DESC  SANE_I18N("Skips per sheet device setup for faster throughput.")

#define SANE_EPSON_ADF_DFD_SENSITIVITY_NAME  "double-feed-detection-sensitivity"
#define SANE_EPSON_ADF_DFD_SENSITIVITY_TITLE SANE_I18N("Double Feed Detection Sensitivity")
#define SANE_EPSON_ADF_DFD_SENSITIVITY_DESC  SANE_I18N("Sets the sensitivity with which multi-sheet page feeds are detected and reported as errors.")

#define SANE_EPSON_ADF_DUPLEX_DIRECTION_MATCHES_NAME "adf-duplex-direction-matches"
#define SANE_EPSON_ADF_DUPLEX_DIRECTION_MATCHES_TITLE SANE_I18N("ADF Duplex Direction Matches")
#define SANE_EPSON_ADF_DUPLEX_DIRECTION_MATCHES_DESC SANE_I18N("Indicates whether the device's ADF duplex mode, if available, scans in the same direction for the front and back.")

#define SANE_EPSON_POLLING_TIME_NAME "polling-time"
#define SANE_EPSON_POLLING_TIME_TITLE SANE_I18N("Polling Time")
#define SANE_EPSON_POLLING_TIME_DESC SANE_I18N("Time between queries when waiting for device state changes.")

#define SANE_EPSON_NEEDS_POLLING_NAME "needs-polling"
#define SANE_EPSON_NEEDS_POLLING_TITLE SANE_I18N("Needs Polling")
#define SANE_EPSON_NEEDS_POLLING_DESC SANE_I18N("Indicates whether the scanner needs to poll.")

#define SANE_EPSON_CALIBRATE_NAME "calibrate"
#define SANE_EPSON_CALIBRATE_TITLE SANE_I18N("Calibrate")
#define SANE_EPSON_CALIBRATE_DESC SANE_I18N("Performs color matching to make sure that the document's color tones are scanned correctly.")

#define SANE_EPSON_CLEAN_NAME "clean"
#define SANE_EPSON_CLEAN_TITLE SANE_I18N("Clean")
#define SANE_EPSON_CLEAN_DESC SANE_I18N("Cleans the scanners reading section.")

#define LINES_SHUFFLE_MAX	(17)	/* 2 x 8 lines plus 1 */

#define SANE_EPSON_MAX_RETRIES	(120)	/* how often do we retry during warmup ? */

#define MAX_READ_ATTEMPTS       10      /* maximum number of attempts at
                                           reading scan data */
#define DEFAULT_POLLING_TIME	(1000 * 1000)	/* usec */

enum {
  OPT_NUM_OPTS = 0,
  /*  */
  OPT_MODE_GROUP,
  OPT_MODE,
  OPT_BIT_DEPTH,
  OPT_HALFTONE,
  OPT_DROPOUT,
  OPT_BRIGHTNESS_METHOD,
  OPT_BRIGHTNESS,
  OPT_CONTRAST,
  OPT_SHARPNESS,
  OPT_GAMMA_CORRECTION,
  OPT_COLOR_CORRECTION,
  OPT_RESOLUTION,
  OPT_X_RESOLUTION,
  OPT_Y_RESOLUTION,
  OPT_THRESHOLD,
  /*  */
  OPT_ADVANCED_GROUP,
  OPT_MIRROR,
  OPT_SPEED,
  OPT_AAS,
  OPT_LIMIT_RESOLUTION,
  OPT_ZOOM,
  OPT_GAMMA_VECTOR_R,
  OPT_GAMMA_VECTOR_G,
  OPT_GAMMA_VECTOR_B,
  OPT_WAIT_FOR_BUTTON,
  OPT_MONITOR_BUTTON,
  OPT_POLLING_TIME,
  OPT_NEEDS_POLLING,
  /*  */
  OPT_CCT_GROUP,
  OPT_CCT_1,
  OPT_CCT_2,
  OPT_CCT_3,
  OPT_CCT_4,
  OPT_CCT_5,
  OPT_CCT_6,
  OPT_CCT_7,
  OPT_CCT_8,
  OPT_CCT_9,
  /*  */
  OPT_PREVIEW_GROUP,
  OPT_PREVIEW,
  OPT_PREVIEW_SPEED,
  /*  */
  OPT_GEOMETRY_GROUP,
  OPT_SCAN_AREA,
  OPT_TL_X,
  OPT_TL_Y,
  OPT_BR_X,
  OPT_BR_Y,
  OPT_QUICK_FORMAT,
  /*  */
  OPT_EQU_GROUP,
  OPT_SOURCE,
  OPT_AUTO_EJECT,
  OPT_FILM_TYPE,
  OPT_FOCUS,
  OPT_BAY,
  OPT_EJECT,
  OPT_ADF_MODE,
  OPT_DETECT_DOC_SIZE,
  OPT_SCAN_AREA_IS_VALID,
  OPT_ADF_AUTO_SCAN,
  OPT_ADF_DFD_SENSITIVITY,
  OPT_EXT_SANE_STATUS,
  OPT_ADF_DUPLEX_DIRECTION_MATCHES,
  OPT_DESKEW,
  OPT_AUTOCROP,
  OPT_CALIBRATE,
  OPT_CLEAN,
  /*  */
  NUM_OPTIONS
};

/*! Container for image data.
 */
typedef struct
{
  size_t cap;                   /*!< buffer capacity */
  SANE_Byte *buf;               /*!< start of image data */
  SANE_Byte *end;               /*!< end of image data */
  SANE_Byte *ptr;               /*!< current position */
  SANE_Parameters ctx;          /*!< buffer context */

  bool cancel_requested;
  bool all_data_fetched;
  bool transfer_started;
  bool transfer_stopped;

} buffer;

/*! Software representation of the \e logical device.
 */
struct Epson_Scanner
{
  struct Epson_Scanner *next;

  const void *dip;
  device *hw;
  SANE_Option_Descriptor opt[NUM_OPTIONS];
  Option_Value val[NUM_OPTIONS];
  Option_Value val_bak[NUM_OPTIONS];

  /* image data acquisition buffers and parameters */
  buffer *src;                  /*!< buffer to provide data to frontend */
  buffer  raw;                  /*!< device image data blocks */
  buffer  img;                  /*!< complete in-memory image */

  SANE_Byte *line_buffer[LINES_SHUFFLE_MAX];
  size_t     cap_line_buffer;

  SANE_Int color_shuffle_line;	/* current line number for color shuffling */
  SANE_Int line_distance;	/* current line distance */
  SANE_Int current_output_line;	/* line counter when color shuffling */

  SANE_Bool invert_image;
  SANE_Word gamma_table[3][256];
  double    cct[9];
  LUT      *lut;
  double    brightness;
  double    contrast;

  /*! Number of frames acquired so far.
   *  This includes partial frames if scan_finish() is called before a
   *  frame is completed.
   */
  unsigned int frame_count;

  /*! Number of raw scan lines data gotten from scanner
   *  This corresponds to the \c ESC_d parameter.
   */
  unsigned int line_count;
};

typedef struct Epson_Scanner Epson_Scanner;

SANE_Status estimate_parameters (Epson_Scanner *, SANE_Parameters *);
SANE_Status fetch_image_data (Epson_Scanner *, SANE_Byte *, SANE_Int,
                              SANE_Int *);

#endif /* not epkowa_h */
