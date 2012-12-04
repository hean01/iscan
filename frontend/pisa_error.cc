/* 
   SANE EPSON backend
   Copyright (C) 2001, 2009  SEIKO EPSON CORPORATION

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

#include <config.h>

#include "gettext.h"
#define  _(msg_id)	gettext (msg_id)

/*------------------------------------------------------------*/
#include "pisa_error.h"

pisa_error::pisa_error( pisa_error_id status )
  : m_id( status )
{
}

pisa_error::pisa_error( SANE_Status status )
  : m_id( pisa_error_id( status | 0xff00 ) )
{
}

enum                            // FIXME: needs sync with epkowa.c!
  {
    EXT_SANE_STATUS_NONE,
    EXT_SANE_STATUS_MULTI_FEED,
    EXT_SANE_STATUS_TRAY_CLOSED,
    EXT_SANE_STATUS_MAX,
  };

pisa_error::pisa_error (SANE_Status status, const sane_scan& ss)
  : m_id (pisa_error_id (status | 0xff00))
{
  SANE_Word value = 0;

  switch (m_id)
    {
    case PISA_STATUS_JAMMED:
      ss.get_value ("ext-sane-status", static_cast<void *> (&value), true);
      if (EXT_SANE_STATUS_MULTI_FEED == value)
        m_id = PISA_ERR_MULTI_FEED;
      break;
    case PISA_STATUS_COVER_OPEN:
      ss.get_value ("ext-sane-status", static_cast<void *> (&value), true);
      if (EXT_SANE_STATUS_TRAY_CLOSED == value)
        m_id = PISA_ERR_TRAY_CLOSED;
      break;
    default:
      ;
    }
}

const char * pisa_error::get_error_string ( void ) const
{
  switch (remap())
    {
      // Let's get the SANE status IDs out of the way first.
    case PISA_STATUS_GOOD:
      return _("Operation completed succesfully.");
    case PISA_STATUS_UNSUPPORTED:
      return _("Operation is not supported.");
    case PISA_STATUS_CANCELLED:
      return _("Operation was cancelled.");
    case PISA_STATUS_DEVICE_BUSY:
      return _("Device is busy---retry later.");
    case PISA_STATUS_INVAL:
      return _("Data or argument is invalid.");
    case PISA_STATUS_EOF:
      return _("No more data available (end-of-file).");
    case PISA_STATUS_JAMMED:
      return _("A paper jam occured.  "
	       "Open the Automatic Document Feeder and remove any paper.");
    case PISA_STATUS_NO_DOCS:
      return _("Please load the document(s) into the Automatic Document "
	       "Feeder.");
    case PISA_STATUS_COVER_OPEN:
      return _("The automatic document feeder or scanner unit is open.\n"
	       "Please close it.");
    case PISA_STATUS_IO_ERROR:
      return _("Error during device I/O.");
    case PISA_STATUS_NO_MEM:
      return _("Out of memory.");
    case PISA_STATUS_ACCESS_DENIED:
      return _("Access to resource has been denied.");
      // Now we add our own.
    case PISA_ERR_OUTOFMEMORY:
      return _( "There is not enough disk space for operation" );

    case PISA_ERR_CONNECT:
      return _( "Could not send command to scanner.\n"
		"Check the scanner's status." );

    case PISA_ERR_UNSUPPORT:
      return _( "Scanner model not supported" );

    case PISA_ERR_AREALARGE:
      return _( "Selected area is too large for this resolution.\n"
		"Reduce the selected area or resolution." );

    case PISA_ERR_FILENAME:
      return _( "Could not create file" );

    case PISA_ERR_FILEOPEN:
      return _( "Could not create file" );

    case PISA_ERR_OVERWRITE:
      return _( "A file with the same name already exists.\n"
                "Click \"Overwrite\" to replace the file or "
                "\"Cancel\" if you want to use another file name.");

    case PISA_ERR_MRRESTOOHIGH:
      return _( "The Image Type setting you selected cannot be used "
		"with this resolution.\n"
		"Reduce the Resolution or Scale setting." );

    case PISA_ERR_TRAY_CLOSED:
      return _("Tray cover is closed.  Please open the tray cover and "
               "then scan again.");

    case PISA_ERR_MULTI_FEED:
      return _("A multi page feed occurred in the auto document feeder.\n"
               "Open the cover, remove the documents, and then try again.  "
               "If documents remain on the tray, remove them and then reload "
               "them.");

    default:
      break;
    }

  return _( "Unexpected error occurred" );
}

pisa_error_id
pisa_error::remap() const
{
  switch (m_id)
    {
      // do what pase_sane_scan.cc used (yuck!) to do, except for
      // statuses that we really need to special case
    case PISA_STATUS_UNSUPPORTED:
    case PISA_STATUS_DEVICE_BUSY:
    case PISA_STATUS_INVAL:
    case PISA_STATUS_EOF:
    case PISA_STATUS_IO_ERROR:
    case PISA_STATUS_NO_MEM:
    case PISA_STATUS_ACCESS_DENIED:
      return PISA_ERR_CONNECT;
    default:
      return m_id;
    }
  return PISA_ERR_INVALID_ERROR_ID;
}
