/*  test-pcx.cc -- utility functions for testing purposes
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include "file-opener.hh"
#include "imgstream.hh"
#include "pnm.h"

int main (int argc, char *argv[])
{
  using iscan::file_opener;
  using iscan::imgstream;

  char *i = NULL;
  char *o = NULL;

  pnm *img = NULL;
  int x_res = 0;
  int y_res = 0;

  if (argc != 3)
  {
    std::cerr << "usage: ./test-pcx input.pnm output.pcx"
              << std::endl;
    return EXIT_FAILURE;
  }
  i = argv[1];
  o = argv[2];

  img = read_pnm (i);
  if (!img)
    return EXIT_FAILURE;

  x_res = 300;
  y_res = 300;

  file_opener *fo = NULL;
  imgstream *is = NULL;
  iscan::file_format format = iscan::PCX;
  fo = new file_opener (std::string (o));
  is = create_imgstream (*fo, format, false);

  is->next ();
  is->size (img->pixels_per_line, img->lines);
  is->depth (img->depth);
  iscan::colour_space space;
  if (1 == img->format)
    space = iscan::RGB;
  else if (0 == img->format && 1 == img->depth)
    space = iscan::mono;
  else if (0 == img->format)
    space = iscan::grey;
  else
    return EXIT_FAILURE;
  is->colour (space);
  is->resolution (x_res, y_res);

  int l;
  char *ptr = (char *)img->buffer;
  for (l=0; l<img->lines; ++l)
  {
    is->write (ptr, img->bytes_per_line);
    ptr += img->bytes_per_line;
  }
  is->flush ();

  delete is;
  delete fo;

  return 0;
}
