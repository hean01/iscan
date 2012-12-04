##  Makefile.am -- an -*- automake -*- template for Makefile.in
##  Copyright (C) 2008  SEIKO EPSON CORPORATION
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


AM_CPPFLAGS = $(XML_CFLAGS)

check_PROGRAMS = \
	xmltest

TESTS =

xmltest_LDADD = ../libepkowa.la
xmltest_SOURCES = xmltest.c xmltest.h

EXTRA_DIST = \
	47542d58393730.xml \
	45532d48333030.xml \
	50657266656374696f6e363130.xml \
	xmltest-runner.sh

if HAVE_CXXTESTGEN

check_PROGRAMS += \
        cfg-obj \
        net-obj \
        network \
        model-info

TESTS += \
	cfg-obj \
	net-obj \
	model-info

cfg_obj_LDADD = ../libepkowa.la
cfg_obj_SOURCES = \
	test-cfg-obj.cc \
	test-cfg-obj.hh

net_obj_LDADD = ../libepkowa.la
net_obj_SOURCES = \
	test-net-obj.cc \
	test-net-obj.hh

model_info_LDADD = ../libepkowa.la
model_info_SOURCES = \
	test-model-info.cc \
	test-model-info.hh

#  Use the CxxTest code generator on all files matching test-*.hh to
#  create the corresponding test-*.cc file.  This convention is used
#  so that we can still use regular source code without this getting
#  clobbered as soon as we change the header file.

.hh.cc:
	@if test xtest- = "x`echo $^ | sed -n 's|^\(test-\).*|\1|p'`"; \
	then \
	    echo "$(CXXTESTGEN) $(CXXTESTGEN_OPTS) -o $@ $^"; \
	    $(CXXTESTGEN) $(CXXTESTGEN_OPTS) -o $@ $^; \
	fi

endif ## HAVE_CXXTESTGEN
