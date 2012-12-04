/*  backend.c -- implements the SANE epkowa backend
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#define BACKEND_CREATE_FORWARDERS
#include "backend.h"

#define BACKEND_BUILD  213
#define BACKEND_SOURCE PACKAGE_STRING

#include <errno.h>
#include <ltdl.h>
#include <string.h>

#include "cfg-obj.h"
#include "dip-obj.h"
#include "net-obj.h"
#include "model-info.h"
#include "utils.h"

/* Deprecated includes */
#include "include/sane/sanei_usb.h"
#include "epkowa.h"

/* Deprecated API calls */
extern SANE_Status epkowa_open (const char *, SANE_Handle *, const void *);


typedef struct
{
  void  *cfg;
  void  *net;
  void  *dip;
  list  *sane_dev;
  void **dev_list;
  void  *model_info_cache;

} backend_type;

static backend_type *be = NULL;


static void be_sane_dev_dtor (void *);


/*! \defgroup SANE_API  SANE API Entry Points
 *
 *  The SANE API entry points make up the \e full API available to the
 *  SANE frontend application programmer.  Users of this API should be
 *  careful \e never to assume \e anything about a backend's behaviour
 *  beyond what is required by the SANE standard.  The standard can be
 *  retrieved via http://www.sane-project.org/docs.html.
 *
 *  Whatever documentation may be provided here serves to document the
 *  implementation, if anything.  In case of discrepancy with the SANE
 *  specification, the SANE specification is correct.
 *
 *  @{
 */


/*! \brief  Prepares the backend for use by a SANE frontend.
 *
 *  \remarks
 *  This function \e must be called before any other SANE API entry is
 *  called.  It is the only SANE function that may be called after the
 *  sane_exit() function has been called.
 *
 *  \todo
 *  Deal properly with the \c SANE_Auth_Callback.
 *  It is completely ignored in the current implementation.
 */
SANE_Status
sane_init (SANE_Int *version_code, SANE_Auth_Callback authorize)
{
  SANE_Status status = SANE_STATUS_GOOD;

  if (be)
    {
      log_call ("(%p, %p)", version_code, authorize);
      err_minor ("backend already initialised");
      return status;
    }

  msg_init ();
  log_call ("(%p, %p)", version_code, authorize);
  log_info ("%s", BACKEND_SOURCE);
  log_info ("version %d.%d.%d", SANE_MAJOR, SANE_MINOR, BACKEND_BUILD);

  if (version_code)
    {
      *version_code = SANE_VERSION_CODE (SANE_MAJOR, SANE_MINOR,
                                         BACKEND_BUILD);
    }

  if (authorize)
    {
      err_minor ("authorisation not supported");
    }

  be = t_calloc (1, backend_type);
  if (!be)
    {
      return SANE_STATUS_NO_MEM;
    }

  /*  Needs to be done _before_ cfg_init() because that refers to the
   *  model_info_cache somewhere deep down in its bowels.
   */
  be->model_info_cache = model_info_cache_init (MODELDATADIR, &status);
  if (!be->model_info_cache)
    {
      sane_exit ();
      return status;
    }

  be->cfg = cfg_init (PKGDATADIR, &status);
  if (!be->cfg)
    {
      sane_exit ();
      return status;
    }

  if (cfg_has (be->cfg, CFG_KEY_NET))
    {
      be->net = net_init (PKGLIBDIR, &status);
      if (!be->net)
        {
          if (SANE_STATUS_GOOD != status)
            err_fatal ("%s", sane_strstatus (status));
          err_major ("disabling network device support");
          cfg_set (be->cfg, CFG_KEY_NET, false);
          status = SANE_STATUS_GOOD;
        }
    }
  if (cfg_has (be->cfg, CFG_KEY_PIO))
    {
      /* place holder */
    }
  if (cfg_has (be->cfg, CFG_KEY_SCSI))
    {
      /* place holder */
    }
  if (cfg_has (be->cfg, CFG_KEY_USB))
    {
      sanei_usb_init ();
    }

  if (cfg_has (be->cfg, CFG_KEY_INTERPRETER))
    {
      if (0 != lt_dlinit ())
        {
          err_fatal ("%s", lt_dlerror ());
          err_major ("disabling interpreter support");
          cfg_set (be->cfg, CFG_KEY_INTERPRETER, false);
        }
    }

  be->dip = dip_init (PKGLIBDIR, &status);
  if (!be->dip)
    {
      sane_exit ();
      return status;
    }

  return status;
}


/*! \brief  Releases all resources held by the backend.
 *
 *  \remarks
 *  Applications \e must call this function to terminate use of the
 *  backend.  After it has been called, sane_init() has to be called
 *  before other SANE API can be used.  The function needs to close
 *  any open handles.
 *
 *  \remarks
 *  The implementation must be able to deal properly with a partially
 *  initialised backend so that sane_init() can use this function for
 *  its error recovery.
 */
void
sane_exit (void)
{
  log_call ("()");

  if (!be)
    {
      msg_init ();
      err_minor ("backend is not initialized");
      return;
    }

  be->dip = dip_exit (be->dip);

  if (cfg_has (be->cfg, CFG_KEY_INTERPRETER))
    {
      lt_dlexit ();
    }

  if (cfg_has (be->cfg, CFG_KEY_USB))
    {
      /* sanei_usb_exit, if only there was one! */
    }
  if (cfg_has (be->cfg, CFG_KEY_SCSI))
    {
      /* place holder */
    }
  if (cfg_has (be->cfg, CFG_KEY_PIO))
    {
      /* place holder */
    }
  if (be->net)
    {
      be->net = net_exit (be->net);
    }

  be->cfg = cfg_exit (be->cfg);
  delete (be->dev_list);
  list_destroy (be->sane_dev, be_sane_dev_dtor);

  be->model_info_cache = model_info_cache_exit (be->model_info_cache);

  delete (be);
}


/*! \brief  Creates a list of devices available through the backend.
 *
 *  \remarks
 *  The returned \a device_list \e must remain unchanged and valid
 *  until this function is called again or sane_exit() is called.
 *
 *  \remarks
 *  Applications are \e not required to call this function before
 *  they call sane_open().
 */
SANE_Status
sane_get_devices (const SANE_Device ***device_list, SANE_Bool local_only)
{
  SANE_Status status = SANE_STATUS_GOOD;
  list *sane_dev = NULL;

  log_call ("(%p, %d)", device_list, local_only);

  if (!be)
    {
      msg_init ();
      err_fatal ("backend is not initialized");
      return SANE_STATUS_ACCESS_DENIED;
    }

  if (!device_list)
    {
      err_fatal ("%s", strerror (EINVAL));
      return SANE_STATUS_INVAL;
    }

  sane_dev = list_create ();
  if (sane_dev)
    {
      if (!local_only && cfg_has (be->cfg, CFG_KEY_NET))
        {
          cfg_find (be->cfg, CFG_KEY_NET, sane_dev);
        }
      if (cfg_has (be->cfg, CFG_KEY_PIO))
        {
          cfg_find (be->cfg, CFG_KEY_PIO, sane_dev);
        }
      if (cfg_has (be->cfg, CFG_KEY_SCSI))
        {
          cfg_find (be->cfg, CFG_KEY_SCSI, sane_dev);
        }
      if (cfg_has (be->cfg, CFG_KEY_USB))
        {
          cfg_find (be->cfg, CFG_KEY_USB, sane_dev);
        }

      if (cfg_has (be->cfg, CFG_KEY_INTERPRETER))
        {
          cfg_find (be->cfg, CFG_KEY_INTERPRETER, sane_dev);
        }

      if (be->sane_dev)
        {
          delete (be->dev_list);
          list_destroy (be->sane_dev, be_sane_dev_dtor);
        }
      be->sane_dev = sane_dev;
    }

  be->dev_list = list_normalize (be->sane_dev);
  *device_list = (const SANE_Device **) be->dev_list;
  if (!*device_list)
    {
      status = SANE_STATUS_NO_MEM;
    }

  return status;
}


/*! \brief  Establishes a connection to a named device.
 *
 *  \remarks
 *  Applications are allowed to call this function directly, without a
 *  call to sane_get_devices() first.  An empty string may be used for
 *  the \a device_name to request the first available device.
 *
 *  \todo Register the handle with \c be before \c SANE_STATUS_GOOD is
 *        returned so we can check in the other API entries whether we
 *        got a known handle passed in as well as call sane_close() on
 *        all open handles in sane_exit().
 */
SANE_Status
sane_open (SANE_String_Const device_name, SANE_Handle *handle)
{
  const SANE_Device *sane_dev = NULL;

  log_call ("(%s, %p)", device_name, handle);

  if (!be)
    {
      msg_init ();
      err_fatal ("backend is not initialized");
      return SANE_STATUS_ACCESS_DENIED;
    }

  if (!handle)
    {
      err_fatal ("%s", strerror (EINVAL));
      return SANE_STATUS_INVAL;
    }

  if (!device_name)
    {
      /*  The SANE API specification explicitly talks about a zero
       *  length string.  There is no mention about what should be
       *  done in the case where there is NO string at all.
       *  We degrade gracefully.
       */
      err_minor ("assuming frontend meant to pass an empty string");
    }

  if (!be->sane_dev)
    {                       /* FIXME: does more than necessary */
      const SANE_Device **dev = NULL;
      sane_get_devices (&dev, false);
    }

  if (0 == list_size (be->sane_dev))
    {
      err_major ("no supported devices available");
      return SANE_STATUS_ACCESS_DENIED;
    }

  if (!device_name || 0 == strlen (device_name))
    {
      sane_dev = be->sane_dev->head->data;
    }
  else
    {
      list_reset (be->sane_dev);
      while ((sane_dev = list_next (be->sane_dev))
             && 0 != strcmp_c (sane_dev->name, device_name))
        {
          /* nothing to do, condition does all the processing */
        }
    }

  if (!sane_dev)
  {
    err_major ("no such device");
    return SANE_STATUS_INVAL;
  }

  return epkowa_open (sane_dev->name, handle, be->dip);
}

//! Obtains the current scan parameters for a device
/*! \remarks
 *  The parameters are only guaranteed to be accurate between a call
 *  to sane_start() and the completion of that request.  Outside of
 *  that scope the parameters are a best effort only and the backend
 *  is at liberty to change them.
 */
SANE_Status
sane_get_parameters (SANE_Handle handle, SANE_Parameters *parameters)
{
  SANE_Status status = SANE_STATUS_GOOD;
  Epson_Scanner *s;

  log_call ("(%p, %p)", handle, parameters);

  if (!handle || !parameters)
    {
      err_fatal ("%s", strerror (EINVAL));
      return SANE_STATUS_INVAL;
    }

  s = (Epson_Scanner *) handle;

  if (s->src->transfer_started && !s->src->transfer_stopped)
    {
      static const char *const color_space[] = {
        "GRAY", "RGB", "RED", "GREEN", "BLUE"
      };

      const SANE_Parameters *p = &s->src->ctx;

      log_info
        ("Scan area   : %.2f x %.2f [mm^2]",
         SANE_UNFIX (s->val[OPT_BR_X].w) - SANE_UNFIX (s->val[OPT_TL_X].w),
         SANE_UNFIX (s->val[OPT_BR_Y].w) - SANE_UNFIX (s->val[OPT_TL_Y].w));
      log_info
        ("Offset      : (%.2f, %.2f) [mm]",
         SANE_UNFIX (s->val[OPT_TL_X].w), SANE_UNFIX (s->val[OPT_TL_Y].w));

      log_info ("Color space : %s-%d", color_space[p->format], p->depth);
      log_info ("Image size  : %d x %d [pixels^2] (%.2f x %.2f [mm^2])",
                p->pixels_per_line, p->lines,
                p->pixels_per_line * MM_PER_INCH / s->val[OPT_X_RESOLUTION].w,
                p->lines * MM_PER_INCH / s->val[OPT_Y_RESOLUTION].w);
      log_info ("X Resolution: %d [dpi]", s->val[OPT_X_RESOLUTION].w);
      log_info ("Y Resolution: %d [dpi]", s->val[OPT_Y_RESOLUTION].w);

      memcpy (parameters, p, sizeof (*p));
    }
  else
    {
      status = estimate_parameters (s, parameters);
    }

  return status;
}

/*! \brief  Acquires up to \a max_length bytes of new image data.
 *
 *  \remarks
 *  The \a length is guaranteed to be zero in case of an unsuccessful
 *  request.
 *
 *  \remarks
 *  The implementation allows for \c NULL as a \a buffer value.  This
 *  caters to a frontends that do not prepare buffer space when they
 *  expect a \c SANE_STATUS_EOF return value.
 */
SANE_Status
sane_read (SANE_Handle handle, SANE_Byte *buffer,
           SANE_Int max_length, SANE_Int *length)
{
  SANE_Status status = SANE_STATUS_GOOD;
  Epson_Scanner *s;

  log_call ("(%p, %p, %i, %p)", handle, buffer, max_length, length);

  if (length) *length = 0;

  if (!handle)
    {
      err_fatal ("%s", strerror (EINVAL));
      return SANE_STATUS_INVAL;
    }

  s = (Epson_Scanner *) handle;

  require (s->src == &s->raw || s->src == &s->img);

  if (s->src == &s->raw)
    {
      status = fetch_image_data (s, buffer, max_length, length);
    }
  else if (s->src == &s->img)
    {
      /**/ if (!s->img.ptr)
        {
          err_major ("%s", strerror (ENOMEM));
          status = SANE_STATUS_NO_MEM;
        }
      else if (s->img.ptr == s->img.end)
        {
          status = SANE_STATUS_EOF;
        }
      else if (s->img.cancel_requested)
        {
          s->img.transfer_stopped = true;
          status = SANE_STATUS_CANCELLED;
        }
      else if (buffer && 0 < max_length)
        {
          SANE_Int len = s->img.end - s->img.ptr;

          if (len > max_length) len = max_length;
          memcpy (buffer, s->img.ptr, len);
          s->img.ptr += len;
          if (length) *length = len;
        }
      else
        {
          status = SANE_STATUS_NO_MEM;
        }
    }

  if (SANE_STATUS_EOF == status)
    {
      s->src->transfer_stopped = true;
    }

  return status;
}

/*!
    @}
 */


/*! Releases the resources held by a SANE_Device.

    This function is primarily useful to maintain the \c sane_dev
    member of a backend_type object.
 */
static void
be_sane_dev_dtor (void *p)
{
  SANE_Device *sd = (SANE_Device *) p;
  if (!sd) return;

  const_delete (sd->name  , SANE_String);
  const_delete (sd->vendor, SANE_String);
  const_delete (sd->model , SANE_String);
  const_delete (sd->type  , SANE_String);

  delete (sd);
}
