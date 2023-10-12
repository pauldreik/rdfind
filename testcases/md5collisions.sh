#!/bin/sh
#This just tests basic operation


set -e
. "$(dirname "$0")/common_funcs.sh"

reset_teststate


#check md5 collision files
mkdir md5coll
cp "$testscriptsdir"/md5collisions/*.ps md5coll
sync

#make sure nothing happens when using sha
$rdfind  -checksum sha1 -deleteduplicates true md5coll 2>&1 |tee rdfind.out
grep -q "^Deleted 0 files.$" rdfind.out
dbgecho "using sha1 did not delete any files, as expected"

$rdfind  -checksum md5 -deleteduplicates true md5coll 2>&1 |tee rdfind.out
grep -q "^Deleted 1 files.$" rdfind.out
dbgecho "using md5 did delete files, as expected"


dbgecho "all is good in this test!"
