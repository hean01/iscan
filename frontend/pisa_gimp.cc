/* 
   SANE EPSON backend
   Copyright (C) 2001, 2005, 2009 SEIKO EPSON CORPORATION

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

#include <gtk/gtk.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dlfcn.h>

#include "pisa_main.h"
#include "pisa_gimp.h"
#include "pisa_error.h"
#include "pisa_enums.h"

#ifdef HAVE_ANY_GIMP
/*----------------------------------------------------------*/
static void * handle_libgimp = 0;
int g_gimpversion = 1;

typedef gchar* lib_gimp_version (void);
typedef gint lib_gimp_main ( gint argc, gchar * argv [ ] );
typedef void lib_gimp_quit ( void );
typedef void lib_gimp_install_procedure ( gchar * name,
					  gchar * blurb,
					  gchar * help,
					  gchar * author,
					  gchar * copyright,
					  gchar * date,
					  gchar * menu_path,
					  gchar * image_types,
					  gint    type,
					  gint    nparams,
					  gint    nreturn_vals,
					  GimpParamDef * params,
					  GimpParamDef * return_vals );
typedef guint lib_gimp_tile_height ( void );
typedef gint32 lib_gimp_image_new ( gint width,
				    gint height,
				    GimpImageBaseType type );
typedef gint32 lib_gimp_layer_new ( gint32 image_ID,
				    gchar * name,
				    gint width,
				    gint height,
				    GimpImageType type,
				    gdouble opacity,
				    GimpLayerModeEffects mode );
typedef gboolean lib_gimp_image_add_layer ( gint32 image_ID,
					    gint32 layer_ID,
					    gint position );
typedef GimpDrawable * lib_gimp_drawable_get ( gint32 drawable_ID );
typedef void lib_gimp_pixel_rgn_init (GimpPixelRgn  *pr,
				      GimpDrawable  *drawable,
				      gint           x,
				      gint           y,
				      gint           width,
				      gint           height,
				      gint           dirty,
				      gint           shadow);
typedef void lib_gimp_pixel_rgn_set_rect (GimpPixelRgn  *pr,
					  guchar        *buf,
					  gint           x,
					  gint           y,
					  gint           width,
					  gint           height );
typedef gboolean lib_gimp_image_remove_layer (gint32 image_ID,
					      gint32 layer_ID );
typedef gboolean lib_gimp_image_delete ( gint32 image_ID );
typedef void lib_gimp_drawable_flush ( GimpDrawable *drawable );
typedef void lib_gimp_drawable_detach ( GimpDrawable *drawable );
typedef gint32 lib_gimp_display_new ( gint32 image_ID );
typedef gchar * lib_gimp_gtkrc ( void );
typedef gboolean lib_gimp_use_xshm ( void );

// Define function prototype for plug-in gimp-2
typedef void lib_gimp_extension_ack_2 ( void );
typedef void lib_gimp_install_procedure_2 (const  gchar * name,
					  const gchar * blurb,
					  const gchar * help,
					  const gchar * author,
					  const gchar * copyright,
					  const gchar * date,
					  const char * menu_path,
					  const gchar * image_types,
					  GimpPDBProcType type,
					  gint    nparams,
					  gint    nreturn_vals,
					  const GimpParamDef * params,
					  const GimpParamDef * return_vals );

#ifdef HAVE_GIMP_2
typedef gint lib_gimp_main_2 (  const GimpPlugInInfo *info,
			     gint argc, gchar * argv [ ] );	
#else
typedef gint lib_gimp_main_2 (  const GimpPlugInInfo_2 *info,
			     gint argc, gchar * argv [ ] );	
#endif 

lib_gimp_version * plib_gimp_version;
lib_gimp_main * plib_gimp_main;
lib_gimp_quit * plib_gimp_quit;
lib_gimp_install_procedure * plib_gimp_install_procedure;
lib_gimp_tile_height * plib_gimp_tile_height;
lib_gimp_image_new * plib_gimp_image_new;
lib_gimp_layer_new * plib_gimp_layer_new;
lib_gimp_image_add_layer * plib_gimp_image_add_layer;
lib_gimp_drawable_get * plib_gimp_drawable_get;
lib_gimp_pixel_rgn_init * plib_gimp_pixel_rgn_init;
lib_gimp_pixel_rgn_set_rect * plib_gimp_pixel_rgn_set_rect;
lib_gimp_image_remove_layer * plib_gimp_image_remove_layer;
lib_gimp_image_delete * plib_gimp_image_delete;
lib_gimp_drawable_flush * plib_gimp_drawable_flush;
lib_gimp_drawable_detach * plib_gimp_drawable_detach;
lib_gimp_display_new * plib_gimp_display_new;
lib_gimp_gtkrc * plib_gimp_gtkrc;
lib_gimp_use_xshm * plib_gimp_use_xshm;

/* Use for plug-in gimp-2 */
lib_gimp_main_2 * plib_gimp_main_2;
lib_gimp_install_procedure_2 * plib_gimp_install_procedure_2;
lib_gimp_extension_ack_2* plib_gimp_extension_ack_2; 


static void query ( void );
#ifdef HAVE_GIMP_2
static void run ( char * name, int nparams, GimpParam_1 * param,
		  int * nreturn_vals, GimpParam_1 ** return_vals );
#else
static void run ( char * name, int nparams, GimpParam * param,
		  int * nreturn_vals, GimpParam ** return_vals );
#endif // HAVE_GIMP_2

static void gimp2_query ( void );
#ifdef HAVE_GIMP_2
static void gimp2_run ( const gchar      *name,
 		        gint              n_params,
 			const GimpParam  *param,
			gint             *n_return_vals,
             		GimpParam       **return_vals);
#else
static void gimp2_run ( const gchar      *name,
 			gint              n_params,
 			const GimpParam_2  *param,
 			gint             *n_return_vals,
 			GimpParam_2       **return_vals);

#endif // HAVE_GIMP_2

/*----------------------------------------------------------*/
// Define these structures for supporting with gimp-1 and gimp-2 
#ifdef HAVE_GIMP_2
GimpPlugInInfo_1 PLUG_IN_INFO =
{
  0,
  0,
  query,
  run,
};
GimpPlugInInfo PLUG_IN_INFO_2 =
{
  0,
  0,
  gimp2_query,
  gimp2_run,
};
#else
GimpPlugInInfo PLUG_IN_INFO =
{
  0,		// init
  0,		// quit
  query,	// query
  run,		// run
};
GimpPlugInInfo_2 PLUG_IN_INFO_2 = {
  0,
  0, 
  gimp2_query,   
  gimp2_run,     
};
#endif // HAVE_GIMP_2

#endif // HAVE_ANY_GIMP

/*----------------------------------------------------------*/
gint pisa_gimp_main ( gint argc, gchar * argv [ ] )
{
#ifdef HAVE_ANY_GIMP
  int i, j, k;
  char libname [ 32 ];

  for( k = 1; k <= 2; k++){
    
    for ( i = 9; 0 <= i; i-- )
      {
	if ( handle_libgimp )
	  break;

	for ( j = 9; 0 <= j; j-- )
	  {
	    sprintf ( libname, "libgimp-%d.%d.so.%d", k, i, j );
	    handle_libgimp = dlopen ( libname, RTLD_LAZY );
	    if ( handle_libgimp ){
	      g_gimpversion = k;
	      break;
	    }
	  }
      }
  }
  if ( handle_libgimp == 0 )
    return 1;

  if ( g_gimpversion == 1){
    plib_gimp_main                   = ( lib_gimp_main * ) dlsym ( handle_libgimp, "gimp_main" );
    plib_gimp_install_procedure      = ( lib_gimp_install_procedure * ) dlsym ( handle_libgimp, "gimp_install_procedure" );
    
    // This function isn't supported in gimp-2
    plib_gimp_use_xshm               = ( lib_gimp_use_xshm * ) dlsym ( handle_libgimp, "gimp_use_xshm" );
    
    if ( ! plib_gimp_main ||  ! plib_gimp_install_procedure || ! plib_gimp_use_xshm) {
      return 1;
    }
    
  }
  else{
    // These functions are changed in gimp-2
    plib_gimp_main_2                 = ( lib_gimp_main_2 * ) dlsym ( handle_libgimp, "gimp_main" );
    plib_gimp_install_procedure_2    = ( lib_gimp_install_procedure_2 * ) dlsym ( handle_libgimp, "gimp_install_procedure" );
    
    // This is a new function in gimp-2
    plib_gimp_extension_ack_2        = ( lib_gimp_extension_ack_2 *) dlsym(handle_libgimp, "gimp_extension_ack")  ;
    
    if ( ! plib_gimp_main_2 ||  ! plib_gimp_install_procedure_2 || ! plib_gimp_extension_ack_2 ) {
      return 1;
    }
  }

  plib_gimp_version                = ( lib_gimp_version * ) dlsym ( handle_libgimp, "gimp_version" );
  plib_gimp_quit                   = ( lib_gimp_quit * ) dlsym ( handle_libgimp, "gimp_quit" );
  plib_gimp_tile_height            = ( lib_gimp_tile_height * ) dlsym ( handle_libgimp, "gimp_tile_height" );
  plib_gimp_image_new              = ( lib_gimp_image_new * ) dlsym ( handle_libgimp, "gimp_image_new" );
  plib_gimp_layer_new              = ( lib_gimp_layer_new * ) dlsym ( handle_libgimp, "gimp_layer_new" );
  plib_gimp_image_add_layer        = ( lib_gimp_image_add_layer * ) dlsym ( handle_libgimp, "gimp_image_add_layer" );
  plib_gimp_drawable_get           = ( lib_gimp_drawable_get * ) dlsym ( handle_libgimp, "gimp_drawable_get" );
  plib_gimp_pixel_rgn_init         = ( lib_gimp_pixel_rgn_init * ) dlsym ( handle_libgimp, "gimp_pixel_rgn_init" );
  plib_gimp_pixel_rgn_set_rect     = ( lib_gimp_pixel_rgn_set_rect * ) dlsym ( handle_libgimp, "gimp_pixel_rgn_set_rect" );
  plib_gimp_image_remove_layer     = ( lib_gimp_image_remove_layer * ) dlsym ( handle_libgimp, "gimp_image_remove_layer" );
  plib_gimp_image_delete           = ( lib_gimp_image_delete * ) dlsym ( handle_libgimp, "gimp_image_delete" );
  plib_gimp_drawable_flush         = ( lib_gimp_drawable_flush * ) dlsym ( handle_libgimp, "gimp_drawable_flush" );
  plib_gimp_drawable_detach        = ( lib_gimp_drawable_detach * ) dlsym ( handle_libgimp, "gimp_drawable_detach" );
  plib_gimp_display_new            = ( lib_gimp_display_new * ) dlsym ( handle_libgimp, "gimp_display_new" );
  plib_gimp_gtkrc                  = ( lib_gimp_gtkrc * ) dlsym ( handle_libgimp, "gimp_gtkrc" );

  if ( ! plib_gimp_quit || ! plib_gimp_tile_height || ! plib_gimp_image_new || ! plib_gimp_layer_new ||
       ! plib_gimp_image_add_layer || ! plib_gimp_drawable_get ||
       ! plib_gimp_pixel_rgn_init || ! plib_gimp_pixel_rgn_set_rect ||
       ! plib_gimp_image_remove_layer || ! plib_gimp_image_delete ||
       ! plib_gimp_drawable_flush || ! plib_gimp_drawable_detach ||
       ! plib_gimp_display_new || ! plib_gimp_gtkrc )      
    return 1;

  if (g_gimpversion == 1){
    return plib_gimp_main ( argc, argv );
  }
  else{
    return plib_gimp_main_2 (&PLUG_IN_INFO_2, argc, argv );
  }

#else
  return 1;
#endif // HAVE_ANY_GIMP
}

/*----------------------------------------------------------*/
void pisa_gimp_quit ( void )
{
#ifdef HAVE_ANY_GIMP
  plib_gimp_quit ( );
  dlclose ( handle_libgimp );
#endif
}

#ifdef HAVE_ANY_GIMP

/*----------------------------------------------------------*/
gchar * pisa_gimp_gtkrc ( void )
{
  return plib_gimp_gtkrc ( );
}

/*----------------------------------------------------------*/
gboolean pisa_gimp_use_xshm ( void )
{
  return plib_gimp_use_xshm ( );
}

/*----------------------------------------------------------*/
static void query ( void )
{
  static GimpParamDef args [ ] =
  {
    { GIMP_PDB_INT32, "run_mode", "Interactive, non-interactive" },
  };

  static GimpParamDef	* return_vals = 0;
  static int		nargs = sizeof ( args ) / sizeof ( args [ 0 ] );
  static int		nreturn_vals = 0;

  plib_gimp_install_procedure (
			      "Image Scan! for Linux",
			      "Front-end to the SANE interface",
			      "This function provides access to scanners "
			      "and other image acquisition devices through "
			      "the SANE (Scanner Access Now Easy) interface.",
			      "AVASYS CORPORATION",
			      "SEIKO EPSON CORPORATION",
			      "2001, 2005",
			      "<Toolbox>/Xtns/Acquire Image/Scanning (iscan)...",
			      "RGB, GRAY",
			      GIMP_EXTENSION,
			      nargs, nreturn_vals,
			      args, return_vals );
}
/*----------------------------------------------------------*/
static void gimp2_query ( void )
{
  static GimpParamDef args [ ] =
  {
    { GIMP_PDB_INT32, "run_mode", "Interactive, non-interactive" },
  };

  static GimpParamDef	* return_vals = 0;
  static int		nargs = sizeof ( args ) / sizeof ( args [ 0 ] );
  static int		nreturn_vals = 0;

  char* menu = "<Toolbox>/File/Acquire/Scanning (iscan)...";
  int maj, min = 0;
  if (!plib_gimp_version ||
      2 != sscanf (plib_gimp_version (), "%d.%d.", &maj, &min))
    {
      maj = 2;
      min = 0;
    }
  if (2 < maj || (2 == maj && 6 <= min))
    {
      // accommodate new menu structure introduced in gimp 2.6
      menu = "<Image>/File/Create/Acquire/Scanning (iscan)...";
    }

  // Must call this function for supporting gimp-2 
  plib_gimp_install_procedure_2 (
			      "Image Scan! for Linux",
			      "Front-end to the SANE interface",
			      "This function provides access to scanners "
			      "and other image acquisition devices through "
			      "the SANE (Scanner Access Now Easy) interface.",
			      "AVASYS CORPORATION",
			      "SEIKO EPSON CORPORATION",
			      "2009",
			      menu,
			      "",
			      GIMP_EXTENSION,
			      nargs, nreturn_vals,
			      args, return_vals );
}

/*----------------------------------------------------------*/

#ifdef HAVE_GIMP_2
static void run ( char * name, int nparams, GimpParam_1 * param,
		  int * nreturn_vals, GimpParam_1 ** return_vals )
#else
static void run ( char * name, int nparams, GimpParam * param,
		  int * nreturn_vals, GimpParam ** return_vals )

#endif // HAVE_GIMP_2
{
#ifdef HAVE_GIMP_2
  static GimpParam_1	values [ 1 ];
  GimpRunMode   	run_mode;
#else
  static GimpParam	values [ 1 ];
  GimpRunModeType	run_mode;
#endif // HAVE_GIMP_2
  char			* argv [ 1 ];
  int			argc = 1;

  name = name;
  nparams = nparams;

#ifdef HAVE_GIMP_2
  run_mode = ( GimpRunMode ) param [ 0 ].data.d_int32;
#else 
  run_mode = ( GimpRunModeType ) param [ 0 ].data.d_int32;
#endif

  * nreturn_vals = 1;
  * return_vals	 = values;

  values [ 0 ].type		= GIMP_PDB_STATUS;
  values [ 0 ].data.d_status	= GIMP_PDB_SUCCESS;

  argv [ 0 ] = "iscan";

  switch ( run_mode )
    {
    case GIMP_RUN_INTERACTIVE:
      pisa_start ( argc, argv, 1);
      break;

    default:
      break;
    }

  return;
}

#ifdef HAVE_GIMP_2
static void gimp2_run (   const gchar      *name,
			  gint              n_params,
			  const GimpParam  *param,
			  gint             *n_return_vals,
			  GimpParam       **return_vals)
#else
static void gimp2_run (   const gchar      *name,
			  gint              n_params,
			  const GimpParam_2  *param,
			  gint             *n_return_vals,
		          GimpParam_2       **return_vals)

#endif// HAVE_GIMP_2
{
#ifdef HAVE_GIMP_2
  static GimpParam	values [ 1 ];
  GimpRunMode    	run_mode;
#else
  static GimpParam_2	values [ 1 ];
	GimpRunModeType 	run_mode;
#endif// HAVE_GIMP_2
  char			* argv [ 1 ];
  int			argc = 1;

  name = name;
  n_params = n_params;

#ifdef HAVE_GIMP_2
  run_mode = ( GimpRunMode ) param [ 0 ].data.d_int32;
#else
  run_mode = ( GimpRunModeType ) param [ 0 ].data.d_int32;
#endif  // HAVE_GIMP_2

  * n_return_vals = 1;
  * return_vals	 = values;

  values [ 0 ].type		= GIMP_PDB_STATUS;
  values [ 0 ].data.d_status	= GIMP_PDB_SUCCESS;

  argv [ 0 ] = "iscan";

  switch ( run_mode )
    {
    case GIMP_RUN_INTERACTIVE:
      // This procedure must be called with GIMP_EXTENSION
      plib_gimp_extension_ack_2();
      pisa_start ( argc, argv, 2);
      break;
    default:
      break;
    }

  return;
}

/*----------------------------------------------------------*/
int gimp_scan::create_gimp_image ( int width,
				   int height,
				   int pixeltype,
				   int depth )
{
  size_t		tile_size;
  GimpImageBaseType	image_type;
  GimpImageType		drawable_type;

  // initialize member variable
  m_rows	= 0;
  m_width	= width;
  m_height	= height;
  m_pixeltype	= pixeltype;
  m_depth	= depth;
  m_rowbytes	= 0;

  m_tile	= 0;
  m_drawable	= 0;

  tile_size = width * ::plib_gimp_tile_height ( );

  if ( pixeltype == PISA_PT_RGB )
    {
      tile_size		*= 3;
      image_type	= GIMP_RGB;
      drawable_type	= GIMP_RGB_IMAGE;
      m_rowbytes	= width * 3;
    }
  else if ( pixeltype == PISA_PT_GRAY || pixeltype == PISA_PT_BW )
    {
      image_type	= GIMP_GRAY;
      drawable_type	= GIMP_GRAY_IMAGE;
      m_rowbytes	= width;
    }
  else
    return PISA_ERR_PARAMETER;

  m_image_id = ::plib_gimp_image_new ( m_width, m_height, image_type );
  
  m_layer_id = ::plib_gimp_layer_new ( m_image_id, "Background",
				  m_width, m_height, drawable_type,
				  100.0, GIMP_NORMAL_MODE );
  ::plib_gimp_image_add_layer ( m_image_id, m_layer_id, 0 );

  m_drawable = ::plib_gimp_drawable_get ( m_layer_id );

  ::plib_gimp_pixel_rgn_init ( & m_region, m_drawable, 0, 0,
			      m_drawable->width, m_drawable->height,
			      TRUE, FALSE );

  m_tile = g_new ( guchar, tile_size );

  if ( m_tile == 0 )
    return PISA_ERR_OUTOFMEMORY;

  return PISA_ERR_SUCCESS;
} 

/*----------------------------------------------------------*/
unsigned char * gimp_scan::get_next_buf ( void )
{
  int	tile_height = ::plib_gimp_tile_height ( );

  return & m_tile [ ( m_rows % tile_height ) * m_rowbytes ];
}

/*----------------------------------------------------------*/
int gimp_scan::set_image_rect ( void )
{
  int	tile_height = ::plib_gimp_tile_height ( );

  m_rows++;

  if ( m_rows % tile_height == 0 )
    {
      if ( m_depth == 1 )
	bw2gray ( );
      
      ::plib_gimp_pixel_rgn_set_rect ( & m_region, m_tile,
				      0, m_rows - tile_height,
				      m_width, tile_height );
    }
  
  return PISA_ERR_SUCCESS;
}

/*----------------------------------------------------------*/
int gimp_scan::finish_scan ( int cancel )
{
  int	remaining;

  if ( cancel )
    {
      ::plib_gimp_image_remove_layer ( m_image_id, m_layer_id );
      ::plib_gimp_image_delete ( m_image_id );
      ::free ( m_tile );
    }
  else
    {
      remaining = m_rows % ::plib_gimp_tile_height ( );
      
      if ( remaining )
	{
	  if ( m_depth == 1 )
	    bw2gray ( );
	  
	  ::plib_gimp_pixel_rgn_set_rect ( & m_region, m_tile,
					  0, m_rows - remaining,
					  m_width, remaining );
	}
      
      ::plib_gimp_drawable_flush ( m_drawable );
      ::plib_gimp_display_new ( m_image_id );
      ::plib_gimp_drawable_detach ( m_drawable );
      ::free ( m_tile );
    }
  
  return PISA_ERR_SUCCESS;
}

/*----------------------------------------------------------*/
int gimp_scan::bw2gray ( void )
{
  unsigned char	* tmp;
  unsigned char * bw_buf;
  long		bw_buf_size;
  long		i, j, k, tile_height;

  tile_height = ::plib_gimp_tile_height ( );
  bw_buf_size = ( ( m_width + 7 ) / 8 );

  bw_buf = new unsigned char [ bw_buf_size ];

  if ( ! bw_buf )
    return PISA_ERR_OUTOFMEMORY;

  for ( i = 0; i < tile_height; i++ )
    {
      ::memcpy ( bw_buf, m_tile + m_width * i, bw_buf_size );
      
      tmp = bw_buf;
      
      for ( j = 0; j < m_width; j += 8 )
	{
	  for ( k = 7; k >= 0; k-- )
	    {
	      if ( j + 7 - k < m_width )
		{
		  m_tile [ i * m_width + j + ( 7 - k ) ] = ( * tmp & ( 1 << k ) ) ?
		    0x00 : 0xff;
		}
	    }
	  tmp++;
	}
    }
  
  delete [ ] bw_buf;
  
  return PISA_ERR_SUCCESS;
}

#endif // HAVE_ANY_GIMP


