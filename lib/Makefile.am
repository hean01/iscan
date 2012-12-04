##  Makefile.am -- an automake template for a Makefile.in file
##  Copyright (C) 2004  Olaf Meeuwissen
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


SUBDIRS = \
	pdf \
	tests

if ENABLE_FRONTEND
noinst_LTLIBRARIES = libimage-stream.la
libimage_stream_la_CPPFLAGS = -I$(top_srcdir)/include
libimage_stream_la_LDFLAGS = -static
libimage_stream_la_LIBADD  = \
	$(LIBLTDL) \
	$(top_builddir)/lib/pdf/libpdf.la
libimage_stream_la_SOURCES = \
	$(libimage_stream_la_files)
endif
libimage_stream_la_files = \
	basic-imgstream.cc \
	basic-imgstream.hh \
	fax-encoder.cc \
	fax-encoder.hh \
	file-opener.cc \
	file-opener.hh \
	imgstream.cc \
	imgstream.hh \
	jpegstream.cc \
	jpegstream.hh \
	pcxstream.cc \
	pcxstream.hh \
	pdfstream.cc \
	pdfstream.hh \
	pngstream.cc \
	pngstream.hh \
	pnmstream.cc \
	pnmstream.hh \
	tiffstream.cc \
	tiffstream.hh

EXTRA_DIST = \
	$(libimage_stream_la_files)
