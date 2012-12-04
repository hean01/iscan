##  Makefile.am -- an automake template for a Makefile.in file
##  Copyright (C) 2004, 2005, 2007, 2008  Olaf Meeuwissen
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
bin_PROGRAMS = iscan
iscan_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/non-free \
	-DLOCALEDIR=\"$(datadir)/locale\"
iscan_CXXFLAGS = \
	@GTK_CFLAGS@ \
	@GIMP_CFLAGS@ \
	@GDK_IMLIB_CFLAGS@
iscan_LDADD = \
	$(top_builddir)/lib/libimage-stream.la \
	-lsane \
	@LIBLTDL@ \
	@GTK_LIBS@ \
	@GDK_IMLIB_LIBS@ \
	$(top_builddir)/non-free/libesmod.so
iscan_SOURCES = \
	$(iscan_source_files)
endif

iscan_source_files = \
	esmod-wrapper.hh \
	file-selector.cc \
	file-selector.h \
	gimp-plugin.h \
	pisa_aleart_dialog.cc \
	pisa_aleart_dialog.h \
	pisa_change_unit.cc \
	pisa_change_unit.h \
	pisa_configuration.cc \
	pisa_configuration.h \
	pisa_default_val.h \
	pisa_enums.h \
	pisa_error.cc \
	pisa_error.h \
	pisa_esmod_structs.h \
	pisa_gamma_correction.cc \
	pisa_gamma_correction.h \
	pisa_gimp.cc \
	pisa_gimp.h \
	pisa_gimp_1_0_patch.h \
	pisa_image_controls.cc \
	pisa_image_controls.h \
	pisa_img_converter.cc \
	pisa_img_converter.h \
	pisa_main.cc \
	pisa_main.h \
	pisa_main_window.cc \
	pisa_main_window.h \
	pisa_marquee.cc \
	pisa_marquee.h \
	pisa_preference.cc \
	pisa_preference.h \
	pisa_preview_window.cc \
	pisa_preview_window.h \
	pisa_progress_window.cc \
	pisa_progress_window.h \
	pisa_sane_scan.cc \
	pisa_sane_scan.h \
	pisa_scan_manager.cc \
	pisa_scan_manager.h \
	pisa_scan_selector.cc \
	pisa_scan_selector.h \
	pisa_scan_tool.cc \
	pisa_scan_tool.h \
	pisa_settings.cc \
	pisa_settings.h \
	pisa_structs.h \
	pisa_tool.cc \
	pisa_tool.h \
	pisa_view_manager.cc \
	pisa_view_manager.h \
	xpm_data.cc \
	xpm_data.h


EXTRA_DIST = \
	$(iscan_source_files)
