##  Makefile.am -- an -*- automake -*- template for Makefile.in
##  Copyright (C) 2011  SEIKO EPSON CORPORATION
##
##  License: GPLv2+
##  Authors: AVASYS CORPORATION
##
##  This file is part of the "Image Scan!" build infra-structure.
##
##  The "Image Scan!" build infra-structure is free software.
##  You can redistribute it and/or modify it under the terms of the GNU
##  General Public License as published by the Free Software Foundation;
##  either version 2 of the License or at your option any later version.
##
##    This program is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##    GNU General Public License for more details.
##
##  You ought to have received a copy of the GNU General Public License
##  along with this package.  If not, see <http://www.gnu.org/licenses/>.

AM_CPPFLAGS = \
	-I$(top_srcdir)/lib

TESTS = \
	run-test-pcx.sh

check_PROGRAMS = \
	test-pcx

test_pcx_LDADD = \
	../libimage-stream.la \
	-lstdc++
test_pcx_SOURCES = \
	test-pcx.cc \
	pnm.c \
	pnm.h

EXTRA_DIST = \
	even-width.pbm \
	even-width.pgm \
	even-width.ppm \
	odd-width.pbm \
	odd-width.pgm \
	odd-width.ppm \
	run-test-pcx.sh
