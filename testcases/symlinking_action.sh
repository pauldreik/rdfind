#!/bin/sh
# Investigate what happen when symlinking fails.
#


set -e
. "$(dirname "$0")/common_funcs.sh"

reset_teststate

#make identical files.
files="first subdir/b c some/deeply/nested/subdir/d"
nfiles=4
for n in $files ; do
    mkdir -p $(dirname $datadir/$n)
    echo "hello symlink" > $datadir/$n
done

#eliminate them.
$rdfind -makesymlinks true $datadir/first $datadir/

#make sure the first one is untouched (it has the highest rank), and the rest are symlinks.
export LANG=

for n in $files ; do
if [ $n = "first" ]; then
   inodeforfirst=$(stat -c %i "$datadir/first")
   if [ x"$(stat -c %F "$datadir/first")" != x"regular file" ] ; then
      dbgecho "expected first to be a regular file"
      exit 1
   fi
else
   if [ x"$(stat -c %F "$datadir/$n")" != x"symbolic link" ] ; then
     dbgecho "expected file $n to be a symbolic link"
     exit 1
   fi
   inodeforn=$(stat --dereference -c %i "$datadir/$n")
   if [ $inodeforfirst != $inodeforn ] ; then
     dbgecho "$n does not refer to first - inode mismatch $inodeforfirst vs $inodeforn"
     exit 1
   fi  
 
fi
done
dbgecho passed the happy path

#now try to make a symlink somewhere where it fails.
if [ "$(id -u)" -eq 0 ]; then
    dbgecho "running as root or through sudo, dangerous! Will not proceed with this unit tests."
    exit 1
fi

reset_teststate
system_file=$(which ls)
cp $system_file .
$rdfind -makesymlinks true . $system_file 2>&1 |tee rdfind.out
if ! grep -iq "failed to make symlink" rdfind.out ; then
   dbgecho "did not get the expected error message. see for yourself above."
   exit 1
fi	

#make sure that our own copy is still there
if [ ! -e $(basename $system_file) ] ; then
    dbgecho file is missing, rdfind should not have removed it!
    exit 1
fi
dbgecho passed the test with trying to write to a system directory




dbgecho "all is good in this test!"
