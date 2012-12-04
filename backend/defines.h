/*  defines.h -- a grab bag of preprocessor utilities
 *  Copyright (C) 2008  SEIKO EPSON CORPORATION
 *
 *  License: AVASYS PUBLIC LICENSE
 *  Author : AVASYS CORPORATION
 *
 *  This file is part of Image Scan! for Linux.
 *  It is distributed under the terms of the AVASYS PUBLIC LICENSE.
 *
 *  You should have received a verbatim copy of the AVASYS PUBLIC
 *  LICENSE along with the software.
 */


#ifndef defines_h
#define defines_h

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/* Assorted constants.
 */

#define MM_PER_INCH     25.4    /* exactly */


#ifndef __cplusplus
/*! A C++ Boolean type and corresponding keywords for our C code.
 */
typedef enum {
  false = 0,
  true
} bool;
#endif


/*  Run-time contract validation.
 */
#include <stdlib.h>
#include "message.h"

#define _assert(type,condition)                                 \
  if (!(condition))                                             \
  {                                                             \
    err_fatal ("failed: %s (%s)", type, #condition);            \
    exit (EXIT_FAILURE);                                        \
  }
#define require(condition)      _assert ("require", condition)
#define promise(condition)      _assert ("promise", condition)


/*  "Typed" memory allocation convenience wrappers.
 *  These are meant to make the invocations consistent and take care of
 *  the casting for you.
 */
#include <alloca.h>
#include <stdlib.h>

#define t_alloca(sz,t)          ((t *) alloca ((sz) * sizeof (t)))
#define t_calloc(sz,t)          ((t *) calloc ((sz) , sizeof (t)))
#define t_malloc(sz,t)          ((t *) malloc ((sz) * sizeof (t)))
#define t_realloc(p,sz,t)       ((t *) realloc ((p), (sz) * sizeof (t)))

/*  Compute sizes of _statically_ allocated arrays easily.
 */
#define num_of(p)               (sizeof (p) / sizeof (*p))

/*  Safely release acquired resources.
 *  The const_delete() is meant for those rare cases where you need to
 *  clean up const t* typed memory areas.
 */
#define delete(p)               do { if (p) free (p); p = 0; } while (0)
#define const_delete(p,t)       do { if (p) free ((t) p); p = 0; } while (0)


/* Portable path and file name component separators.
 */
#ifdef __unix
#define PATH_SEP_STR ":"
#define PATH_SEP PATH_SEP_STR[0]
#define FILE_SEP_STR "/"
#define FILE_SEP FILE_SEP_STR[0]
#else
#define PATH_SEP_STR ";"
#define PATH_SEP PATH_SEP_STR[0]
#define FILE_SEP_STR "\\"
#define FILE_SEP FILE_SEP_STR[0]
#endif


  typedef unsigned char byte;


#ifdef __cplusplus
}       /* extern "C" */
#endif

#endif  /* !defined (defines_h) */
