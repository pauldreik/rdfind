#!/bin/sh
#This makes sure we can handle files bigger than 2^32-1 bytes

rdfind=$(pwd)/rdfind

#bail out on the first error
set -e

dbgecho() {
    echo "$0 debug: " "$@"
}

echo -n "checking for rdfind ..." && [ -x $rdfind ] && echo " OK."
echo -n "checking for mktemp ..." && [ -x mktemp ] && echo " OK."

#create a temporary directory, which is automatically deleted
#on exit
datadir=$(mktemp --tmpdir=/tmp -d rdfindtestcases.d.XXXXXXXXXXXX)
dbgecho "temp dir is $datadir"

cleanup () {
cd /
rm -rf $datadir
}

trap cleanup 0

[ -d $datadir ]
cd $datadir

#create a large file, sparse.
filesizem1=2147483647 #size, in bytes. This is no problem.
filesize=$(($filesizem1+1)) #size, in bytes. This is a problematic value.

#below, dd is used and the file is later appended to, to avoid problems
#on Hurd which currently (20130619) can not take $filesize as argument to
#dd without complaining and erroring out.

#make two files, which differ at the first byte to make
#rdfind return fast after comparing the initial part.
echo "a">sparse-file1
echo "b">sparse-file2
dd if=/dev/null of=sparse-file1 bs=1 seek=$filesizem1 count=1
dd if=/dev/null of=sparse-file2 bs=1 seek=$filesizem1 count=1
head -c1 /dev/zero >>sparse-file1
head -c1 /dev/zero >>sparse-file2
#let the filesystem settle
sync

#now run rdfind on the local files. Move them to a subdir
#To prevent rdfind from reading its result file or rdfind.out
mkdir subdir
mv sparse-file* subdir
$rdfind subdir  2>&1 |tee rdfind.out
dbgecho "rdfind ran ok."

#make sure rdfind.out contains the right size
grep -q "^Total size is $((filesize*2)) bytes" rdfind.out

#make sure none could be reduced
grep -q "^It seems like you have 0 files that are not unique$" rdfind.out
