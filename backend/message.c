/*  message.c -- consistent error, progress and debugging feedback
 *  Copyright (C) 2008, 2009, 2011  SEIKO EPSON CORPORATION
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

/*! \file
    \brief  Infra-structure to provide consistent backend feedback.

    \todo  Describe purpose of the three message categories and their
           levels.  Also document usage policy and message formatting
           conventions.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "message.h"

#if ENABLE_DEBUG

unsigned long msg_level = 0;


#include <ctype.h>
#include <stdlib.h>
#include <strings.h>

/*! Initialises the message infra-structure.

    This function sets level at which the backend should produce
    feedback.  The value is gotten from the \c SANE_DEBUG_EPKOWA
    environment variable.

    The following, case insensitive string literals are supported
    (in increasing level of feedback):

     - \c FATAL
     - \c MAJOR
     - \c MINOR
     - \c INFO
     - \c CALL
     - \c DATA
     - \c CMD
     - \c HEX
     - \c IMG

    \todo  Add support for decimal literal level specification.
 */
void
msg_init (void)
{
  struct level_def
  {
    const char     *key;
    msg_level_type  val;
  };

  const struct level_def def[] =
    {
      {"FATAL", ERR_FATAL},
      {"MAJOR", ERR_MAJOR},
      {"MINOR", ERR_MINOR},

      {"INFO" , LOG_INFO},
      {"CALL" , LOG_CALL},
      {"DATA" , LOG_DATA},

      {"CMD"  , DBG_CMD},
      {"HEX"  , DBG_HEX},
      {"IMG"  , DBG_IMG},

      {NULL}                    /* array terminator */
    };

  const char *level = getenv ("SANE_DEBUG_EPKOWA");
  const struct level_def *p = def;

  msg_level = 0;

  if (!level) return;

  while (p && p->key)
    {
      if (0 == strcasecmp (level, p->key))
        {
          msg_level = p->val;
          log_info ("setting message level to '%s' (%d)",
                    p->key, p->val);
          return;
        }
      ++p;
    }
}


/*! Dumps the contents of a \a buffer in hexadecimal format.
 */
void
msg_dump (const char *fmt, const void *buffer, size_t sz)
{
  const size_t quad_length = 4;
  const size_t quad_count  = 4;
  const size_t line_length = quad_length * quad_count;

  const unsigned char *buf = buffer;

  char ascii[line_length + 1];
  size_t i = 0;

  ascii[line_length] = '\0';

  while (i < sz)
    {
      if (0 == i % line_length)         /* header */
        fprintf (stderr, "%s%08zx: ", fmt, i);

      ascii[i % line_length] = (isprint (buf[i]) ? buf[i] : '.');

      fprintf (stderr, " %02x", buf[i]);
      ++i;
      if (0 == i % quad_length)         /* spacer */
        fprintf (stderr, " ");
      if (0 == i % line_length)         /* trailer */
        fprintf (stderr, " |%s|\n", ascii);
    }

  if (0 != i % line_length)             /* last line */
    {
      do
        {                               /* align trailer */
          ascii[i % line_length] = ' ';
          fprintf (stderr, "   ");
          ++i;
          if (0 == i % quad_length)
            fprintf (stderr, " ");
        }
      while (0 != i % line_length);
      fprintf (stderr, " |%s|\n", ascii);
    }
}

#endif  /* ENABLE_DEBUG */
