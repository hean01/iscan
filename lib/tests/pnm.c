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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "pnm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

pnm *
read_pnm (const char *file)
{
  pnm image, *rv;

  char *line = NULL;
  size_t sz  = 0;

  FILE *fp;

  if (file)
    fp = fopen (file, "r");
  else
    fp = stdin;

  if (!fp) return NULL;

  getline (&line, &sz, fp);     /* file magic number */
  if ('P' != line[0]) return NULL;

  image.depth = 8;
  /**/ if ('6' == line[1]) image.format = 1;
  else if ('5' == line[1]) image.format = 0;
  else if ('4' == line[1]) { image.format = 0; image.depth = 1; }
  else return NULL;

  getline (&line, &sz, fp);
  while ('#' == line[0])        /* skip header comments */
    getline (&line, &sz, fp);
  sscanf (line, "%d%d", &image.pixels_per_line, &image.lines);
  if (1 < image.depth)
  {
  getline (&line, &sz, fp);     /* max color value */
  sscanf (line, "%d", &image.depth);
  /**/ if (image.depth < (1 <<  1)) image.depth =  1;
  else if (image.depth < (1 <<  8)) image.depth =  8;
  else if (image.depth < (1 << 16)) image.depth = 16;
  else return NULL;
  }

  free (line);
  sz = 0;

  image.bytes_per_line = (image.pixels_per_line
                          * (image.format ? 3 :1)
                          * (image.depth == 16 ? 2 : 1));
  if (1 == image.depth)
    {
      image.bytes_per_line = (image.bytes_per_line + 7) / 8;
    }
  image.size = image.bytes_per_line * image.lines;

  image.buffer = malloc (image.size);
  if (!image.buffer) return NULL;
  while (sz < image.size)
    {
      size_t s = fread (image.buffer, 1, image.size, fp);
      if (0 == s)
        {
          if (ferror (fp))
            {
              fprintf (stderr, "error reading image file\n");
              free (image.buffer);
              return NULL;
            }
          if (feof (fp))
            {
              fprintf (stderr, "premature end of image file\n");
              free (image.buffer);
              return NULL;
            }
        }
      sz += s;
    }
  fclose (fp);

  rv = (pnm *) malloc (sizeof (pnm));
  if (!rv)
    {
      free (image.buffer);
      return NULL;
    }
  memcpy (rv, &image, sizeof (pnm));

  return rv;
}

void
write_pnm (const char *file, const pnm *image, const char *comment)
{
  FILE *fp;

  if (file)
    fp = fopen (file, "w");
  else
    fp = stdout;

  if (!fp) return;

  if (1 == image->depth)
  {
    fprintf (fp, "P4\n");
  }
  else
  {
  fprintf (fp, "P%d\n", (image->format ? 6 : 5));
  }
  fprintf (fp, "# %s\n", comment);
  fprintf (fp, "%d %d\n", image->pixels_per_line, image->lines);
  if (1 != image->depth)
  {
  fprintf (fp, "%d\n", (1 << image->depth) - 1);
  }
  fwrite (image->buffer, 1, image->bytes_per_line * image->lines, fp);
  fflush (fp);
  fclose (fp);
}
