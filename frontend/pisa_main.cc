/* 
   SANE EPSON backend
   Copyright (C) 2001 SEIKO EPSON CORPORATION

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
#include <stdio.h>
#include <stdlib.h>
#include <libintl.h>
#include <locale.h>

#include "pisa_main.h"
#include "pisa_gimp.h"
#include "pisa_view_manager.h"
#include "pisa_error.h"

int g_gimp_plugin = 1;

/*----------------------------------------------------------*/
int main ( int argc, char * argv [ ] )
{
  int	result;

  setlocale ( LC_ALL, "" );
  bindtextdomain ( PACKAGE, LOCALEDIR );
#ifdef HAVE_GTK_2
  bind_textdomain_codeset (PACKAGE, "UTF-8");
#endif
  textdomain ( PACKAGE );

  if ( 1 < argc )
    result = pisa_gimp_main ( argc, argv );
  else
    result = 1;

  if ( result )
    {
      g_gimp_plugin = 0;
      pisa_start ( argc, argv );
    }

  exit ( EXIT_SUCCESS );
}

/*----------------------------------------------------------*/
int pisa_gimp_plugin ( void )
{
  return g_gimp_plugin;
}

/*----------------------------------------------------------*/
void pisa_start ( int argc, char * argv [ ] , int gimpversion) 
{
  if ( PISA_ERR_SUCCESS != view_manager::create_view_manager ( argc, argv ) )
    {
      if ( g_gimp_plugin )
	pisa_gimp_quit ( );
      else
	exit ( EXIT_FAILURE );
    }
  
  if ( g_gimp_plugin )
    {
#ifdef HAVE_ANY_GIMP
      gtk_rc_parse ( pisa_gimp_gtkrc ( ) );
      if (gimpversion == 1)
	{
	  gdk_set_use_xshm ( pisa_gimp_use_xshm ( ) );
	}
      else
	{
	  gdk_set_use_xshm ( TRUE );
	}
#endif	// HAVE_ANY_GIMP
    }

  g_view_manager->create_window ( ID_WINDOW_MAIN  );
  g_view_manager->main ( );
}

/*----------------------------------------------------------*/
void pisa_quit ( void )
{
  g_view_manager->release_view_manager ( );

  if ( g_gimp_plugin )
    ::pisa_gimp_quit ( );

  exit ( EXIT_SUCCESS );
}

