#!/bin/sh
# Investigate what happen when hard linking fails.
#
# See https://github.com/pauldreik/rdfind/issues/5


set -e
. "$(dirname "$0")/common_funcs.sh"

reset_teststate

#make identical files.
files="a subdir/b c some/deeply/nested/subdir/d"
nfiles=4
for n in $files ; do
    mkdir -p $(dirname $datadir/$n)
    echo "hello hardlink" > $datadir/$n
done

#eliminate them.
$rdfind -makehardlinks true $datadir/

#make sure one is a hard link to the other.
for n in $files ; do
    nhardlinks=$(stat -c %h $datadir/$n)
    if [ $nhardlinks -ne $nfiles ] ; then
	dbgecho "expected $nfiles hardlinks, got $nhardlinks"
	exit 1
    fi
done
dbgecho "all is good in this test!"
