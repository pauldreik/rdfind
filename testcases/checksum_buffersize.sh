#!/bin/sh
# Test that selection of buffersizes works as expected.

set -e
. "$(dirname "$0")/common_funcs.sh"

reset_teststate

TEST_DIR=buffersizes_speedtest
mkdir -p "$TEST_DIR"

make_test_files() {
    dbgecho "creating test files in $TEST_DIR"
    head -c 1000000 /dev/zero >"$TEST_DIR/a"
    cp "$TEST_DIR/a" "$TEST_DIR/b"
    cp "$TEST_DIR/a" "$TEST_DIR/c"
    cp "$TEST_DIR/a" "$TEST_DIR/d"
    cp "$TEST_DIR/a" "$TEST_DIR/e"
}

dbgecho "check so all buffersizes behave the same"

# disables only run once shellscheck
# shellcheck disable=SC2043
for checksumtype in sha256; do
    i=1
    while :; do
        if [ $i -gt 128 ]; then
            break
        fi
        i="$((i*2))"
        make_test_files
        dbgecho "testing buffersize $((i*1024))"
        dbgecho "testing $checksumtype"
        # Fix this properly by making rdfind to array and use "${rdfind[@]}"
        # this requires bash not sh
        # shellcheck disable=SC2086
        $rdfind -buffersize $((i*1024)) -checksum "$checksumtype" -deleteduplicates true "$TEST_DIR" >/dev/null
        [ -e "$TEST_DIR/a" ]
        [ ! -e "$TEST_DIR/b" ]
        [ ! -e "$TEST_DIR/c" ]
        [ ! -e "$TEST_DIR/d" ]
        [ ! -e "$TEST_DIR/e" ]
    done
done
