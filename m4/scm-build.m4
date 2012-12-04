dnl  scm-build.m4 -- convenience macros for source and binary packaging
dnl  Copyright (C) 2008  AVASYS CORPORATION
dnl
dnl  License: cc-by-sa-3.0
dnl  Authors: Olaf Meeuwissen
dnl
dnl  This file is courtesy of the 'SCM Build' package.
dnl  The 'SCM Build' package itself is free software, released under
dnl  the terms of the GNU General Public License as published by the
dnl  Free Software Foundation, either version 3 of the License, or (at
dnl  your option) any later version.
dnl
dnl  This file, however, has been made available under the terms of the
dnl  Creative Commons Attribution-Share Alike 3.0 Unported License.
dnl  See <http://creativecommons.org/licenses/by-sa/3.0/> for details.


AC_DEFUN([_SCM_DEFAULT],
[
  AC_SUBST([$1], ['$2'])
  AC_CONFIG_COMMANDS_PRE([
    if test "x$$1" = 'x$2'; then
      $1="$2"
    fi
  ])
])

AC_DEFUN_ONCE([SCM_INIT],
[
  SCM_SRC_INIT($@)
  SCM_DEB_INIT
  SCM_RPM_INIT
])

AC_DEFUN_ONCE([SCM_RELEASE],
[
  AC_SUBST([scm_src_release], ["$1"])
  scm_pkg_release=`echo $1 | sed 's/^-*//'`
  AC_SUBST([PACKAGE_RELEASE], [$scm_pkg_release])
])

AC_DEFUN_ONCE([SCM_VENDOR],
[
  AC_SUBST([scm_src_vendor], ["$1"])
  AC_SUBST([scm_src_vendor_email], [$2])
])

AC_DEFUN_ONCE([SCM_AUTHOR],
[
  AC_SUBST([scm_src_author], ["$1"])
  AC_SUBST([scm_src_author_email], [$2])
])

AC_DEFUN_ONCE([SCM_MAINT],
[
  SCM_SRC_MAINT($@)
])

AC_DEFUN_ONCE([SCM_WEBSITE],
[
  AC_SUBST([scm_src_website], [$1])
])

AC_DEFUN_ONCE([SCM_INFO],
[
  AC_SUBST([scm_src_summary], [$1])
  AC_SUBST([scm_src_header],  [$2])
  AC_SUBST([scm_src_footer],  [$3])
])

AC_DEFUN_ONCE([SCM_SRC_INIT],
[
  _SCM_DEFAULT([scm_src_release],      [0])
  _SCM_DEFAULT([scm_src_vendor_email], [${PACKAGE_BUGREPORT}])
  _SCM_DEFAULT([scm_src_author],       [${scm_src_vendor}])
  _SCM_DEFAULT([scm_src_author_email], [${scm_src_vendor_email}])
  _SCM_DEFAULT([scm_src_maint],        [${scm_src_author}])
  _SCM_DEFAULT([scm_src_maint_email],  [${scm_src_author_email}])

  _SCM_DEFAULT([scm_src_summary], [${PACKAGE_NAME}])

  AM_CONDITIONAL([scm_src_have_readme_in],
                 [test -e "$srcdir/README.in"])
])

AC_DEFUN_ONCE([SCM_SRC_MAINT],
[
  AC_SUBST([scm_src_maint], ["$1"])
  AC_SUBST([scm_src_maint_email], [$2])
])


AC_DEFUN_ONCE([SCM_DEB_INIT],
[
  AC_REQUIRE([SCM_SRC_INIT])

  _SCM_DEFAULT([scm_deb_maint],       [${scm_src_maint}])
  _SCM_DEFAULT([scm_deb_maint_email], [${scm_src_maint_email}])

  _SCM_DEFAULT([scm_deb_dists], [unstable])
  _SCM_DEFAULT([scm_deb_urgency], [low])

  AC_SUBST([scm_deb_builddir], ['${top_builddir}/debian'])
  AC_SUBST([scm_deb_srcdir],   ['${top_srcdir}/debian'])

  AM_CONDITIONAL([scm_deb_have_control_in],
                 [test -e "$srcdir/debian/control.in"])
  AM_CONDITIONAL([scm_deb_have_copyright_in],
                 [test -e "$srcdir/debian/copyright.in"])
  AM_CONDITIONAL([scm_deb_have_rules_in],
                 [test -e "$srcdir/debian/rules.in"])
])

AC_DEFUN_ONCE([SCM_DEB_MAINT],
[
  AC_SUBST([scm_deb_maint], ["$1"])
  AC_SUBST([scm_deb_maint_email], [$2])
])


AC_DEFUN_ONCE([SCM_RPM_INIT],
[
  AC_REQUIRE([SCM_SRC_INIT])
  AC_REQUIRE([AC_CONFIG_SRCDIR])

  _SCM_DEFAULT([scm_rpm_maint],       [${scm_src_maint}])
  _SCM_DEFAULT([scm_rpm_maint_email], [${scm_src_maint_email}])

  AC_SUBST([scm_rpm_spec],    [${PACKAGE_TARNAME}.spec])
  AC_SUBST([scm_rpm_spec_in], [rpm.spec.in])

  AM_CONDITIONAL([scm_rpm_have_spec_in],
                 [test -e "$srcdir/$scm_rpm_spec_in"])
  AM_CONDITIONAL([scm_rpm_spec_is_srcdir_file],
                 [test x"$srcdir/$scm_rpm_spec" != x"$ac_unique_file"])
])

AC_DEFUN_ONCE([SCM_RPM_MAINT],
[
  AC_SUBST([scm_rpm_maint], ["$1"])
  AC_SUBST([scm_rpm_maint_email], [$2])
])
