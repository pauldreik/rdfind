#!/bin/sh
# Test for sha1 vs sha256

set -e
. "$(dirname "$0")/common_funcs.sh"

reset_teststate

# set small first/last bytes sizes
firstlastoptions="-firstbytessize 64 -lastbytessize 64"

#unpack collisions example from https://shattered.it/static/shattered.pdf
base64 --decode <"$testscriptsdir/sha1collisions/coll.tar.bz2.b64" | tar xvfj -

#make sure nothing happens when using sha256
# shellcheck disable=SC2086
$rdfind $firstlastoptions -checksum sha256 -deleteduplicates true . 2>&1 | tee rdfind.out
grep -q "^Deleted 0 files.$" rdfind.out
dbgecho "using sha256 did not delete any files, as expected"

# shellcheck disable=SC2086
$rdfind $firstlastoptions -checksum sha1 -deleteduplicates true . 2>&1 | tee rdfind.out
grep -q "^Deleted 1 files.$" rdfind.out
dbgecho "using sha1 did delete the files, as expected"

dbgecho "all is good in this test!"
