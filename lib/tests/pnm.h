/*  pnm.c -- utility functions for testing purposes
 *  Copyright (C) 2011  SEIKO EPSON CORPORATION
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


#ifndef pnm_h
#define pnm_h

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct {
    void *buffer;               /*!< image data */
    size_t size;                /*!< number of bytes allocated for buffer */
    int format;                 /*!< zero for grayscale, non-zero for RGB */
    int32_t bytes_per_line;
    int32_t pixels_per_line;
    int32_t lines;
    int32_t depth;
  } pnm;

  /*! \brief Reads a PNM image from \a file
   *
   *  Comments in the image file are allowed only between the first
   *  and second non-comment lines.  That is, only a single block of
   *  consecutive comment lines after the first line is supported.
   *
   *  Memory to hold the image data is acquired using malloc() and the
   *  caller is responsible for releasing this resource.
   *
   *  Passing the \c NULL pointer for \a file will read from \c stdin.
   */
  pnm * read_pnm (const char *file);

  /*! \brief Outputs a PNM \a image to \a file
   *
   *  Files produced are reusable for input.
   *
   *  Passing the \c NULL pointer for \a file will result in output on
   *  \c stdout.
   */
  void write_pnm (const char *file, const pnm *image,
                  const char *comment);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* defined (pnm_h) */
