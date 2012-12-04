dnl  configure.ac -- an -*- autoconf -*- template for configure
dnl  Copyright (C) 2004--2007  Olaf Meeuwissen
dnl  Copyright (C) 2009, 2012  SEIKO EPSON CORPORATION
dnl
dnl  This file is part of the "Image Scan!" build infra-structure.
dnl
dnl  The "Image Scan!" build infra-structure is free software.
dnl  You can redistribute it and/or modify it under the terms of the GNU
dnl  General Public License as published by the Free Software Foundation;
dnl  either version 2 of the License or at your option any later version.
dnl
dnl  This program is distributed in the hope that it will be useful, but
dnl  WITHOUT ANY WARRANTY;  without even the implied warranty of FITNESS
dnl  FOR A PARTICULAR PURPOSE or MERCHANTABILITY.
dnl  See the GNU General Public License for more details.
dnl
dnl  You should have received a verbatim copy of the GNU General Public
dnl  License along with this program; if not, write to:
dnl
dnl	 Free Software Foundation, Inc.
dnl	 59 Temple Place, Suite 330
dnl	 Boston, MA  02111-1307	 USA
dnl
dnl  Process this file with autoconf to produce a configure script.


AC_PREREQ(2.60)
AC_INIT([Image Scan! for Linux],
	[2.29.1], [linux-scanner@epson.jp], [iscan])

SCM_INIT([-Wall -Werror])

SCM_RELEASE([5])

SCM_VENDOR([SEIKO EPSON CORPORATION])
SCM_AUTHOR([AVASYS CORPORATION])
SCM_WEBSITE([http://download.ebz.epson.net/dsc/search/01/search/?OSC=LX])
SCM_INFO(["simple, easy to use scanner utility for EPSON scanners"],["\
Image Scan! is a graphical scanner utility for people that do not need\\
all the bells and whistles provided by several of the other utilities\\
out there (xsane, QuiteInsane, Kooka).\\
.\\
At the moment it only supports SEIKO EPSON scanners and all-in-ones.\\
However, the scanner driver it provides can be used by any other SANE\\
standard compliant scanner utility.\
"],["\
Note that several scanners require a non-free plugin before they can\\
be used with this software.\
"])

AC_SUBST(DATA_PKG_NAME, "iscan-data")
AC_SUBST(MODEL_DATA_DIR_NAME, "device")

AC_CONFIG_SRCDIR([iscan.spec])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.7 gnu -Wall])


dnl  checks for programs

AC_PROG_CXX

#  Transitionary check for the C++ ABI while we are switching provided
#  binaries from gcc-3.[23] to gcc-3.4 or later.  Currently gcc-3.[23]
#  is still the official compiler version to use, but experimental bi-
#  naries are released with a ".c2" suffix.
#  The CXX_ABI for gcc-3.[23] is 102, btw.
#  Note that the PACKAGE_RELEASE for the gcc-3.4 or later packages has
#  to be larger than that for the gcc-3.[23] packages (as long as both
#  are distributed) for smooth upgrade paths.

if test x"$CXX" = xg++; then
    AC_MSG_CHECKING([C++ ABI version])
    CXX_ABI="`$CXX -E -dM - < /dev/null | $AWK '/GXX_ABI/ { print $3 }'`"
    AC_MSG_RESULT([$CXX_ABI])
    if test x"$CXX_ABI" = x1002; then
        PACKAGE_CXX_ABI=".c2"
    else
        PACKAGE_CXX_ABI=""
    fi
    AC_SUBST(PACKAGE_CXX_ABI)
fi

AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

#  CxxTest unit testing support
AC_CHECK_PROGS([CXXTESTGEN], [cxxtestgen cxxtestgen.py cxxtestgen.pl])
AM_CONDITIONAL([HAVE_CXXTESTGEN], [test x"$CXXTESTGEN" != x])
if test x$CXXTESTGEN != x; then
    CXXTESTGEN_OPTS="--runner=ErrorPrinter"
fi
AC_SUBST(CXXTESTGEN_OPTS)

#  Kludge for our binary packaging support
AC_CHECK_PROGS([DPKG], [dpkg])
AM_CONDITIONAL([HAVE_DPKG], [test x"$DPKG" != x])

dnl  checks for modules

PKG_CHECK_MODULES(GTK, gtk+-2.0,
   [
    AC_DEFINE([HAVE_GTK_2], 1,
	      [Define to 1 if you have the gtk+-2.0 library.])
   ],[
    PKG_CHECK_MODULES(GTK, gtk+) 
    PKG_CHECK_MODULES(GDK_IMLIB, imlibgdk)
   ])

AC_ARG_ENABLE(dependency-reduction,
	AC_HELP_STRING([--enable-dependency-reduction],
		[removes some unnecessary gtk related dependencies]),
	[if test "x$enable_dependency_reduction" = "xyes"; then
	    [GTK_LIBS=`echo $GTK_LIBS | sed -e "s|-latk-[0-9.]*||"`
	    GTK_LIBS=`echo $GTK_LIBS | sed -e "s|-lfontconfig||"`
	    GTK_LIBS=`echo $GTK_LIBS | sed -e "s|-lgio-[0-9.]*||"`
	    GTK_LIBS=`echo $GTK_LIBS | sed -e "s|-lgdk_pixbuf-[0-9.]*||"`
	    GTK_LIBS=`echo $GTK_LIBS | sed -e "s|-lgmodule-[0-9.]*||"`
	    GTK_LIBS=`echo $GTK_LIBS | sed -e "s|-lcairo||"`
	    GTK_LIBS=`echo $GTK_LIBS | sed -e "s|-lpango-[0-9.]*||"`
	    GTK_LIBS=`echo $GTK_LIBS | sed -e "s|-lpangocairo-[0-9.]*||"`
	    GTK_LIBS=`echo $GTK_LIBS | sed -e "s|-lfreetype||"`
	    GTK_LIBS=`echo $GTK_LIBS | sed -e "s|-lpangoft2-[0-9.]*||"`
	    GTK_CFLAGS=`echo $GTK_CFLAGS | sed -e "s|-I/usr/include/pixman-[0-9.]*||"`
	    GTK_CFLAGS=`echo $GTK_CFLAGS | sed -e "s|-I/usr/include/freetype2||"`
	    GTK_CFLAGS=`echo $GTK_CFLAGS | sed -e "s|-I/usr/include/directfb||"`
	    GTK_CFLAGS=`echo $GTK_CFLAGS | sed -e "s|-I/usr/include/libpng12||"`]
	 fi])

PKG_CHECK_MODULES(GIMP, gimp-2.0,
   [
    AC_DEFINE([HAVE_GIMP_2], 1,
	      [Define to 1 if you have the gimp-2.0 library.])
    AC_DEFINE([HAVE_ANY_GIMP], 1,
	      [Define to 1 if you have the gimp library.])
   ],[
    AC_MSG_CHECKING(for GIMP - version >= 1.0.0)
    AC_PATH_PROG(GIMP_CONFIG, gimp-config, no)
    if test x"$GIMP_CONFIG" = xno; then
	AC_MSG_RESULT(no)
    else
	GIMP_CFLAGS=`$GIMP_CONFIG --cflags`
	AC_MSG_RESULT(yes)
	AC_DEFINE([HAVE_ANY_GIMP], 1,
		  [Define to 1 if you have the gimp library.])
    fi
   ])

AC_ARG_ENABLE(gimp,
	AC_HELP_STRING([--enable-gimp],
		[ensure GIMP plugin functionality is available]),
	[if test "x$enable_gimp" != xno; then
	    if test "x$GIMP_CONFIG" = xno; then
	        AC_MSG_ERROR([required module not found])
	    fi
	 fi])


dnl  checks for libraries

PKG_CHECK_MODULES(XML, libxml-2.0)
PKG_CHECK_MODULES(LIBUSB_1_0, libusb-1.0,
    [
        AC_DEFINE([HAVE_LIBUSB_1_0], 1,
        [Define to 1 if you have the libusb-1.0 library.])
    ],[
        AC_CHECK_LIB([usb], [usb_get_busses])
    ])
AC_CHECK_LIB([sane], [sane_init])
AC_CHECK_LIB([rt], [clock_gettime], [HAVE_LIBRT=1])


dnl  checks for header files

AC_USE_SYSTEM_EXTENSIONS
AC_FUNC_ALLOCA
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([\
	fcntl.h \
	libintl.h \
	limits.h \
	locale.h \
	sane/sane.h \
	scsi/sg.h \
	stddef.h \
	stdint.h \
	stdlib.h \
	string.h \
	sys/ioctl.h \
	sys/param.h \
	sys/time.h \
	syslog.h \
	unistd.h \
	])

dnl  Conditionally check for header files needed to enable GIMP plug-in
dnl  functionality.
if test x != x"${GTK_CFLAGS}"; then
    iscan_save_CPPFLAGS="${CPPFLAGS}"
    CPPFLAGS="${CPPFLAGS} ${GTK_CFLAGS}"
    AC_CHECK_HEADERS([\
        gtk/gtk.h \
        ])
    CPPFLAGS="${iscan_save_CPPFLAGS}"
fi


dnl  checks for typedefs, structures, and compiler characteristics

AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_STRUCT_TIMEZONE
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_UID_T


dnl  checks for library functions

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION(0.14)

AC_FUNC_CLOSEDIR_VOID
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STAT
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([\
	alarm \
	atexit \
	bzero \
	floor \
	memset \
	regcomp \
	select \
	setenv \
	setlocale \
	strcasecmp \
	strncasecmp \
	strchr \
	strdup \
	strerror \
	strndup \
	strrchr \
	strstr \
	strtol \
	strtoul \
	])


dnl  The somewhat obscure LT_AC_PROG_SED is here to make sure that the
dnl  SED environment variable gets set before it's used by the libtool
dnl  machinery.  This works around buggy libtool versions.
LT_AC_PROG_SED
AC_LIBLTDL_INSTALLABLE
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
AC_LIB_LTDL
AC_SUBST(LTDLINCL)
AC_SUBST(LIBLTDL)


dnl  Graphic file format options

AC_DEFUN([ISCAN_FILE_FORMAT],
   [AC_ARG_ENABLE($1,
		  AC_HELP_STRING([--enable-$1],
				 [ensure support for the $3 file format]),
		  [if test "x$enable_$1" != xno; then
		      iff_header="`echo $2 | $as_tr_sh`"
		      AC_CHECK_HEADERS([$2])
		      if test `eval echo '$ac_cv_header_'$iff_header` \
			       != yes; then
			 AC_MSG_ERROR([required header file missing])
		      fi
		   fi],
		  [AC_CHECK_HEADERS([$2])])
   ])

ISCAN_FILE_FORMAT(jpeg,jpeglib.h,JPEG)
ISCAN_FILE_FORMAT(png,png.h,PNG)
ISCAN_FILE_FORMAT(tiff,tiffio.h,TIFF)


dnl  Support for performance measurements.

AC_MSG_CHECKING(whether timing support will be activated)
AC_ARG_ENABLE(timing,
	      AC_HELP_STRING([--enable-timing],
			     [output crude scan timing statistics]),
	      [if test "x$enable_timing" != xno; then
	          if test "x$HAVE_LIBRT" != x1; then
		     enable_timing=no
		     AC_MSG_WARN([could not find library needed for timing support])
		  fi
	       fi
	      ],
	      [enable_timing=no])
AC_MSG_RESULT([$enable_timing])
AM_CONDITIONAL(ENABLE_TIMING, test x$enable_timing = xyes)


dnl  Facilitate builds on architectures other than i386 and friends.

AC_SUBST(ISCAN_HOST_CPU,[$host_cpu])
AC_DEFINE_UNQUOTED(ISCAN_HOST_CPU,["$host_cpu"],
	[Run-time architecture tag])
AC_MSG_CHECKING(whether to build the frontend application)
AC_ARG_ENABLE(frontend,
	      AC_HELP_STRING([--enable-frontend],
			     [ensure the frontend application is built]),
	      [if test "x$enable_frontend" != xno; then
		  case $host_cpu in
		    i?86) :
			  ;;
		    x86_64) :
			  ;;
		    *)	  AC_MSG_ERROR([requires IA32 architecture])
			  ;;
		  esac
	       fi],
	      [case $host_cpu in
	         i?86)   enable_frontend=yes ;;
		 x86_64) enable_frontend=yes ;;
		 *)      enable_frontend=no  ;;
	       esac])
AC_MSG_RESULT([$enable_frontend])
AM_CONDITIONAL(ENABLE_FRONTEND, test x$enable_frontend = xyes)


dnl  SANE related issues

dnl  The SANE version information is used by the epkowa backend and in
dnl  the determination of the libtool -version-info.
AC_SUBST(SANE_MAJOR,1)
AC_SUBST(SANE_MINOR,0)
AC_SUBST(SANE_REVISION,15)
AC_DEFINE_UNQUOTED(SANE_MAJOR, $SANE_MAJOR,
	[Major version of the SANE API that we claim to support.])
AC_DEFINE_UNQUOTED(SANE_MINOR, $SANE_MINOR,
	[Minor version of the SANE API that we claim to support.])
dnl  The code taken from sane-backends provides replacement code for a
dnl  number of functions for reasons of portability.
AC_REPLACE_FUNCS([\
	sigprocmask \
	])


AC_CONFIG_FILES([
	Makefile
	backend/Makefile
	backend/tests/Makefile
	doc/Makefile
	frontend/Makefile
	lib/Makefile
	lib/pdf/Makefile
	lib/tests/Makefile
	non-free/Makefile
	po/Makefile.in
	utils/Makefile
	])

AC_OUTPUT
