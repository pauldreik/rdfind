#!/bin/sh
# Performance test for checksumming with diffrent buffersizes. Not meant
# to be run for regular testing.

set -e
. "$(dirname "$0")/common_funcs.sh"

reset_teststate

TEST_DIR=buffersizes_speedtest
mkdir -p "$TEST_DIR"

make_test_files() {
    dbgecho "creating test files in $TEST_DIR"
    head -c $((1024*1024*500)) /dev/zero >"$TEST_DIR/a"
    cp "$TEST_DIR/a" "$TEST_DIR/b"
    cp "$TEST_DIR/a" "$TEST_DIR/c"
    cp "$TEST_DIR/a" "$TEST_DIR/d"
    cp "$TEST_DIR/a" "$TEST_DIR/e"
}

dbgecho "run speed test for all shecksums and buffersizes"

make_test_files

for checksumtype in md5 sha1 sha256 sha512; do
    i=1
    while :; do
        if [ $i -gt 128 ]; then
            break
        fi
        i="$((i*2))"
        dbgecho "testing buffersize $((i*1024))"
        dbgecho "testing $checksumtype"
        # Fix this properly by making rdfind to array and use "${rdfind[@]}"
        # this requires bash not sh
        # shellcheck disable=SC2086
        time $rdfind -buffersize $((i*1024)) -checksum "$checksumtype" -dryrun true -deleteduplicates true "$TEST_DIR" >/dev/null
        dbgecho "testing $checksumtype memusage"
        # shellcheck disable=SC2086
        /usr/bin/time -f '%M kB' $rdfind -buffersize $((i*1024)) -checksum "$checksumtype" -dryrun true -deleteduplicates true "$TEST_DIR" >/dev/null
    done
done


