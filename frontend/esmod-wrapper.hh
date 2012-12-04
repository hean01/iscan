//  esmod-wrapper.hh -- 
//  Copyright (C) 2008  SEIKO EPSON CORPORATION
//
//  This file is part of the 'iscan' program.
//
//  The 'iscan' program is free-ish software.
//  You can redistribute it and/or modify it under the terms of the GNU
//  General Public License as published by the Free Software Foundation;
//  either version 2 of the License or at your option any later version.
//
//  This program is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY;  without even the implied warranty of FITNESS
//  FOR A PARTICULAR PURPOSE or MERCHANTABILITY.
//  See the GNU General Public License for more details.
//
//  You should have received a verbatim copy of the GNU General Public
//  License along with this program; if not, write to:
//
//      Free Software Foundation, Inc.
//      59 Temple Place, Suite 330
//      Boston, MA  02111-1307  USA
//

//  As a special exception, the copyright holders give permission
//  to link the code of this program with the esmod library and
//  distribute linked combinations including the two.  You must obey
//  the GNU General Public License in all respects for all of the
//  code used other then esmod.

#ifndef esmod_wrapper_hh_included
#define esmod_wrapper_hh_included

#ifndef __cplusplus
#error "This is a C++ header file; use a C++ compiler to compile it."
#endif

#include "esmod.hh"

#define ISCAN_DEFAULT_GAMMA     ESMOD_DEFAULT_GAMMA
#define ISCAN_DEFAULT_HILITE    ESMOD_DEFAULT_HILITE
#define ISCAN_DEFAULT_SHADOW    ESMOD_DEFAULT_SHADOW
#define ISCAN_DEFAULT_THRESHOLD ESMOD_DEFAULT_THRESHOLD

#include "pisa_enums.h"
#include "pisa_structs.h"
#include "pisa_esmod_structs.h"
#include "pisa_marquee.h"
#include "pisa_settings.h"

namespace iscan
{

  class focus : public esmod::focus
  {
  public:
    focus (const pisa_image_info& parms);
    focus (struct sharp_img_info parms);
  };

  class moire : public esmod::moire
  {
  public:
    moire (struct moire_img_info parms, bool is_dumb);
  };

  class scale : public esmod::scale
  {
  public:
    scale (struct resize_img_info parms);
  };

  // WARNING: These quite likely modify global state in libesmod.
  void auto_expose (int, int, const pisa_image_info&, const _rectL&,
                    marquee&, bool, bool is_dumb);
  void build_LUT (int, int, const settings&, marquee&, bool is_dumb);

  esmod::type_type esmod_film_type (int iscan_film_type);
  esmod::type_type esmod_focus_type (int iscan_focus_type);
  esmod::type_type esmod_image_type (int iscan_image_type);
  esmod::type_type esmod_option_type (int iscan_option_type);
  esmod::type_type esmod_pixel_type (int iscan_pixel_type);
  esmod::type_type esmod_scale_type (int iscan_scale_type);

} // namespace iscan

inline
iscan::focus::focus (const pisa_image_info& info)
  : esmod::focus::focus (info.m_width, info.m_height, info.m_rowbytes,
                         info.m_bits_per_pixel)
{
}

inline
iscan::focus::focus (const struct sharp_img_info info)
  : esmod::focus::focus (info.in_width,  info.in_height,  info.in_rowbytes,
                         info.out_width, info.out_height, info.out_rowbytes,
                         info.bits_per_pixel,
                         info.strength, info.radius, info.clipping,
                         esmod_focus_type (info.sharp_flag))
{
}

inline
iscan::moire::moire (const struct moire_img_info info, bool is_dumb)
  : esmod::moire::moire (info.in_width,  info.in_height,  info.in_rowbytes,
                         info.out_width, info.out_height, info.out_rowbytes,
                         info.bits_per_pixel,
                         info.resolution, is_dumb)
{
}

inline
iscan::scale::scale (const struct resize_img_info info)
  : esmod::scale::scale (info.in_width,  info.in_height,  info.in_rowbytes,
                         info.out_width, info.out_height, info.out_rowbytes,
                         info.bits_per_pixel,
                         esmod_scale_type (info.resize_flag))
{
}

inline void
iscan::auto_expose (int option_type, int film_type,
                    const pisa_image_info& info, const _rectL& r,
                    marquee& m, bool is_doc, bool is_dumb)
{
  esmod::auto_expose (esmod_option_type (option_type),
                      esmod_film_type (film_type), info.m_img,
                      info.m_width, info.m_height, info.m_rowbytes,
                      r.top, r.left, r.bottom, r.right,
                      &m.gamma, &m.highlight, &m.shadow, &m.graybalance,
                      m.film_gamma, m.film_yp, m.grayl,
                      is_doc, is_dumb);
}

inline void
iscan::build_LUT (int option_type, int film_type, const settings& s,
                  marquee& m, bool is_dumb)
{
  esmod::build_LUT (esmod_option_type (option_type),
                    esmod_film_type (film_type),
                    esmod_pixel_type (s.imgtype.pixeltype),
                    esmod_image_type (s.imgtype.ae_option),
                    m.gamma, m.highlight, m.shadow, m.graybalance,
                    m.film_gamma, m.film_yp, m.grayl,
                    m.gamma_table[0], m.gamma_table[1], m.lut.gamma_r,
                    is_dumb);
}

inline esmod::type_type
iscan::esmod_film_type (int iscan_film_type)
{
  esmod::type_type val;

  switch (iscan_film_type)
    {
    case PISA_FT_POSI:
      val = ESMOD_FILM_POSITIVE;
      break;
    case PISA_FT_NEGA:
      val = ESMOD_FILM_NEGATIVE;
      break;
    case PISA_FT_REFLECT:
      val = -1;
      break;
    default:
      throw;
    }
  return val;
}

inline esmod::type_type
iscan::esmod_focus_type (int iscan_focus_type)
{
  esmod::type_type val;

  switch (iscan_focus_type)
    {
    case PISA_SH_UMASK:
      val = ESMOD_FOCUS_UMASK;
      break;
    case PISA_SH_GAUSS:
      val = ESMOD_FOCUS_GAUSS;
      break;
    case PISA_SH_UMASKY:
      val = ESMOD_FOCUS_UMASK_Y;
      break;
    default:
      throw;
    }
  return val;
}

inline esmod::type_type
iscan::esmod_image_type (int iscan_image_type)
{
  esmod::type_type val;

  switch (iscan_image_type)
    {
    case PISA_AE_PHOTO:
      val = ESMOD_IMAGE_PHOTO;
      break;
    case PISA_AE_DOC:
      val = ESMOD_IMAGE_DOCUMENT;
      break;
    case PISA_AE_GRAYED:
      val = ESMOD_IMAGE_LINE_ART;
      break;
    default:
      throw;
    }
  return val;
}

inline esmod::type_type
iscan::esmod_option_type (int iscan_option_type)
{
  esmod::type_type val;

  switch (iscan_option_type)
    {
    case PISA_OP_FLATBED:
      val = ESMOD_OPTION_FLATBED;
      break;
    case PISA_OP_ADF:
    case PISA_OP_ADFDPLX:
      val = ESMOD_OPTION_ADF;
      break;
    case PISA_OP_TPU:
      val = ESMOD_OPTION_TPU;
      break;
    default:
      throw;
    }
  return val;
}

inline esmod::type_type
iscan::esmod_pixel_type (int iscan_pixel_type)
{
  esmod::type_type val;

  switch (iscan_pixel_type)
    {
    case PISA_PT_BW:
      val = ESMOD_PIXEL_MONO;
      break;
    case PISA_PT_GRAY:
      val = ESMOD_PIXEL_GRAY;
      break;
    case PISA_PT_RGB:
      val = ESMOD_PIXEL_RGB;
      break;
    default:
      throw;
    }
  return val;
}

inline esmod::type_type
iscan::esmod_scale_type (int iscan_scale_type)
{
  esmod::type_type val;

  switch (iscan_scale_type)
    {
    case PISA_RS_NN:
      val = ESMOD_SCALE_NEAREST_NEIGHBOUR;
      break;
    case PISA_RS_BL:
      val = ESMOD_SCALE_BILINEAR;
      break;
    case PISA_RS_BC:
      val = ESMOD_SCALE_BICUBIC;
      break;
    default:
      throw;
    }
  return val;
}

#endif  /* !defined (esmod_wrapper_hh_included) */
