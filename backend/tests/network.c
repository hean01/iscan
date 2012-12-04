/*  network.c -- a network mock program for use by unit tests
 *  Copyright (C) 2008  SEIKO EPSON CORPORATION
 *
 *  License: GPLv2+
 *  Authors: AVASYS CORPORATION
 *
 *  This file is part of Image Scan!'s SANE backend test suite.
 *
 *  Image Scan!'s SANE backend test suite is free software.
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
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

int
main (int argc, char *argv[])
{
  socklen_t n;
  int r;
  int s;
  int as;
  struct sockaddr_in addr;

  s = socket (AF_INET, SOCK_STREAM, 0);
  if (0 > s)
    {
      perror ("socket");
      return EXIT_FAILURE;
    }

  memset (&addr, 0, sizeof (addr));
  addr.sin_family = PF_INET;
  addr.sin_port = 0;
  addr.sin_addr.s_addr = INADDR_ANY;

  r = bind (s, (struct sockaddr *) &addr, sizeof (addr));
  if (0 > r)
    {
      perror ("bind");
      return EXIT_FAILURE;
    }

  n = sizeof (addr);
  r = getsockname (s, (struct sockaddr *) &addr, &n);
  if (0 > r)
    {
      perror ("getsockname");
      return EXIT_FAILURE;
    }

  setvbuf (stdout, NULL, _IONBF, BUFSIZ);
  fprintf (stdout, "%d\n", ntohs (addr.sin_port));
  fclose (stdout);              /* not strictly needed */

  listen (s, 0);
  as = accept (s, (struct sockaddr*) &addr, (socklen_t*) &n);

  pause ();

  close (s);
  close (as);

  return EXIT_SUCCESS;
}
