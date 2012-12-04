/*  backend.h -- SANE backend implementation boiler plate
 *  Copyright (C) 2007  EPSON AVASYS CORPORATION
 *  Copyright (C) 2008  SEIKO EPSON CORPORATION
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


#ifndef included_backend_h
#define included_backend_h

/*! \file
 *  \brief  Provides backend implementation boiler plate.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sane/sane.h>
#include <stdio.h>


#ifndef BACKEND_NAME
#error "Define the BACKEND_NAME macro to match your backend's name"
#error "before including this file (backend.h)."
#error "The value should not be quoted.  Characters in your backend"
#error "name that are not allowed in C variables should be replaced"
#error "by underscores, one for each such character."
#endif /* !defined (BACKEND_NAME) */


#ifdef __cplusplus
extern "C"
{
#endif
/*  Set up the machinery used to create the "shadow" API entries.
 *  FIXME: Move these #define's into their own header file so that
 *         multi-file plug-ins can use them too.
 *  FIXME: Add an api_entry() function that mimicks the API_ENTRY()
 *         #define so that plug-ins can use dlsym() and friends more
 *         easily.  You need a way to create names at run-time.  The
 *         #define only works at compile-time!
 */
#define CONCAT_INTERNAL(s1,s2)  s1##s2
#define CONCAT(s1,s2)           CONCAT_INTERNAL(s1,s2)

#define API_ENTRY(backend,function) \
  CONCAT(sane_,CONCAT(backend,CONCAT(_,function)))

/*  For each API entry, set up "shadow" API entries with a unique name.
 */

SANE_Status
API_ENTRY (BACKEND_NAME, init) (SANE_Int *version,
                                SANE_Auth_Callback authoriser);
void
API_ENTRY (BACKEND_NAME, exit) (void);

SANE_Status
API_ENTRY (BACKEND_NAME, get_devices) (const SANE_Device ***device_list,
                                       SANE_Bool local_only);

SANE_Status
API_ENTRY (BACKEND_NAME, open) (SANE_String_Const device_name,
                                SANE_Handle *handle);
void
API_ENTRY (BACKEND_NAME, close) (SANE_Handle handle);

const SANE_Option_Descriptor *
API_ENTRY (BACKEND_NAME, get_option_descriptor) (SANE_Handle handle,
                                                 SANE_Int index);
SANE_Status
API_ENTRY (BACKEND_NAME, control_option) (SANE_Handle handle, SANE_Int index,
                                          SANE_Action action, void *value,
                                          SANE_Word *info);
SANE_Status
API_ENTRY (BACKEND_NAME, get_parameters) (SANE_Handle handle,
                                          SANE_Parameters *parameters);

SANE_Status
API_ENTRY (BACKEND_NAME, start) (SANE_Handle handle);
SANE_Status
API_ENTRY (BACKEND_NAME, read) (SANE_Handle handle, SANE_Byte *buffer,
                                SANE_Int max_length, SANE_Int *length);
void
API_ENTRY (BACKEND_NAME, cancel) (SANE_Handle handle);

SANE_Status
API_ENTRY (BACKEND_NAME, set_io_mode) (SANE_Handle handle,
                                       SANE_Bool non_blocking);
SANE_Status
API_ENTRY (BACKEND_NAME, get_select_fd) (SANE_Handle handle,
                                         SANE_Int *fdp);


SANE_String_Const
API_ENTRY (BACKEND_NAME, strstatus) (SANE_Status status);


#ifdef BACKEND_CREATE_FORWARDERS

/*  For each API entry, forward the original call to the "shadow" API
 *  entry just declared.
 *  Most of the argument validation can be added here.
 */

SANE_Status
sane_init (SANE_Int *version, SANE_Auth_Callback authoriser)
{
  SANE_Status status;

  status = API_ENTRY (BACKEND_NAME, init) (version, authoriser);
  return status;
}

void
sane_exit (void)
{
  API_ENTRY (BACKEND_NAME, exit) ();
  return;
}


SANE_Status
sane_get_devices (const SANE_Device ***device_list, SANE_Bool local_only)
{
  SANE_Status status;

  status = API_ENTRY (BACKEND_NAME, get_devices) (device_list, local_only);
  return status;
}


SANE_Status
sane_open (SANE_String_Const device_name, SANE_Handle *handle)
{
  SANE_Status status;

  status = API_ENTRY (BACKEND_NAME, open) (device_name, handle);
  return status;
}

void
sane_close (SANE_Handle handle)
{
  API_ENTRY (BACKEND_NAME, close) (handle);
  return;
}


const SANE_Option_Descriptor *
sane_get_option_descriptor (SANE_Handle handle, SANE_Int index)
{
  const SANE_Option_Descriptor *desc;

  desc = API_ENTRY (BACKEND_NAME, get_option_descriptor) (handle, index);
  return desc;
}

SANE_Status
sane_control_option (SANE_Handle handle, SANE_Int index, SANE_Action action,
                     void *value, SANE_Word *info)
{
  SANE_Status status;

  status = API_ENTRY (BACKEND_NAME, control_option) (handle, index, action,
                                                     value, info);
  return status;
}

SANE_Status
sane_get_parameters (SANE_Handle handle, SANE_Parameters *parameters)
{
  SANE_Status status;

  status = API_ENTRY (BACKEND_NAME, get_parameters) (handle, parameters);
  return status;
}


SANE_Status
sane_start (SANE_Handle handle)
{
  SANE_Status status;

  status = API_ENTRY (BACKEND_NAME, start) (handle);
  return status;
}

SANE_Status
sane_read (SANE_Handle handle, SANE_Byte *buffer, SANE_Int max_length,
           SANE_Int *length)
{
  SANE_Status status;

  status = API_ENTRY (BACKEND_NAME, read) (handle, buffer, max_length,
                                           length);
  return status;
}

void
sane_cancel (SANE_Handle handle)
{
  API_ENTRY (BACKEND_NAME, cancel) (handle);
  return;
}


SANE_Status
sane_set_io_mode (SANE_Handle handle, SANE_Bool non_blocking)
{
  SANE_Status status;

  status = API_ENTRY (BACKEND_NAME, set_io_mode) (handle, non_blocking);
  return status;
}

SANE_Status
sane_get_select_fd (SANE_Handle handle, SANE_Int *fdp)
{
  SANE_Status status;

  status = API_ENTRY (BACKEND_NAME, get_select_fd) (handle, fdp);
  return status;
}

SANE_String_Const
sane_strstatus (SANE_Status status)
{
  SANE_String_Const str;

  str = API_ENTRY (BACKEND_NAME, strstatus) (status);
  return str;
}

/*! \brief  Marks a string for the translation tools.
 */
#define N(string)       (string)

/*  Backends should not be bothered with implementing their own
 *  version of this bit of SANE API.  At best, all will do the same
 *  thing, at worst every one has its own implementation returning
 *  different strings for the same status.  This implementation will
 *  be used by every backend without them even noticing.
 */
SANE_String_Const
API_ENTRY (BACKEND_NAME, strstatus) (SANE_Status status)
{
  switch (status)
    {
    case SANE_STATUS_GOOD:
      return N("Success");
    case SANE_STATUS_UNSUPPORTED:
      return N("Operation not supported");
    case SANE_STATUS_CANCELLED:
      return N("Operation was cancelled");
    case SANE_STATUS_DEVICE_BUSY:
      return N("Device busy");
    case SANE_STATUS_INVAL:
      return N("Invalid argument");
    case SANE_STATUS_EOF:
      return N("End of file reached");
    case SANE_STATUS_JAMMED:
      return N("Document feeder jammed");
    case SANE_STATUS_NO_DOCS:
      return N("Document feeder out of documents");
    case SANE_STATUS_COVER_OPEN:
      return N("Scanner cover is open");
    case SANE_STATUS_IO_ERROR:
      return N("Error during device I/O");
    case SANE_STATUS_NO_MEM:
      return N("Out of memory");
    case SANE_STATUS_ACCESS_DENIED:
      return N("Access to resource has been denied");

    default:
      {
        static char msg[80];    /* not re-entrant! */

        snprintf (msg, 80, N("Unknown status code (%d)"), status);
        return msg;
      }
    }
}

#undef N                        /* no longer needed, clean up */


#endif  /* !defined (BACKEND_CREATE_FORWARDERS) */


/*  Use the preprocessor to rename API entries in the backend to match
 *  the "shadow" API entries we have set up above.
 *  This way, the implementer can ignore all these name playing games.
 */

#define sane_init       API_ENTRY (BACKEND_NAME, init)
#define sane_exit       API_ENTRY (BACKEND_NAME, exit)
#define sane_get_devices \
                        API_ENTRY (BACKEND_NAME, get_devices)
#define sane_open       API_ENTRY (BACKEND_NAME, open)
#define sane_close      API_ENTRY (BACKEND_NAME, close)
#define sane_get_option_descriptor \
                        API_ENTRY (BACKEND_NAME, get_option_descriptor)
#define sane_control_option \
                        API_ENTRY (BACKEND_NAME, control_option)
#define sane_get_parameters \
                        API_ENTRY (BACKEND_NAME, get_parameters)
#define sane_start      API_ENTRY (BACKEND_NAME, start)
#define sane_read       API_ENTRY (BACKEND_NAME, read)
#define sane_cancel     API_ENTRY (BACKEND_NAME, cancel)
#define sane_set_io_mode \
                        API_ENTRY (BACKEND_NAME, set_io_mode)
#define sane_get_select_fd \
                        API_ENTRY (BACKEND_NAME, get_select_fd)


#ifdef __cplusplus
}       /* extern "C" */
#endif

#endif  /* !defined (included_backend_h) */
