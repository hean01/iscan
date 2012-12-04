//  esmod.hh -- exposed API of the 'esmod' library
//  Copyright (C) 2004, 2008  SEIKO EPSON CORPORATION
//
//  This file is part of the 'esmod' library.
//
//  The 'esmod' library is non-free software.
//  It is distributed under the terms of the AVASYS PUBLIC LICENSE.
//
//  You should have received a verbatim copy of the AVASYS PUBLIC
//  LICENSE along with the software.


#ifndef esmod_hh_included
#define esmod_hh_included

#ifndef __cplusplus
#error "This is a C++ header file; use a C++ compiler to compile it."
#endif

#include <cstddef>              /* for size_t */

namespace esmod
{

#define ESMOD_OPTION_FLATBED            (0)
#define ESMOD_OPTION_TPU                (1)
#define ESMOD_OPTION_ADF                (2)

#define ESMOD_FILM_NEGATIVE             (0)
#define ESMOD_FILM_POSITIVE             (1)

#define ESMOD_PIXEL_RGB                 (0)
#define ESMOD_PIXEL_GRAY                (1)
#define ESMOD_PIXEL_MONO                (2)

#define ESMOD_IMAGE_PHOTO               (0)
#define ESMOD_IMAGE_DOCUMENT            (1)
#define ESMOD_IMAGE_LINE_ART            (2)

#define ESMOD_DEFAULT_GAMMA             (1.0)
#define ESMOD_DEFAULT_HILITE          (245)
#define ESMOD_DEFAULT_SHADOW            (8)
#define ESMOD_DEFAULT_THRESHOLD       (160)

#define ESMOD_FOCUS_UMASK               (1)
#define ESMOD_FOCUS_GAUSS               (2)
#define ESMOD_FOCUS_UMASK_Y             (3)

#define ESMOD_SCALE_NEAREST_NEIGHBOUR   (1)
#define ESMOD_SCALE_BILINEAR            (2)
#define ESMOD_SCALE_BICUBIC             (3)


typedef unsigned char byte_type;
typedef size_t        size_type;
typedef bool          bool_type;
typedef int           type_type;
typedef long          long_type;
typedef unsigned long parm_type;
typedef double        real_type;


class filter
{
public:
  virtual ~filter () {};

  //! Returns a block of n bytes of post-filter data.
  virtual filter& getblock (      byte_type *block, size_type n) = 0;
  //! Feeds the filter a block of n bytes of data.
  virtual filter& putblock (const byte_type *block, size_type n) = 0;
  //! Combines putblock() and getblock() in a single call.
  virtual filter& exec     (const byte_type *i_block, size_type i_n,
                                  byte_type *o_block, size_type o_n)
  {
           putblock (i_block, i_n);
    return getblock (o_block, o_n);
  }

  //! Returns number of additional input lines needed for out_lines of output.
  virtual size_type get_line_quote (size_type out_lines)
  { return out_lines; };

protected:
  void *_hidden_data;
  filter () {};

private:                        // undefined to prevent copying
  filter (const filter&);
  filter& operator= (const filter&);
};

class focus : public filter
{
public:
  focus (long_type width, long_type height, long_type rowbytes,
         size_type bits_per_pixel);
  focus (long_type i_width, long_type i_height, long_type i_rowbytes,
         long_type o_width, long_type o_height, long_type o_rowbytes,
         size_type bits_per_pixel,
         parm_type strength, parm_type radius, parm_type clipping,
         type_type focus_type);
  virtual ~focus ();

  void set_parms (size_type resolution, bool_type film_type, bool_type is_dumb,
                  parm_type *strength, parm_type *radius, parm_type *clipping);

  virtual filter& getblock (      byte_type *block, size_type n);
  virtual filter& putblock (const byte_type *block, size_type n);
  virtual filter& exec     (const byte_type *i_block, size_type i_n,
                                  byte_type *o_block, size_type o_n);

  virtual size_type get_line_quote (size_type out_lines);
};

class moire : public filter
{
public:
  moire (long_type i_width, long_type i_height, long_type i_rowbytes,
         long_type o_width, long_type o_height, long_type o_rowbytes,
         size_type bits_per_pixel,
         size_type resolution, bool_type is_dumb);
  virtual ~moire ();

  virtual filter& getblock (      byte_type *block, size_type n);
  virtual filter& putblock (const byte_type *block, size_type n);
  virtual filter& exec     (const byte_type *i_block, size_type i_n,
                                  byte_type *o_block, size_type o_n);

  static  size_type get_res_quote  (size_type out_res, bool_type is_dumb);
  virtual size_type get_line_quote (size_type out_lines);
};

class scale : public filter
{
public:
  scale (long_type i_width, long_type i_height, long_type i_rowbytes,
         long_type o_width, long_type o_height, long_type o_rowbytes,
         size_type bits_per_pixel,
         type_type scale_type);
  virtual ~scale ();

  virtual filter& getblock (      byte_type *block, size_type n);
  virtual filter& putblock (const byte_type *block, size_type n);
  virtual filter& exec     (const byte_type *i_block, size_type i_n,
                                  byte_type *o_block, size_type o_n);

  virtual size_type get_line_quote (size_type out_lines);
};

// WARNING: These quite likely modify global state in libesmod.
void auto_expose (type_type option_type, type_type film_type,
                  byte_type *image,
                  long_type width, long_type height, long_type rowbytes,
                  long_type top, long_type left,
                  long_type bottom, long_type right,
                  long_type *gamma, long_type *hilite,
                  long_type *shadow, long_type *graybalance,
                  real_type film_gamma[3],
                  real_type film_yp[3],
                  real_type grayl[3],
                  bool_type is_photo, bool_type is_dumb);
void build_LUT (type_type option_type, type_type film_type,
                type_type pixel_type, type_type image_type,
                long_type gamma, long_type hilite,
                long_type shadow, long_type graybalance,
                const real_type film_gamma[3],
                const real_type film_yp[3],
                const real_type grayl[3],
                byte_type master[256],
                byte_type rgb[3 * 256],
                byte_type lut[3 * 256],
                bool is_dumb);

} // namespace esmod

#endif  /* !defined (esmod_hh_included) */
