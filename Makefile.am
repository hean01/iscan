##  Makefile.am -- an automake template for a Makefile.in file
##  Copyright (C) 2004, 2005  Olaf Meeuwissen
##  Copyright (C) 2007, 2008  EPSON AVASYS CORPORATION
##  Copyright (C) 2009  AVASYS CORPORATION
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

##  Note: SUBDIRS are recursed in the order listed.

SUBDIRS = \
	lib \
	backend \
	non-free \
	frontend \
	utils \
	doc

if ENABLE_FRONTEND
SUBDIRS += po
endif


ACLOCAL_AMFLAGS = -I m4

M4_MACROS = \
	m4/codeset.m4 \
	m4/gettext.m4 \
	m4/glibc21.m4 \
	m4/iconv.m4 \
	m4/intdiv0.m4 \
	m4/intmax.m4 \
	m4/inttypes-pri.m4 \
	m4/inttypes.m4 \
	m4/inttypes_h.m4 \
	m4/isc-posix.m4 \
	m4/lcmessage.m4 \
	m4/lib-ld.m4 \
	m4/lib-link.m4 \
	m4/lib-prefix.m4 \
	m4/longdouble.m4 \
	m4/longlong.m4 \
	m4/nls.m4 \
	m4/po.m4 \
	m4/printf-posix.m4 \
	m4/progtest.m4 \
	m4/signed.m4 \
	m4/size_max.m4 \
	m4/stdint_h.m4 \
	m4/uintmax_t.m4 \
	m4/ulonglong.m4 \
	m4/wchar_t.m4 \
	m4/wint_t.m4 \
	m4/xsize.m4

EXTRA_DIST = \
	$(M4_MACROS) \
	gettext.h \
	ltmain.sh.diff \
	m4/libtool.m4 \
	NEWS.ja \
	README.ja \
	bootstrap \
	\
	debian/changelog \
	debian/control \
	debian/control.in \
	debian/copyright \
	debian/rules \
	\
	debian/dll.conf \
	debian/$(PACKAGE_TARNAME).postinst.in \
	debian/$(PACKAGE_TARNAME).postrm.in \
	debian/$(PACKAGE_TARNAME).preinst.in \
	debian/$(PACKAGE_TARNAME).prerm.in \
	debian/$(PACKAGE_TARNAME).docs \
	debian/$(PACKAGE_TARNAME).menu \
	debian/$(PACKAGE_TARNAME).examples \
	\
	$(PACKAGE_TARNAME).desktop \
	$(PACKAGE_TARNAME).spec


update-changelog:
	> $(srcdir)/ChangeLog


##  	Source tarball related targets

##  Things to do before we pack up the sources in a tarball.

dist-hook: \
	deb-check-control \
	rpm-check-spec

##  	Debian packaging support related targets

##  Convenience variables to make writing the various Debian related
##  targets a bit easier.

DEB_PACKAGE_NAME = $(PACKAGE_TARNAME)_$(PACKAGE_VERSION)-$(PACKAGE_RELEASE)

##  A little QA check to make sure we don't accidentally release with
##  a control file that contains substitutable parameters.

deb-check-control: $(srcdir)/debian/control
	status=; \
	for f in $^; do \
	    if `LC_ALL=C $(EGREP) '@\w+@' $$f >/dev/null 2>&1`; then \
		echo "$$f contains unsubstituted variables; not releasing"; \
		status=1; \
	    fi; \
	done; \
	test x = x$$status || exit 1

.PHONY: deb-check-control


##  ^L  RPM packaging related targets

##  A little QA check to make sure we don't accidentally release with
##  a spec file that contains substitutable parameters.

rpm-check-spec: $(srcdir)/$(PACKAGE_TARNAME).spec
	status=; \
	for f in $^; do \
	  if `LC_ALL=C $(EGREP) '@\w+@' $$f >/dev/null 2>&1`; then \
	    echo "$$f has unsubstituted variables; not releasing" 1>&2; \
	    status=1; \
	  fi; \
	done; \
	test x = x$$status || exit 1

.PHONY: rpm-check-spec

MAINTAINERCLEANFILES =
MAINTAINERCLEANLOCAL = 

include scm-build.am
