/*  timing.c -- optional support for run-time time stamp collection
 *  Copyright (C) 2009  SEIKO EPSON CORPORATION
 *
 *  License: GPLv2+|iscan
 *  Authors: AVASYS CORPORATION
 *
 *  This file is part of the SANE backend distributed with Image Scan!
 *
 *  Image Scan!'s SANE backend is free software.
 *  You can redistribute it and/or modify it under the terms of the GNU
 *  General Public License as published by the Free Software Foundation;
 *  either version 2 of the License or at your option any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *  You ought to have received a copy of the GNU General Public License
 *  along with this package.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 *  Linking Image Scan!'s SANE backend statically or dynamically with
 *  other modules is making a combined work based on this SANE backend.
 *  Thus, the terms and conditions of the GNU General Public License
 *  cover the whole combination.
 *
 *  As a special exception, the copyright holders of Image Scan!'s SANE
 *  backend give you permission to link Image Scan!'s SANE backend with
 *  SANE frontends that communicate with Image Scan!'s SANE backend
 *  solely through the SANE Application Programming Interface,
 *  regardless of the license terms of these SANE frontends, and to
 *  copy and distribute the resulting combined work under terms of your
 *  choice, provided that every copy of the combined work is
 *  accompanied by a complete copy of the source code of Image Scan!'s
 *  SANE backend (the version of Image Scan!'s SANE backend used to
 *  produce the combined work), being distributed under the terms of
 *  the GNU General Public License plus this exception.  An independent
 *  module is a module which is not derived from or based on Image
 *  Scan!'s SANE backend.
 *
 *  As a special exception, the copyright holders of Image Scan!'s SANE
 *  backend give you permission to link Image Scan!'s SANE backend with
 *  independent modules that communicate with Image Scan!'s SANE
 *  backend solely through the "Interpreter" interface, regardless of
 *  the license terms of these independent modules, and to copy and
 *  distribute the resulting combined work under terms of your choice,
 *  provided that every copy of the combined work is accompanied by a
 *  complete copy of the source code of Image Scan!'s SANE backend (the
 *  version of Image Scan!'s SANE backend used to produce the combined
 *  work), being distributed under the terms of the GNU General Public
 *  License plus this exception.  An independent module is a module
 *  which is not derived from or based on Image Scan!'s SANE backend.
 *
 *  Note that people who make modified versions of Image Scan!'s SANE
 *  backend are not obligated to grant special exceptions for their
 *  modified versions; it is their choice whether to do so.  The GNU
 *  General Public License gives permission to release a modified
 *  version without this exception; this exception also makes it
 *  possible to release a modified version which carries forward this
 *  exception.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "timing.h"

#if ENABLE_TIMING

#include <math.h>
#include <string.h>

size_t time_pass_count = 0;

struct time_interval_ time_scan;
struct time_interval_ time_pass[TIME_PASS_MAX];

static const long NANOS_PER_SEC = 1000000000;

static void
time_compute_span (struct time_interval_ *m)
{
  m->span.t.tv_nsec = m->stop.t.tv_nsec - m->start.t.tv_nsec;
  if (0 > m->span.t.tv_nsec)
    {
      m->span.t.tv_nsec += NANOS_PER_SEC;
      m->stop.t.tv_sec  -= 1;
    }
  m->span.t.tv_sec = m->stop.t.tv_sec - m->start.t.tv_sec;

  m->span.is_valid = m->start.is_valid && m->stop.is_valid;
}

void
time_clear (void)
{
  memset (&time_scan, 0, sizeof (time_scan));
  memset ( time_pass, 0, sizeof (*time_pass) * TIME_PASS_MAX);
}

static double
time_stamp_to_double (const struct time_stamp_ *ts)
{
  return ((double) ts->t.tv_nsec) / NANOS_PER_SEC + ts->t.tv_sec;
}

static void
time_fprintf_interval (FILE *stream, const struct time_interval_ *ti,
                       const char *prefix)
{
  char str[512];

  if (ti->span.is_valid)
    snprintf (str, num_of (str), "%s: %f (start: %f, stop: %f)", prefix,
              time_stamp_to_double (&ti->span),
              time_stamp_to_double (&ti->start),
              time_stamp_to_double (&ti->stop));
  else if (ti->start.is_valid)
    snprintf (str, num_of (str), "%s: --- (start: %f, stop: ---)", prefix,
              time_stamp_to_double (&ti->start));
  else if (ti->stop.is_valid)
    snprintf (str, num_of (str), "%s: --- (start: ---, stop: %f)", prefix,
              time_stamp_to_double (&ti->stop));
  else
    snprintf (str, num_of (str), "%s: --- (start: ---, stop: ---)", prefix);

  fprintf (stream, "%s\n", str);
}

/* There is a script in utils/ that can combine the results of
   multiple scans (in one or more SANE frontend sessions) into
   a single CSV file.
 */
void
time_stats (size_t count)
{
  size_t i, n = 0;
  double t, sum = 0, sum_sq = 0;

  fprintf (stderr, "\f\n");     /* start a new form */
  fprintf (stderr, "Per pass timing data (in seconds)\n");
  for (i = 0; i < count && i < TIME_PASS_MAX; ++i)
    {
      time_compute_span (&time_pass[i]);
      time_fprintf_interval (stderr, &time_pass[i], "pass");
    }
  fprintf (stderr, "\n");

  fprintf (stderr, "Scan timing data (in seconds)\n");
  time_compute_span (&time_scan);
  time_fprintf_interval (stderr, &time_scan, "scan");
  fprintf (stderr, "\n");

  if (time_scan.span.is_valid)
    {
      if (0 < count)
        fprintf (stderr, "Scan avg: %f s/pass\n",
                 time_stamp_to_double (&time_scan.span) / count);
    }

  for (i = 0; i < count && i < TIME_PASS_MAX; ++i)
    {
      if (time_pass[i].span.is_valid)
        {
          ++n;
          t = time_stamp_to_double (&time_pass[i].span);
          sum    += t;
          sum_sq += (t * t);
        }
    }

  if (0 != n)
    {
      fprintf (stderr, "Pass sum: %f s, %zd passes\n", sum, n);
      fprintf (stderr, "Pass avg: %f", sum / n);
      if (1 < n)                /* add standard deviation */
        {
          fprintf (stderr, " (+/- %f)",
                   sqrt ((n * sum_sq - sum * sum) / n / (n-1)));
        }
      fprintf (stderr, " s/pass\n");
    }
}

#endif /* ENABLE_TIMING */
