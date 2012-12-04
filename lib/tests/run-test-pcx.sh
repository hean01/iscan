#! /bin/sh
#  run-test-pcx.sh --  unit test for pcxstream class
#  Copyright (C) 2011  SEIKO EPSON CORPORATION
#
#  License: GPLv2+
#  Authors: AVASYS CORPORATION
#
#  This file is part of the "Image Scan!" test suite.
#
#  The "Image Scan!" test suite is free software.
#  You can redistribute it and/or modify it under the terms of the GNU
#  General Public License as published by the Free Software Foundation;
#  either version 2 of the License or at your option any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#  You ought to have received a copy of the GNU General Public License
#  along with this package.  If not, see <http://www.gnu.org/licenses/>.

if ! test -x ./test-pcx; then
    echo "FAIL: ./test-pcx not found, run make first"
    exit 1
fi

DO_COMPARE=`type convert >/dev/null && echo yes`
if test "xyes" != x$DO_COMPARE; then
    echo "INFO: compare test is skipped"
fi

# Compare test is additional.
compare () {
    FMT=`echo $SRC | sed 's,.*\.,,'`
    convert "pcx:$DST" "$FMT:-" 2>/dev/null | cmp "$SRC" -
    return $?
}

# Make temporary output in $builddir unless overridden.  Only clean up
# if tests succeed.
run_test () {
    SRC="$input"
    DST=`mktemp ${TMPDIR:=.}/pcx.XXXXXXXX`

    if ! ./test-pcx "$SRC" "$DST"; then
        echo "FAIL: ./test-pcx $SRC $DST"
        TEST_RESULT=FAIL
        return
    fi
    if test "xyes" = x$DO_COMPARE; then
        if ! compare "$SRC" "$DST"; then
            echo "FAIL: compare $SRC $DST"
            TEST_RESULT=FAIL
            return
        fi
    fi
    rm -f "$DST"
}

# `make check` normally sets $srcdir
SRCDIR=${srcdir:=.}
TEST_RESULT=PASS
for input in \
    "$SRCDIR/even-width.pbm" \
    "$SRCDIR/even-width.pgm" \
    "$SRCDIR/even-width.ppm" \
    "$SRCDIR/odd-width.pbm" \
    "$SRCDIR/odd-width.pgm" \
    "$SRCDIR/odd-width.ppm" \
    ; do
    run_test
done

test "PASS" = "$TEST_RESULT"
exit $?
