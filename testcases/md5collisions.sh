#!/bin/sh
#This just tests basic operation

rdfind=$(pwd)/rdfind


#bail out on the first error
set -e

#where is the test scripts dir?
testscriptsdir=$(dirname $(readlink -f $0))


dbgecho() {
    echo "$0 debug: " "$@"
}

echo -n "checking for rdfind ..." && [ -x $rdfind ] && echo " OK."
echo -n "checking for mktemp ..." && [ -x mktemp ] && echo " OK."

#create a temporary directory, which is automatically deleted
#on exit
datadir=$(mktemp -d -t rdfindtestcases.d.XXXXXXXXXXXX)
dbgecho "temp dir is $datadir"

cleanup () {
cd /
rm -rf $datadir
}

trap cleanup 0

[ -d $datadir ]
cd $datadir


#check md5 collision files
mkdir md5coll
cp $testscriptsdir/md5collisions/*.ps md5coll
sync

#make sure nothing happens when using sha
$rdfind  -checksum sha1 -deleteduplicates true md5coll 2>&1 |tee rdfind.out
grep -q "^Deleted 0 files.$" rdfind.out
dbgecho "using sha1 did not delete any files, as expected"

$rdfind  -checksum md5 -deleteduplicates true md5coll 2>&1 |tee rdfind.out
grep -q "^Deleted 1 files.$" rdfind.out
dbgecho "using md5 did delete files, as expected"


dbgecho "all is good in this test!"
