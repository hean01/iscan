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

man_MANS = \
	sane-epkowa.5 \
	iscan-registry.8

if ENABLE_FRONTEND
man_MANS += iscan.1
endif


EXTRA_DIST = \
	sane-epkowa.man \
	iscan.man \
	iscan-registry.man \
	xinetd.sane

CLEANFILES = $(man_MANS)

configdir = $(sysconfdir)/sane.d

%.1 %.5 %.8: %.man
	@sed -e 's|@CONFIGDIR@|$(configdir)|g' \
	     -e 's|@LIBDIR@|$(libdir)/sane|g' \
	     $^ >$@
	@echo Generating manpage $@...
