/* pisa_enums.h
   Copyright (C) 2001, 2004, 2008, 2009 SEIKO EPSON CORPORATION

   Date         Author      Reason
   06/01/2001   N.Sasaki    New

   This file is part of the `iscan' program.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   As a special exception, the copyright holders give permission
   to link the code of this program with the esmod library and
   distribute linked combinations including the two.  You must obey
   the GNU General Public License in all respects for all of the
   code used other then esmod.
*/

#ifndef ___PISA_ENUMS_H
#define ___PISA_ENUMS_H

/*! Bit flags indicating what scan sources are supported by the device.
 */
enum
{
  PISA_OP_NONE          = 0,
  PISA_OP_FLATBED       = 1 << 0,
  PISA_OP_ADF           = 1 << 1,
  PISA_OP_ADFDPLX       = 1 << 2,
  PISA_OP_TPU           = 1 << 3,
};

enum
{
  PISA_FT_POSI,
  PISA_FT_NEGA,
  PISA_FT_REFLECT,
};

enum
{
  PISA_DFD_OFF,
  PISA_DFD_STD,
  PISA_DFD_THIN,
};

// destination
typedef enum
{
  PISA_DE_FILE,
  PISA_DE_PRINTER,
  PISA_DE_GIMP
} pisa_dest_type;

// pixel type
typedef enum
{
  PISA_PT_BW,
  PISA_PT_GRAY,
  PISA_PT_RGB
} pisa_pixel_type;

// bit depth
typedef enum
{
  PISA_BD_1	= 1 << 0,
  PISA_BD_8	= 1 << 1,
  PISA_BD_12	= 1 << 2,
  PISA_BD_14	= 1 << 3,
  PISA_BD_16	= 1 << 4
} pisa_bitdepth_type;

// de-screening
typedef enum
{
  PISA_DESCREEN_ON,
  PISA_DESCREEN_OFF
} pisa_descreening_type;

// auto exposure option
typedef enum
{
  PISA_AE_PHOTO,
  PISA_AE_DOC,
  PISA_AE_GRAYED
} pisa_aeoption_type;

// dropout
typedef enum
{
  PISA_DO_NONE,
  PISA_DO_RED,
  PISA_DO_GREEN,
  PISA_DO_BLUE
} pisa_dropout_type;

// mono option
typedef enum
{
  PISA_MO_NONE,
  PISA_MO_TET,
  PISA_MO_AAS
} pisa_monooption_type;

// halftone
typedef enum
{
  PISA_HT_NONE,
  PISA_HT_TONEA,
  PISA_HT_TONEB,
  PISA_HT_TONEC,
  PISA_HT_DITHERA,
  PISA_HT_DITHERB,
  PISA_HT_DITHERC,
  PISA_HT_DITHERD,
  PISA_HT_USERA,
  PISA_HT_USERB
} pisa_halftone_type;

// unit
typedef enum
{
  PISA_UNIT_INCHES,
  PISA_UNIT_PIXELS,
  PISA_UNIT_CM
} pisa_unit_type;

// cursor
typedef enum
{
  PISA_CS_ARROW,
  PISA_CS_HAND,
  PISA_CS_CROSS,
  PISA_CS_TOP,
  PISA_CS_BOTTOM,
  PISA_CS_LEFT,
  PISA_CS_RIGHT,
  PISA_CS_LEFTTOP,
  PISA_CS_LEFTBOTTOM,
  PISA_CS_RIGHTTOP,
  PISA_CS_RIGHTBOTTOM
} pisa_cursor_type;

// cursor gamma table
typedef enum {
  PISA_CS_GM_X,
  PISA_CS_GM_FLEUR,
  PISA_CS_GM_TCROSS,
} pisa_cursor_gm_type;

// resize
typedef enum
{
  PISA_RS_NN=1,	// nearest neighbor
  PISA_RS_BL,	// bi linear
  PISA_RS_BC,	// bi cubic
} pisa_rs_type;

// sharp
typedef enum
{
  PISA_SH_UMASK = 1,	// unsharp mask
  PISA_SH_GAUSS,	// gauss
  PISA_SH_UMASKY,	// unsahrp mask y
} pisa_sh_type;

// window 
typedef enum
{
  ID_WINDOW_MAIN,
  ID_WINDOW_PREV,
  ID_WINDOW_CONFIG,
} pisa_window_id;

// main window
enum widget_main_window
{
  WG_MAIN_TOP,
  WG_MAIN_IMG_MENU,
  WG_MAIN_RES_MENU,
  WG_MAIN_WIDTH,
  WG_MAIN_HEIGHT,
  WG_MAIN_FOCUS_0,
  WG_MAIN_FOCUS_25,
  WG_MAIN_DFD,
  WG_MAIN_DFD_BOX,
  WG_MAIN_DFD_STD,
  WG_MAIN_DFD_THIN,
  WG_MAIN_NUM
};

// gamma correction
#define GAMMA_RGB	0x00
#define GAMMA_RED	0x01
#define GAMMA_GRN	0x02
#define GAMMA_BLU	0x03

enum widget_gamma_window
{
  WG_GAMMA_TAB_RGB,
  WG_GAMMA_TAB_RED,
  WG_GAMMA_TAB_GRN,
  WG_GAMMA_TAB_BLU,
  WG_GAMMA_BOX_RGB,
  WG_GAMMA_BOX_RED,
  WG_GAMMA_BOX_GRN,
  WG_GAMMA_BOX_BLU,
  WG_GAMMA_RESET,
  WG_GAMMA_NOTE,
  WG_GAMMA_NUM
};

// configuration
enum widget_configuration
{
  WG_CONFIG_TOP,
  WG_CONFIG_PATH,
  WG_CONFIG_NUM
};

// file select
enum widget_file_select
{
  WG_FS_TOP,
  WG_FS_NUM
};

// print window
enum widget_print_window
{
  WG_PRINT_TOP,
  WG_PRINT_NUM
};

#endif // ___PISA_ENUMS_H


