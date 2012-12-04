#! /bin/sh

test_result=PASS

run_test () {
    ./xmltest "$@"
    if test 0 = $?; then
	echo "PASS: xmltest $@"
    else
	echo "FAIL: xmltest $@"
	test_result=FAIL
    fi
}

run_test GT-X970
run_test PM-A820
run_test EP-210F
run_test GT-X970 ES-H300
run_test GT-X970 LP-M5600
run_test LP-M5600 PM-A820
run_test PM-A820 GT-X970
run_test GT-X970 GT-X970
run_test LP-M5600 LP-M5600
run_test EP-210F hoge
run_test GT-X970 hoge
run_test PM-A820 MAN
run_test EP-210F ES-H300
run_test EP-210F LP-M5600
run_test GT-X970 ES-H300 Perfection610
run_test GT-X970 ES-H300 PM-A820
run_test GT-X970 ES-H300 MAN
run_test GT-X970 LP-M5600 Perfection610
run_test GT-X970 LP-M5600 CX4600
run_test GT-X970 LP-M5600 EP-210F
run_test ES-H300 hoge GT-X970
run_test ES-H300 hoge PM-A820
run_test ES-H300 hoge EP-210F
run_test PM-A820 LP-M5600 CX4600
run_test PM-A820 LP-M5600 GT-X970
run_test PM-A820 LP-M5600 MAN
run_test LP-M5600 Perfection610 GT-X970
run_test LP-M5600 Perfection610 PM-A820
run_test LP-M5600 Perfection610 EP-210F
run_test LP-M5600 EP-210F GT-X970
run_test LP-M5600 EP-210F CX4600
run_test LP-M5600 EP-210F MAN
run_test EP-210F MAN hoge
run_test EP-210F MAN ES-H300
run_test EP-210F MAN LP-M5600
run_test EP-210F GT-X970 ES-H300
run_test EP-210F GT-X970 PM-A820
run_test EP-210F GT-X970 hoge
run_test EP-210F CX4600 Perfection610
run_test EP-210F CX4600 PM-A820
run_test EP-210F CX4600 hoge
run_test ES-H300 ES-H300 ES-H300
run_test PM-A820 PM-A820 PM-A820
run_test GT-X970 GT-X970 CX4600
run_test Perfection610 Perfection610 MAN
run_test Perfection610 Perfection610 GT-X970
run_test LP-M5600 LP-M5600 GT-X970
run_test LP-M5600 LP-M5600 PM-A820
run_test CX4600 CX4600 hoge
run_test EP-210F EP-210F ES-H300
run_test EP-210F EP-210F LP-M5600
run_test EP-210F EP-210F MAN
run_test ES-H300 GT-X970 GT-X970
run_test ES-H300 PM-A820 PM-A820
run_test GT-X970 hoge hoge
run_test CX4600 Perfection610 Perfection610
run_test PM-A820 LP-M5600 LP-M5600
run_test LP-M5600 MAN MAN
run_test hoge GT-X970 GT-X970
run_test MAN PM-A820 PM-A820
run_test hoge EP-210F EP-210F
run_test GT-X970 CX4600 GT-X970
run_test GT-X970 ES-H300 GT-X970
run_test GT-X970 EP-210F GT-X970
run_test PM-A820 ES-H300 PM-A820
run_test LP-M5600 CX4600 LP-M5600
run_test PM-A820 MAN PM-A820
run_test hoge GT-X970 hoge
run_test EP-210F CX4600 EP-210F
run_test MAN hoge MAN

test PASS = "$test_result"
exit $?
