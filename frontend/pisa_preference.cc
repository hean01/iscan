/* 
   SANE EPSON backend
   Copyright (C) 2001 SEIKO EPSON CORPORATION

   Date		Author		Reason
   06/13/2001	N.Sasaki	New

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
#include <errno.h>
#include "pisa_preference.h"

static char * get_line ( FILE * fp, char ** gotstr );
static char * dynamic_fgets ( FILE * fp );
static int parse_line ( char * ptr, cfg_struct * cfg );
static char * parse_word ( char * ptr, char ** word, cfg_key_type key );
static int store_value ( cfg_struct * cfg, char * parameter, char * value );
static char * remove_first_spaces ( char * ptr );
static void set_word ( FILE * fp, cfg_struct * cfg );
static void single_or_double_quote ( char * str, char * ret );

/*----------------------------------------------------------*/
void get_cfg ( char * file, cfg_struct * cfg, int num )
{
  FILE	* fp;
  char * ptr;
  char * line_buf;
  int i;

  if ( 0 == ( fp = fopen ( file, "r" ) ) )
    return;

  for ( i = 0; i < num; i++, cfg++ )
    {
      rewind ( fp );

      while ( 0 != ( ptr = get_line ( fp, & line_buf ) ) )
	{
	  if ( parse_line ( ptr, cfg ) )
	    {
	      free ( line_buf );
	      break;
	    }

	  free ( line_buf );
	}
    }

  fclose ( fp );
}

/*----------------------------------------------------------*/
void set_cfg ( char * file, cfg_struct * cfg, int num )
{
  FILE	* fp;
  int	i;

  if ( 0 == ( fp = fopen ( file, "w" ) ) )
    return;

  for ( i = 0; i < num; i++, cfg++ )
    set_word ( fp, cfg );

  fclose ( fp );
}

/*----------------------------------------------------------*/
static char * get_line ( FILE * fp, char ** gotstr )
{
  char * ptr;

  while ( 1 )
    {
      if ( 0 == ( * gotstr = dynamic_fgets ( fp ) ) )
	return 0;

      ptr = remove_first_spaces ( * gotstr );
      if ( * ptr != '#' && * ptr != '\0' )
	return ptr;

      free ( * gotstr );
    }
}

/*----------------------------------------------------------*/
static char * dynamic_fgets ( FILE * fp )
{
  char	* ptr;
  char	tmp [ 128 ];
  int	i;

  if ( 0 == ( ptr = ( char * ) malloc ( 1 ) ) )
    return 0;

  * ptr = '\0';

  for ( i = 0;; i++ )
    {
      if ( 0 == fgets ( tmp, 128, fp ) )
	{
	  free ( ptr );
	  return 0;
	}

      if ( 0 == ( ptr = ( char * ) realloc ( ptr, 127 * ( i + 1 ) + 1 ) ) )
	return 0;
      
      strcat ( ptr, tmp );

      if ( 0 != strchr ( tmp, '\n' ) )
	{
	  * strchr ( ptr, '\n' ) = '\0';
	  return ptr;
	}

      if ( 0 != feof ( fp ) )
	return ptr;
    }
}

/*----------------------------------------------------------*/
static char * remove_first_spaces ( char * ptr )
{
  while ( * ptr == ' ' || * ptr == '\t' )
    ptr++;

  return ptr;
}

/*----------------------------------------------------------*/
static int parse_line ( char * ptr, cfg_struct * cfg )
{
  char * parameter;
  char * value;

  if ( 0 == ( ptr = parse_word ( ptr, & parameter, CFG_PARAM ) ) )
    return 0;

  if ( 0 == ( ptr = parse_word ( ptr, & value, CFG_VALUE ) ) )
    return 0;
  
  if ( 0 == store_value ( cfg, parameter, value ) )
    return 0;
  
  free ( parameter );
  free ( value );

  return 1;
}

/*----------------------------------------------------------*/
static char * parse_word ( char * ptr, char ** word, cfg_key_type key )
{
  int			len = 0;
  cfg_quote_type	quote;

  switch ( * ptr )
    {
    case '\"':
      quote = CFG_QUOTE_DOUBLE;
      ptr++;
      break;
    case '\'':
      quote = CFG_QUOTE_SINGLE;
      ptr++;
      break;
    default:
      quote = CFG_QUOTE_NO;
      break;
    }

  while ( 1 )
    {
      if ( quote == CFG_QUOTE_NO )
	{
	  if ( * ( ptr + len ) == ' ' || * ( ptr + len ) == '\t' ||
	       * ( ptr + len ) == '\0' || * ( ptr + len ) == '#' ||
	       ( * ( ptr + len ) == '=' && key == CFG_PARAM ) )
	    break;
	}
      else if ( quote == CFG_QUOTE_DOUBLE )
	{
	  if ( * ( ptr + len ) == '\"' )
	    break;
	}
      else if ( quote == CFG_QUOTE_SINGLE )
	{
	  if ( * ( ptr + len ) == '\'' )
	    break;
	}

      if ( * ( ptr + len ) == '\0' )
	return 0;

      len++;
    }

  if ( 0 == ( * word = ( char * ) malloc ( len + 1 ) ) )
    return 0;

  strncpy ( * word, ptr, len );
  * ( * word + len ) = '\0';

  ptr += ( len + ( quote == CFG_QUOTE_NO ? 0 : 1 ) );

  ptr = remove_first_spaces ( ptr );

  switch ( key )
    {
    case CFG_PARAM:
      if ( * ptr != '=' )
	return 0;
      ptr++;
      ptr = remove_first_spaces ( ptr );
      break;
    case CFG_VALUE:
      if ( * ptr != '\0' && * ptr != '#' )
	return 0;
      break;
    }

  return ptr;
}

/*----------------------------------------------------------*/
static int store_value ( cfg_struct * cfg, char * parameter, char * value )
{
  long tmp, utmp;
  double dtmp;
  char * endptr;

  if ( 0 != strcasecmp ( parameter, cfg->name ) )
    return 0;

  errno = 0;

  switch ( cfg->type )
    {
    case CFG_BOOL:
      if ( 0 == strcasecmp ( value, "TRUE" ) ||
	   0 == strcasecmp ( value, "YES" ) ||
	   0 == strcasecmp ( value, "T" ) ||
	   0 == strcasecmp ( value, "Y" ) ||
	   0 == strcasecmp ( value, "1" ) )
	* ( int * ) ( cfg->value ) = 1;
      else
	* ( int * ) ( cfg->value ) = 0;
      return 1;
    case CFG_LONG:
      tmp = strtol ( value, & endptr, 10 );
      if ( * endptr )
	return 0;
      if ( errno == ERANGE )
	return 0;
      * ( long * ) ( cfg->value ) = tmp;
      return 1;
    case CFG_ULONG:
      utmp = strtoul ( value, & endptr, 10 );
      if ( * endptr )
	return 0;
      if ( errno == ERANGE )
	return 0;
      * ( unsigned long * ) ( cfg->value ) = utmp;
      return 1;
    case CFG_DOUBLE:
      dtmp = strtod ( value, & endptr );
      if ( * endptr )
	return 0;
      if ( errno == ERANGE )
	return 0;
      * ( double * ) ( cfg->value ) = dtmp;
      return 1;
    case CFG_STRING:
      strcpy ( ( char * ) cfg->value, value );
      return 1;
    default:
      return 0;
    }

  return 0;
}

/*----------------------------------------------------------*/
static void set_word ( FILE * fp, cfg_struct * cfg )
{
  char c [ 2 ];

  switch ( cfg->type )
    {
    case CFG_BOOL:
      fprintf ( fp, "%s\t= %s\n", cfg->name,
		( * ( int * ) ( cfg->value ) ) ? "true" : "false" );
      break;
    case CFG_LONG:
      fprintf ( fp, "%s\t= %ld\n", cfg->name, * ( long * ) ( cfg->value ) );
      break;
    case CFG_ULONG:
      fprintf ( fp, "%s\t= %lu\n", cfg->name,
		* ( unsigned long * ) ( cfg->value ) );
      break;
    case CFG_DOUBLE:
      fprintf ( fp, "%s\t= %f\n", cfg->name, * ( double * ) ( cfg->value ) );
      break;
    case CFG_STRING:
      single_or_double_quote (  ( char * ) cfg->value, c );
      fprintf ( fp, "%s\t= %s%s%s\n", cfg->name,
		c, ( char * ) cfg->value, c );
      break;
    }
}

/*----------------------------------------------------------*/
static void single_or_double_quote ( char * str, char * ret )
{
  ret [ 1 ] = '\0';

  if ( 0 != strchr ( str, '\"' ) )
    ret [ 0 ] = '\'';
  else if ( 0 != strchr ( str, '\'' ) ||
	    0 != strchr ( str, '#' ) ||
	    0 != strchr ( str, '\t' ) ||
	    0 != strchr ( str, ' ' ) )
    ret [ 0 ] = '\"';
  else
    ret [ 0 ] = '\0';
}

