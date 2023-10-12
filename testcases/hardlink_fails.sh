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
   mkdir -p "$(dirname "$datadir/$n")"
   echo "hello hardlink" > "$datadir/$n"
done

#eliminate them.
$rdfind -makehardlinks true "$datadir/"

#make sure one is a hard link to the other.
for n in $files ; do
   nhardlinks=$(stat -c %h "$datadir/$n")
   if [ $nhardlinks -ne $nfiles ] ; then
      dbgecho "expected $nfiles hardlinks, got $nhardlinks"
      exit 1
   fi
done
dbgecho passed the happy path

#now try to make a hardlink to somewhere that fails.
#ideally, we want to partitions so it is not possible to hardlink,
#but it is difficult to fix that unless the test environment
#is setup that way. therefore, make the hardlinking fail by
#trying to hardlink something we do not have access to.
#unless run as root which would be horrible.
if [ "$(id -u)" -eq 0 ]; then
   dbgecho "running as root or through sudo, dangerous! Will not proceed with this unit tests."
   exit 1
fi

reset_teststate
system_file=$(which ls)
cp "$system_file" .
$rdfind -makehardlinks true . "$system_file" 2>&1 |tee rdfind.out
if ! grep -iq "failed" rdfind.out ; then
   dbgecho "expected failure when trying to make hardlink on system partition"
   exit 1
fi

#make sure that our own copy is still there
if [ ! -e "$(basename "$system_file")" ] ; then
   dbgecho file is missing, rdfind should not have removed it!
   exit 1
fi

dbgecho "all is good in this test!"
