/* pisa_sane_scan.cc
   Copyright (C) 2001, 2004, 2005, 2008, 2009  SEIKO EPSON CORPORATION

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


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>

#include "pisa_sane_scan.h"
#include "pisa_error.h"
#include "pisa_change_unit.h"
#include "pisa_enums.h"


#define MM_PER_INCH	25.4

#define SCSI_STR	"SCSI"
#define PIO_STR		"PIO"
#define USB_STR		"USB"


bool
sane_scan::has_flatbed (void) const
{
  return (PISA_OP_FLATBED & support_option);
}

void
sane_scan::select_flatbed (void)
{
  if (!has_flatbed ()) return;

  _source = PISA_OP_FLATBED;
  _film   = PISA_FT_REFLECT;
}

bool
sane_scan::using_flatbed (void) const
{
  return (PISA_OP_FLATBED == _source);
}

bool
sane_scan::has_adf (void) const
{
  return ((PISA_OP_ADF | PISA_OP_ADFDPLX) & support_option);
}

void
sane_scan::select_adf (bool duplex)
{
  if (!has_duplex ()) duplex = false;
  if (!has_adf ()) return;

  _source = (duplex ? PISA_OP_ADFDPLX : PISA_OP_ADF);
  _film   = PISA_FT_REFLECT;
}

bool
sane_scan::using_adf (void) const
{
  return (PISA_OP_ADF == _source || PISA_OP_ADFDPLX == _source);
}

bool
sane_scan::has_duplex (void) const
{
  return (PISA_OP_ADFDPLX & support_option);
}

bool
sane_scan::using_duplex (void) const
{
  return (PISA_OP_ADFDPLX == _source);
}

bool
sane_scan::has_tpu (void) const
{
  return (PISA_OP_TPU & support_option);
}

void
sane_scan::select_tpu (bool positive)
{
  if (!has_tpu ()) return;

  _source = PISA_OP_TPU;
  _film   = (positive ? PISA_FT_POSI : PISA_FT_NEGA);
}

bool
sane_scan::using_tpu (void) const
{
  return (PISA_OP_TPU == _source);
}

bool
sane_scan::has_dfd (void) const
{
  const SANE_Option_Descriptor *sod = NULL;
  int id = 0;

  try {
    id = get_option_id ("double-feed-detection-sensitivity");
    sod = sane_get_option_descriptor (m_hdevice, id);
    return (sod && !(SANE_CAP_INACTIVE & sod->cap));
  }
  catch (pisa_error& e) {
    if (pisa_error (PISA_ERR_UNSUPPORT) == e) return false;
    throw e;
  }
}

void
sane_scan::set_dfd (long idx)
{
  const SANE_Option_Descriptor *sod = NULL;
  int id = get_option_id ("double-feed-detection-sensitivity");

  sod = sane_get_option_descriptor (m_hdevice, id);
  if (!sod
      || SANE_CAP_INACTIVE & sod->cap
      || SANE_TYPE_STRING != sod->type
      || SANE_CONSTRAINT_STRING_LIST != sod->constraint_type)
    throw pisa_error (PISA_STATUS_UNSUPPORTED);

  const SANE_String_Const *p = sod->constraint.string_list;
  while (p && *p && ++p)
    ;

  if (idx < p - sod->constraint.string_list)
    set_value ("double-feed-detection-sensitivity",
               sod->constraint.string_list[idx]);
  else
    throw pisa_error (PISA_STATUS_UNSUPPORTED);
}

char
sane_scan::get_scan_source (void) const
{
  if (PISA_OP_NONE == _source)
    throw pisa_error (PISA_ERR_CONNECT);

  return _source;
}

char
sane_scan::get_film_type (void) const
{
  return _film;
}

void
sane_scan::clear_button_status (void)
{
  // We (re)set the scan source.  This indirectly resets the scanner's
  // push button status.  I'd like to do something less kludgy but for
  // the moment can't think of anything better :-(
  set_scan_source ();
}

void
sane_scan::init (void)
{
  m_hdevice = 0;

  name = 0;
  max_resolution = -1;

  _source = PISA_OP_NONE;
  _film   = PISA_FT_REFLECT;

  atexit (sane_exit);
  sane_init (0, 0);
}

void
sane_scan::open_device (char *name)
{
  SANE_Status status;
  char *device_name = 0;

  if (!name)
    {
      device_name = new char[64];
      query_device (device_name);
    }
  else
    device_name = name;

  SANE_Handle device;
  status = sane_open (device_name, &device);

  if (SANE_STATUS_GOOD != status)
    {
      if (device_name != name)
	delete[] device_name;
      throw pisa_error (status);
    }

  // we successfully opened a new device, now close whatever we are
  // hanging onto and rebind to the newly opened device

  close_device ();
  m_hdevice = device;

  get_scanner_info (device_name);

  if (device_name != name)
    delete[] device_name;
}

void
sane_scan::close_device (void)
{
  if (m_hdevice)
    sane_close (m_hdevice);

  m_hdevice = 0;
}

void
sane_scan::set_scan_source (void)
{
  if (!m_hdevice)
    throw pisa_error (PISA_ERR_CONNECT);

  char source[32];

  if (using_flatbed ())
    strcpy (source, "Flatbed");
  else if (using_adf ())
    strcpy (source, "Automatic Document Feeder");
  else if (using_tpu ())
    strcpy (source, "Transparency Unit");
  else
    throw pisa_error (PISA_ERR_PARAMETER);

  set_value (SANE_NAME_SCAN_SOURCE, source);

  if (using_adf ())
    {
      char mode[16];

      strcpy (mode, (using_duplex () ? "Duplex" : "Simplex"));
      set_value ("adf-mode", mode);
    }

  max_resolution = -1;
}

void
sane_scan::set_film_type (void)
{
  char	film_type[32];

  switch (_film)
    {
    case PISA_FT_POSI:
      strcpy (film_type, "Positive Film");
      break;

    case PISA_FT_NEGA:
      strcpy (film_type, "Negative Film");
      break;

    case PISA_FT_REFLECT:
      return;
    }

  set_value ("film-type", (void *) film_type);
}

void
sane_scan::get_current_max_size (double *width, double *height)
{
  SANE_Int max_x = 0;
  SANE_Int max_y = 0;

  if (!m_hdevice)
    throw pisa_error ( PISA_ERR_CONNECT);

  set_scan_source ();
  set_film_type ();
  
  int opt_id = 0;
  const SANE_Option_Descriptor *opt_desc = NULL;

  opt_id = get_option_id (SANE_NAME_SCAN_BR_X);
  opt_desc = sane_get_option_descriptor (m_hdevice, opt_id);
  if (!opt_desc || SANE_CONSTRAINT_RANGE != opt_desc->constraint_type)
    throw pisa_error (PISA_STATUS_UNSUPPORTED);
  
  max_x = opt_desc->constraint.range->max;

  opt_id = get_option_id (SANE_NAME_SCAN_BR_Y);
  opt_desc = sane_get_option_descriptor (m_hdevice, opt_id);
  if (!opt_desc || SANE_CONSTRAINT_RANGE != opt_desc->constraint_type)
    throw pisa_error (PISA_STATUS_UNSUPPORTED);
  
  max_y = opt_desc->constraint.range->max;

  *width = SANE_UNFIX (max_x) / MM_PER_INCH;
  *height = SANE_UNFIX (max_y) / MM_PER_INCH;
}


void
sane_scan::get_color_profile (double *coef) const
{
  SANE_Word value;
  char option_name[16];
  int i;

  if (!m_hdevice)
    throw pisa_error (PISA_ERR_CONNECT);

  for (i = 0; i < 9; i++)
    {
      sprintf (option_name, "cct-%d", i + 1);
      get_value (option_name, (void *) &value);
      coef[i] = SANE_UNFIX (value);
    }
}

void
sane_scan::start_scan (int * width, int * height)
{
  SANE_Status	status;

  if (! m_hdevice)
    throw pisa_error (PISA_ERR_CONNECT);

  if (SANE_STATUS_GOOD != (status = sane_start (m_hdevice)))
    {
      pisa_error oops (status, *this);
      if (!using_adf ()
          || SANE_STATUS_NO_DOCS != status) sane_cancel (m_hdevice);
      throw oops;
    }

  if (SANE_STATUS_GOOD != (status = sane_get_parameters (m_hdevice,
							 &m_sane_para)))
    {
      pisa_error oops (status, *this);
      sane_cancel (m_hdevice);
      throw oops;
    }

  *width  = m_sane_para.pixels_per_line;
  *height = m_sane_para.lines;

  m_rows = 0;
}

SANE_Status
sane_scan::acquire_image (unsigned char *img, int row_bytes,
			  int height, int cancel)
{
  if (!m_hdevice)
    throw pisa_error (PISA_ERR_CONNECT);

  SANE_Status    status  = SANE_STATUS_GOOD;
  unsigned char *cur_pos = img;

  for (int i = 0; i < height; i++, cur_pos += row_bytes)
    {
      m_rows++;
      if (cancel)
	{
	  sane_cancel (m_hdevice);
	  return SANE_STATUS_CANCELLED;
	}

      // The SANE standard does not promise to return as much data as
      // we request, so we keep asking until we got all that we want.

      int cnt = row_bytes;
      int len = 0;
      while (SANE_STATUS_GOOD == status && cnt > 0)
	{
	  status = sane_read (m_hdevice, cur_pos + (row_bytes - cnt),
			      cnt, &len);

	  cnt -= len;
	}

      if (status == SANE_STATUS_EOF)
	break;

      if (status != SANE_STATUS_GOOD)
	{
          throw pisa_error (status, *this);
	}

    }

  return status;
}

/*! \brief  Returns the largest resolution not larger than a \a cutoff.

    Returns the largest supported hardware resolution that does not
    exceed the \a cutoff.  Passing a negative value for the \a cutoff,
    which is the default, returns the maximum resolution supported by
    the hardware.

    Throws an unsupported exception when no suitable resolution can be
    determined.
 */
long
sane_scan::get_max_resolution (long cutoff) const
{
  if (0 > cutoff		// use cached value if possible
      && 0 < max_resolution)
    return max_resolution;

  long result = -1;

  try
    {
      long r;

      r = get_max_resolution (SANE_NAME_SCAN_X_RESOLUTION, cutoff);
      if (r > result) result = r;
      r = get_max_resolution (SANE_NAME_SCAN_Y_RESOLUTION, cutoff);
      if (r > result) result = r;
    }
  catch (const pisa_error& e)
    {
      pisa_error expected (PISA_STATUS_UNSUPPORTED);

      if (e != expected) throw;
    }

  if (0 > result)
    throw pisa_error (PISA_STATUS_UNSUPPORTED);

  if (0 > cutoff		// update cached value if necessary
      && 0 > max_resolution)
    const_cast <sane_scan *> (this)->max_resolution = result;

  return result;
}

long
sane_scan::get_max_resolution (SANE_String_Const option_name,
			       long cutoff) const
{
  int opt_id = get_option_id (option_name);

  const SANE_Option_Descriptor
    *opt_desc = sane_get_option_descriptor (m_hdevice, opt_id);

  if (!opt_desc || SANE_TYPE_INT != opt_desc->type)
    throw pisa_error (PISA_STATUS_UNSUPPORTED);

  long result = -1;

  switch (opt_desc->constraint_type)
    {
    case SANE_CONSTRAINT_RANGE:
      {
	const SANE_Range *const range = opt_desc->constraint.range;

	if (0 > cutoff) cutoff = range->max;
	if (0 == range->quant)
	  {
	    result = cutoff;
	  }
	else
	  {			// relies on integer arithmetic
	    result = (((cutoff - range->min) / range->quant)
		      * range->quant) + range->min;
	  }
	break;
      }
    case SANE_CONSTRAINT_WORD_LIST:
      {				// assumes list is in ascending order
	const SANE_Word *list = opt_desc->constraint.word_list;

	size_t last = list[0];

	if (0 > cutoff) cutoff = list[last];
	while (0 < last && cutoff < list[last])
	  {
	    --last;
	  }
	if (0 < last) result = list[last];
	break;
      }
    default:
      throw pisa_error (PISA_STATUS_UNSUPPORTED);
    }

  if (0 > result)
    throw pisa_error (PISA_STATUS_UNSUPPORTED);

  return result;
}

long
sane_scan::get_resolution (SANE_String_Const scan_direction,
			   int min_res) const
{
  SANE_Int def_res_tbl[] = { 75, 150, 300, 600, 1200 };
  SANE_Int def_res_cnt   = 4;

  const SANE_Int *res_tbl;
	SANE_Int  res_cnt;

  {				// initialize resolution table
    int opt_id = get_option_id (scan_direction);
    const SANE_Option_Descriptor *opt_desc
      = sane_get_option_descriptor (m_hdevice, opt_id);

    if (opt_desc && SANE_CONSTRAINT_RANGE == opt_desc->constraint_type)
      {
        long res = min_res;
        long min = opt_desc->constraint.range->min;
        long max = opt_desc->constraint.range->max;

        if (res < min) res = min;
        if (res > max) res = max;

        return res;
      }
    else if (opt_desc
	&& SANE_TYPE_INT	     == opt_desc->type
	&& SANE_CONSTRAINT_WORD_LIST == opt_desc->constraint_type)
      {
	res_cnt =   opt_desc->constraint.word_list[0];
	res_tbl = &(opt_desc->constraint.word_list[1]);
      }
    else
      {
	res_cnt = def_res_cnt;
	res_tbl = def_res_tbl;

	if (1200 == get_max_resolution ())
	  ++res_cnt;
      }
  }

  {				// find appropriate resolution
    int i = 0;
    while (i < res_cnt && res_tbl[i] < min_res)
      {
	++i;
      }
    return (res_cnt == i ? 0 : res_tbl[i]);
  }
}

void
sane_scan::get_scanner_info (const char *dev_name)
{
  int i, opt_id;
  const SANE_Option_Descriptor *opt_desc;
  const char *fbf = "Flatbed";
  const char *tpu = "Transparency Unit";
  const char *adf = "Automatic Document Feeder";

  if (dev_name)
    {
      char *copy = strdup (dev_name);
      if (!copy)
	throw pisa_error (PISA_STATUS_NO_MEM);

      if (name)
	free (name);
      name = copy;
    }
  support_option = 0;
  opt_id = get_option_id (SANE_NAME_SCAN_SOURCE);
  opt_desc = sane_get_option_descriptor (m_hdevice, opt_id);

  if (opt_desc->type != SANE_TYPE_STRING ||
       opt_desc->constraint_type != SANE_CONSTRAINT_STRING_LIST)
    throw pisa_error (PISA_ERR_CONNECT);

  for (i = 0; opt_desc->constraint.string_list[i]; i++)
    {
      if (0 == strcmp (opt_desc->constraint.string_list[i], fbf))
        {
          support_option |= PISA_OP_FLATBED;
        }
      if (0 == strcmp (opt_desc->constraint.string_list[i], tpu))
        {
          support_option |= PISA_OP_TPU;
        }
      if (0 == strcmp (opt_desc->constraint.string_list[i], adf))
        {
          support_option |= PISA_OP_ADF;
        }
    }

  if (PISA_OP_NONE == _source)
    {
      if (has_flatbed ())
        _source = PISA_OP_FLATBED;
      else if (has_adf ())
        _source = PISA_OP_ADF;
      else if (has_tpu ())
        _source = PISA_OP_TPU;
      else
        throw pisa_error (PISA_ERR_CONNECT);
    }

  if (has_adf ())
    {
      char old = _source;
      _source = PISA_OP_ADF;
      set_scan_source ();
      if (is_activate ("adf-mode"))
        support_option |= PISA_OP_ADFDPLX;
      _source = old;
    }
  set_scan_source ();

  max_resolution = get_max_resolution ();
}

void
sane_scan::query_device (char *device_name) const
{
  const SANE_Device	** ppdevice_list;
  SANE_Status		status;
  int			i;

  status = sane_get_devices (& ppdevice_list, SANE_TRUE);

  if (status != SANE_STATUS_GOOD)
    throw pisa_error (status);

  if (*ppdevice_list == 0 ||
      ppdevice_list[0]->vendor == 0)
    throw pisa_error (PISA_ERR_CONNECT);

  for (i = 0; ppdevice_list[i]; i++)
    {
      if (0 == strcasecmp (ppdevice_list[i]->vendor, "epson"))
	{
	  strcpy (device_name, ppdevice_list[i]->name);
	  return;
	}
    }

  throw pisa_error (PISA_ERR_CONNECT);
}

void
sane_scan::finish_acquire (bool eject)
{
  if (eject && using_adf ())
    {
      set_value ("eject", NULL);
    }
  sane_cancel (m_hdevice);
}

int
sane_scan::is_activate (const char *option_name) const
{
  const SANE_Option_Descriptor	* opt_desc;
  SANE_Int			num_dev_options;
  SANE_Status			status;
  int				i;
  int				ret;

  ret = 0;
  status = sane_control_option (m_hdevice, 0,
				SANE_ACTION_GET_VALUE,
				&num_dev_options,
				0);

  if (status != SANE_STATUS_GOOD)
    throw pisa_error (status, *this);

  for (i = 0; i < num_dev_options; i++)
    {
      opt_desc = sane_get_option_descriptor (m_hdevice, i);

      if (opt_desc->name && strcmp (opt_desc->name, option_name) == 0)
	{
	  if (SANE_OPTION_IS_ACTIVE (opt_desc->cap))
	    {
	      ret = 1;
	      break;
	    }
	  else
	    break;
	}
    }

  return ret;
}

int
sane_scan::get_option_id (const char *option_name) const
{
  const SANE_Option_Descriptor	* opt_desc;
  SANE_Int			num_dev_options;
  SANE_Status			status;
  int				i;

  status = sane_control_option (m_hdevice, 0,
				SANE_ACTION_GET_VALUE,
				&num_dev_options,
				0);

  if (status != SANE_STATUS_GOOD)
    throw pisa_error (status, *this);

  for (i = 0; i < num_dev_options; i++)
    {
      opt_desc = sane_get_option_descriptor (m_hdevice, i);

      if (opt_desc->name && strcmp (opt_desc->name, option_name) == 0)
	return i;	// found
    }

  throw pisa_error (PISA_ERR_UNSUPPORT);
}

void
sane_scan::set_value (const char * option_name, const void * value)
{
  SANE_Status	status;
  int		option_id;

  option_id = get_option_id (option_name);

  status = sane_control_option (m_hdevice,
				option_id,
				SANE_ACTION_SET_VALUE,
                                const_cast<void *>(value),
				0);

  if (status != SANE_STATUS_GOOD)
    throw pisa_error (status, *this);
}

void
sane_scan::get_value (const char *option_name, void *value, bool nothrow) const
{
  SANE_Status status;
  int option_id;

  try
    {
      option_id = get_option_id (option_name);
    }
  catch (pisa_error& oops)
    {
      if (!nothrow) throw oops;
      return;
    }

  status = sane_control_option (m_hdevice, option_id,
				SANE_ACTION_GET_VALUE,
				value,
				0);

  if (!nothrow && status != SANE_STATUS_GOOD)
    throw pisa_error (status, *this);
}

void
sane_scan::get_range (const char *option_name,
                      float *max, float *min, float *step)
{
  const SANE_Option_Descriptor *opt_desc = NULL;
  int opt_id = 0;

  if (!option_name)
    {
      throw pisa_error (PISA_ERR_PARAMETER);
    }
  if (!m_hdevice)
    {
      throw pisa_error (PISA_ERR_CONNECT);
    }
  opt_id = get_option_id (option_name);
  opt_desc = sane_get_option_descriptor (m_hdevice, opt_id);
  if (!opt_desc
      || SANE_CONSTRAINT_RANGE != opt_desc->constraint_type)
    {
      throw pisa_error (PISA_STATUS_UNSUPPORTED);
    }

  if ( SANE_TYPE_FIXED == opt_desc->type )
    {
      if (max)
        {
          *max = SANE_UNFIX (opt_desc->constraint.range->max);
        }
      if (min)
        {
          *min = SANE_UNFIX (opt_desc->constraint.range->min);
        }
      if (step)
        {
          *step = SANE_UNFIX (opt_desc->constraint.range->quant);
        }
    }
  else
    {
      if (max)
        {
          *max = opt_desc->constraint.range->max;
        }
      if (min)
        {
          *min = opt_desc->constraint.range->min;
        }
      if (step)
        {
          *step = opt_desc->constraint.range->quant;
        }
    }
}

void
sane_scan::set_preview (bool value)
{
  SANE_Bool v = value;
  set_value ("preview", &v);
}

bool
sane_scan::has_preview (void) const
{
  return is_activate ("preview");
}

void
sane_scan::set_focus (long position)
{
  char focus[32];

  if (0 == is_activate ("focus-position"))
    return;

  if (position == 25)
    strcpy (focus, "Focus 2.5mm above glass");
  else
    strcpy (focus, "Focus on glass");

  set_value ("focus-position", (void *) focus);
}

void
sane_scan::set_speed (long speed)
{
  SANE_Bool value;

  if (0 == is_activate ("speed"))
    return;

  if (speed == 1)
    value = SANE_TRUE;
  else
    value = SANE_FALSE;

  set_value ("speed", (void *) & value);
}

bool
sane_scan::has_zoom (void) const
{
  return is_activate ("zoom");
}

bool
sane_scan::has_focus (void) const
{
  return is_activate ("focus-position");

}

bool
sane_scan::has_draft_mode (void) const
{
  return is_activate ("speed");
}

bool
sane_scan::get_size_check (void) const
{
  SANE_Bool value;
  get_value ("detect-doc-size", &value);
  return value;
}

void
sane_scan::set_size_check (bool value)
{
  SANE_Bool v = value;
  set_value ("detect-doc-size", &v);
}

bool
sane_scan::has_size_check (void) const
{
  return is_activate ("detect-doc-size");
}

bool
sane_scan::get_autocrop (void) const
{
  SANE_Bool value;
  get_value ("autocrop", &value);
  return value;
}

void
sane_scan::set_autocrop (bool value)
{
  SANE_Bool v = value;
  set_value ("autocrop", &v);
}

bool
sane_scan::has_autocrop (void) const
{
  return is_activate ("autocrop");
}

bool
sane_scan::get_deskew (void) const
{
  SANE_Bool value;
  get_value ("deskew", &value);
  return value;
}

void
sane_scan::set_deskew (bool value)
{
  SANE_Bool v = value;
  set_value ("deskew", &v);
}

bool
sane_scan::has_deskew (void) const
{
  return is_activate ("deskew");
}

bool
sane_scan::has_start_button (void) const
{
  return is_activate ("monitor-button");
}

bool
sane_scan::is_button_pressed (void) const
{
  SANE_Bool value = false;

  get_value ("monitor-button", (void *) &value);

  return value;
}

long
sane_scan::get_polling_time (void) const
{
  SANE_Word value = 0;

  get_value ("polling-time", (void *) &value);

  return value;
}

void
sane_scan::disable_wait_for_button (void)
{
  SANE_Bool value = false;
  SANE_Bool off = false;
  if (is_activate ("wait-for-button"))
  {
    get_value ("wait-for-button", &value);
  }
  if (value)
  {
    set_value ("wait-for-button", &off);
  }
}

void
sane_scan::set_color_mode (char pixeltype, char bitdepth)
{
  char color_mode[32];
  SANE_Word depth;

  switch (pixeltype)
    {
    case PISA_PT_RGB:
      strcpy (color_mode, "Color");
      break;

    case PISA_PT_GRAY:
      strcpy (color_mode, "Gray");
      break;

    case PISA_PT_BW:
      strcpy (color_mode, "Binary");
      break;
    }

  switch (bitdepth)
    {
    case PISA_BD_1:
      depth = 1;
      break;

    case PISA_BD_8:
      depth = 8;
      break;
    }

  if (is_activate (SANE_NAME_SCAN_MODE))
    {
      set_value (SANE_NAME_SCAN_MODE, (void *) color_mode);
    }
  if (is_activate (SANE_NAME_BIT_DEPTH))
    {
      set_value (SANE_NAME_BIT_DEPTH, (void *) &depth);
    }
}

void
sane_scan::set_gamma_table (const unsigned char *gamma_table)
{
  SANE_Word	table[256];
  int		i;
  char		user_defined[32];

  strcpy (user_defined, "User defined (Gamma=1.8)");

  set_value ("gamma-correction", (void *) user_defined);

  // red
  for (i = 0; i < 256; i++)
    table[i] = gamma_table[256 * 0 + i];
  set_value (SANE_NAME_GAMMA_VECTOR_R, (void *) table);

  // green
  for (i = 0; i < 256; i++)
    table[i] = gamma_table[256 * 1 + i];
  set_value (SANE_NAME_GAMMA_VECTOR_G, (void *) table);

  // blue
  for (i = 0; i < 256; i++)
    table[i] = gamma_table[256 * 2 + i];
  set_value (SANE_NAME_GAMMA_VECTOR_B, (void *) table );
}

void
sane_scan::set_threshold (long threshold)
{
  set_value (SANE_NAME_THRESHOLD, (void *) & threshold);
}

void
sane_scan::set_color_profile (const double * coef)
{
  SANE_Word	value;
  char		option_name[16];
  int		i;
  char		user_defined[32];

  strcpy (user_defined, "User defined");

  set_value ("color-correction", (void *) user_defined);

  for (i = 0; i < 9; i++)
    {
      sprintf (option_name, "cct-%d", i + 1);
      value = SANE_FIX (coef[i]);
      set_value (option_name, (void *) &value);
    }
}

void
sane_scan::set_brightness (long brightness)
{
  set_value (SANE_NAME_BRIGHTNESS, (void *) & brightness);
}

bool
sane_scan::has_brightness (void) const
{
  return is_activate (SANE_NAME_BRIGHTNESS);
}

void
sane_scan::set_contrast (long contrast)
{
  set_value (SANE_NAME_CONTRAST, (void *) & contrast);
}

bool
sane_scan::has_contrast (void) const
{
  return is_activate (SANE_NAME_CONTRAST);
}

void
sane_scan::set_brightness_method (br_method_val val)
{
  char method[32];

  switch (val)
    {
    case br_gimp:
      strcpy (method, "gimp");
      break;
    case br_iscan:
    default:
      strcpy (method, "iscan");
      break;
    }

  set_value ("brightness-method", (void *) method);
}

void
sane_scan::set_scan_resolution (int resolution_x, int resolution_y)
{
  set_value (SANE_NAME_SCAN_X_RESOLUTION, (void *) &resolution_x);
  set_value (SANE_NAME_SCAN_Y_RESOLUTION, (void *) &resolution_y);
}

void
sane_scan::set_scan_zoom (int zoom_x, int zoom_y)
{
  set_value ("zoom", (void *) & zoom_x);

  zoom_y = zoom_y;
}

void
sane_scan::set_scan_area (double offset_x, double offset_y,
			  double width, double height)
{
  SANE_Word value;
  double tmp;

  value = SANE_FIX (offset_x * MM_PER_INCH);
  set_value (SANE_NAME_SCAN_TL_X, (void *) &value);

  value = SANE_FIX (offset_y * MM_PER_INCH);
  set_value (SANE_NAME_SCAN_TL_Y, (void *) &value);

  tmp = offset_x + width;
  value = SANE_FIX (tmp * MM_PER_INCH);
  set_value (SANE_NAME_SCAN_BR_X, (void *) &value);

  tmp = offset_y + height;
  value = SANE_FIX (tmp * MM_PER_INCH);
  set_value (SANE_NAME_SCAN_BR_Y, (void *) &value);
}

bool
sane_scan::has_clean (void) const
{
  return is_activate ("clean");
}

bool
sane_scan::has_calibrate (void) const
{
  return is_activate ("calibrate");
}

void
sane_scan::clean (void)
{
  set_value ("clean", NULL);
}

void
sane_scan::calibrate (void)
{
  set_value ("calibrate", NULL);
}
