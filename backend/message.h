/*  message.h -- consistent error, progress and debugging feedback
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


#ifndef message_h
#define message_h

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>

#ifndef ENABLE_DEBUG
#define ENABLE_DEBUG 0
#endif

#ifdef __cplusplus
extern "C"
{
#endif


  typedef enum
    {
      ERR_FATAL = (1 << 0),
      ERR_MAJOR = (1 << 1),
      ERR_MINOR = (1 << 2),

      LOG_INFO  = (1 << 3),
      LOG_CALL  = (1 << 4),
      LOG_DATA  = (1 << 5),

      DBG_CMD   = (1 << 6),
      DBG_HEX   = (1 << 7),
      DBG_IMG   = (1 << 8),
    }
    msg_level_type;


  extern unsigned long msg_level;


  /*! \brief Maximum buffer size not considered to be image data
   *
   *  This value can be used by low-level I/O functions that want to
   *  log the interesting data they handle but lack the knowledge to
   *  distinguish between that data and boring old image bytes.
   */
#define MSG_DBG_IMG_THRESHOLD 512


#define FMT_FILE __FILE__ ":%d: "
#define FMT_LINE , __LINE__
#define FMT_MODULE FMT_FILE "[%s]"

#define FMT_FATAL  FMT_MODULE "[F] "
#define FMT_MAJOR  FMT_MODULE "[M] "
#define FMT_MINOR  FMT_MODULE "[m] "
#define FMT_INFO   FMT_MODULE "{I} "
#define FMT_CALL   FMT_MODULE "{C} " "%s "
#define FMT_DATA   FMT_MODULE "{D} "
#define FMT_CMD    FMT_MODULE "(e) "
#define FMT_HEX    "[" MSG_MODULE "]" "(x) "
#define FMT_IMG    "[" MSG_MODULE "]" "(i) "

#if ENABLE_DEBUG

#define err_fatal(fmt,arg...)                                      \
  do                                                               \
    {                                                              \
      if (ERR_FATAL <= msg_level)                                  \
        fprintf (stderr, FMT_FATAL fmt "\n" FMT_LINE, MSG_MODULE,  \
                 ## arg);                                          \
    }                                                              \
  while (0)                                                        \
    /**/

#define err_major(fmt,arg...)                                      \
  do                                                               \
    {                                                              \
      if (ERR_MAJOR <= msg_level)                                  \
        fprintf (stderr, FMT_MAJOR fmt "\n" FMT_LINE, MSG_MODULE,  \
                 ## arg);                                          \
    }                                                              \
  while (0)                                                        \
    /**/

#define err_minor(fmt,arg...)                                      \
  do                                                               \
    {                                                              \
      if (ERR_MINOR <= msg_level)                                  \
        fprintf (stderr, FMT_MINOR fmt "\n" FMT_LINE, MSG_MODULE,  \
                 ## arg);                                          \
    }                                                              \
  while (0)                                                        \
    /**/

#define log_info(fmt,arg...)                                       \
  do                                                               \
    {                                                              \
      if (LOG_INFO <= msg_level)                                   \
        fprintf (stderr, FMT_INFO fmt "\n" FMT_LINE, MSG_MODULE,   \
                 ## arg);                                          \
    }                                                              \
  while (0)                                                        \
    /**/

#define log_call(fmt,arg...)                                       \
  do                                                               \
    {                                                              \
      if (LOG_CALL <= msg_level)                                   \
        fprintf (stderr, FMT_CALL fmt "\n" FMT_LINE, MSG_MODULE,   \
                 __func__, ## arg);                                \
    }                                                              \
  while (0)                                                        \
    /**/

#define log_data(fmt,arg...)                                       \
  do                                                               \
    {                                                              \
      if (LOG_DATA <= msg_level)                                   \
        fprintf (stderr, FMT_DATA fmt "\n" FMT_LINE, MSG_MODULE,   \
                 ## arg);                                          \
    }                                                              \
  while (0)                                                        \
    /**/

#define dbg_cmd(buf,sz)                                            \
  do                                                               \
    {                                                              \
      if (DBG_CMD <= msg_level)                                    \
        fprintf (stderr, FMT_CMD fmt "\n" FMT_LINE, MSG_MODULE,    \
                 ## arg);                                          \
    }                                                              \
  while (0)                                                        \
    /**/

#define dbg_hex(buf,sz)                                            \
  do                                                               \
    {                                                              \
      if (DBG_HEX <= msg_level)                                    \
        msg_dump (FMT_HEX, buf, sz);                               \
    }                                                              \
  while (0)                                                        \
    /**/

#define dbg_img(buf,sz)                                            \
  do                                                               \
    {                                                              \
      if (DBG_IMG <= msg_level)                                    \
        msg_dump (FMT_IMG, buf, sz);                               \
    }                                                              \
  while (0)                                                        \
    /**/

  void msg_init (void);
  void msg_dump (const char *, const void *, size_t);

#else /* !ENABLE_DEBUG */

#define err_fatal(fmt,arg...)
#define err_major(fmt,arg...)
#define err_minor(fmt,arg...)
#define log_info(fmt,arg...)
#define log_call(fmt,arg...)
#define log_data(fmt,arg...)
#define dbg_cmd(buf,sz)
#define dbg_hex(buf,sz)
#define dbg_img(buf,sz)

#define msg_init()
#define msg_dump(fmt,buf,sz)

#endif /* !ENABLE_DEBUG */

#ifdef __cplusplus
}       /* extern "C" */
#endif

#endif  /* !defined (message_h) */
