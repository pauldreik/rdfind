#!/bin/sh
# Performance test for checksumming with different buffersizes. Not meant
# to be run for regular testing.

set -e
. "$(dirname "$0")/common_funcs.sh"

reset_teststate

TEST_DIR=buffersizes_speedtest
mkdir -p "$TEST_DIR"

make_test_files() {
    dbgecho "creating test files in $TEST_DIR/bigfiles"
    mkdir -p "$TEST_DIR/bigfiles"
    head -c $((1024*1024*500)) /dev/zero >"$TEST_DIR/bigfiles/a"
    for f in b c d e; do
      cp "$TEST_DIR/bigfiles/a" "$TEST_DIR/bigfiles/$f"
    done
    dbgecho "creating test files in $TEST_DIR/smallfiles"
    mkdir -p "$TEST_DIR/smallfiles"
    (cd "$TEST_DIR/smallfiles"; head -c100000000 /dev/zero |split --bytes 1000)
}

dbgecho "run speed test for all shecksums and buffersizes"

make_test_files

cat /dev/null >"$TEST_DIR/results.tsv"
for filesize in big small; do
for checksumtype in sha1 xxh128; do
    i=1
    while :; do
        if [ $i -gt 4096 ]; then
            break
        fi
        # Fix this properly by making rdfind to array and use "${rdfind[@]}"
        # this requires bash not sh
        # shellcheck disable=SC2086
        dbgecho "testing $checksumtype $i kB buffersize"
        # shellcheck disable=SC2086
        /usr/bin/time --append --output=$TEST_DIR/results.tsv -f "$filesize\t$i\t$checksumtype\t%e\t%M\t%C" $rdfind -buffersize $((i*1024)) -checksum "$checksumtype" -dryrun true -deleteduplicates true "$TEST_DIR/${filesize}files" >/dev/null 2>&1
        i="$((i*2))"
done
done
done
cat "$TEST_DIR/results.tsv"

