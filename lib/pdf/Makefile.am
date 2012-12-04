##  Makefile.am -- an automake template for a Makefile.in file
##  Copyright (C) 2008  SEIKO EPSON CORPORATION
##
##  This file is part of the "Image Scan!" build infra-structure.
##
##  The "Image Scan!" build infra-structure is free software.
##  You can redistribute it and/or modify it under the terms of the GNU
##  General Public License as published by the Free Software Foundation;
##  either version 2 of the License or at your option any later version.
##
##  This program is distributed in the hope that it will be useful, but
##  WITHOUT ANY WARRANTY;  without even the implied warranty of FITNESS
##  FOR A PARTICULAR PURPOSE or MERCHANTABILITY.
##  See the GNU General Public License for more details.
##
##  You should have received a verbatim copy of the GNU General Public
##  License along with this program; if not, write to:
##
##      Free Software Foundation, Inc.
##      59 Temple Place, Suite 330
##      Boston, MA  02111-1307  USA


if ENABLE_FRONTEND
noinst_LTLIBRARIES = libpdf.la
libpdf_la_LDFLAGS = -static
libpdf_la_SOURCES = \
	$(libpdf_la_files)
endif

libpdf_la_files = \
	array.cc \
	array.hh \
	dictionary.cc \
	dictionary.hh \
	object.cc \
	object.hh \
	primitive.cc \
	primitive.hh \
	writer.cc \
	writer.hh

EXTRA_DIST = \
	$(libpdf_la_files)
