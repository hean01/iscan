##  Makefile.am -- an automake template for a Makefile.in file
##  Copyright (C) 2004, 2005, 2009  Olaf Meeuwissen
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


SUBDIRS = . \
	tests

AM_CPPFLAGS = \
	-DPIC \
	-DPKGDATADIR=\"$(datadir)/$(DATA_PKG_NAME)\" \
	-DMODELDATADIR=\"$(datadir)/$(DATA_PKG_NAME)/$(MODEL_DATA_DIR_NAME)\" \
	-DPKGLIBDIR=\"$(pkglibdir)\" \
	-DPKGLOCALSTATEDIR=\"$(localstatedir)/lib/$(PACKAGE_TARNAME)\" \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DENABLE_DEBUG=1 \
	-DMSG_MODULE=\"epkowa\" \
	-I$(top_srcdir)
AM_CFLAGS = \
	-fPIC

exec_sanelibdir = $(libdir)/sane
exec_sanelib_LTLIBRARIES = \
	libsane-epkowa.la

libsane_epkowa_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-DBACKEND_NAME=epkowa
libsane_epkowa_la_LDFLAGS = \
	-export-symbols-regex ^sane_ \
	-version-info $(SANE_MAJOR):$(SANE_REVISION):$(SANE_MINOR)
libsane_epkowa_la_LIBADD = \
	libepkowa.la
libsane_epkowa_la_SOURCES = \
	backend.c \
	backend.h

noinst_LTLIBRARIES = \
	libepkowa.la

libepkowa_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(LTDLINCL) \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/non-free \
	$(XML_CFLAGS) \
	$(LIBUSB_1_0_CFLAGS) \
	-DV_MAJOR=$(SANE_MAJOR) -DV_MINOR=$(SANE_MINOR)
libepkowa_la_LDFLAGS = \
	-static
libepkowa_la_LIBADD = \
	-lm \
	$(XML_LIBS) \
	$(LIBUSB_1_0_LIBS) \
	$(LIBLTDL)
libepkowa_la_SOURCES = \
	$(sane_backends_files) \
	ipc.c \
	ipc.h \
	cfg-obj.c \
	cfg-obj.h \
	command.c \
	command.h \
	defines.h \
	hw-data.c \
	hw-data.h \
	message.c \
	message.h \
	net-obj.c \
	net-obj.h \
	dip-obj.c \
	dip-obj.h \
	device.c \
	device.h \
	timing.c \
	timing.h \
	utils.c \
	utils.h \
	epkowa_ip.c \
	epkowa_ip.h \
	epkowa_ip_api.h \
	channel.c \
	channel.h \
	channel-net.c \
	channel-pio.c \
	channel-scsi.c \
	channel-usb.c \
	model-info.c \
	model-info.h \
	list.h \
	list.c \
	get-infofile.h \
	get-infofile.c \
	xmlreader.h \
	xmlreader.c

if ENABLE_TIMING
libepkowa_la_CPPFLAGS += -DENABLE_TIMING=1
libepkowa_la_LIBADD   += -lrt
endif

EXTRA_DIST = \
	extension.h \
	profile.c \
	epkowa.conf

sane_backends_files = \
	../include/sane/sanei.h \
	../include/sane/sanei_config.h \
	../include/sane/sanei_debug.h \
	../include/sane/sanei_magic.h \
	../include/sane/sanei_pio.h \
	../include/sane/sanei_scsi.h \
	../include/sane/sanei_usb.h \
	../sanei/linux_sg3_err.h \
	../sanei/sanei_config.c \
	../sanei/sanei_constrain_value.c \
	../sanei/sanei_init_debug.c \
	../sanei/sanei_magic.c \
	../sanei/sanei_pio.c \
	../sanei/sanei_scsi.c \
	../sanei/sanei_usb.c \
	epkowa.c \
	epkowa.h \
	epkowa_scsi.c \
	epkowa_scsi.h


##  There should be NO libsane.so symlink in $(exec_sanelibdir), but
##  libtool insists on making one for us anyway.
##
install-exec-hook:
	-rm -f $(DESTDIR)$(exec_sanelibdir)/libsane.so.$(SANE_MAJOR)


##  Minimal sanity checks on the backends we distribute.
##  WARNING: These checks may fail on non-Linux systems for a variety
##           of reasons.  If you know why, we would like to know (and
##           patches are naturally welcome).
check-local:
	for d in .libs _libs; do \
	  test -d $$d || continue; \
	  for l in $$d/libsane-*.so; do \
	    soname=`objdump -p $$l | awk '/SONAME/ {print $$2}'`; \
	    if test "$$soname" != "libsane.so.$(SANE_MAJOR)"; then \
	      echo "$$l: libsane.so.$(SANE_MAJOR) != $$soname"; \
	      exit 1; \
	    fi; \
	  done; \
	done
