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
   if [ "$nhardlinks" -ne "$nfiles" ] ; then
      dbgecho "expected $nfiles hardlinks, got $nhardlinks"
      exit 1
   fi
done
dbgecho passed the happy path

# try to make a hardlink to somewhere that fails.

reset_teststate
mkdir -p "$datadir/readonly.d/"
echo xxx > "$datadir/readonly.d/a"
echo xxx > "$datadir/readonly.d/b"
chmod 500 "$datadir/readonly.d/"

if [ "$(id -u)" -eq 0 ]; then
   # if running as root, directory rights are not respected. drop the capability
   # for doing that (requires capsh from package libcap2-bin)
   MAYBEDROP="capsh --drop=CAP_DAC_OVERRIDE -- -c"
else
   MAYBEDROP="/bin/sh -c"
fi
$MAYBEDROP "$rdfind -makehardlinks true $datadir/readonly.d/" 2>&1 |tee rdfind.out
if ! grep -iq "failed" rdfind.out ; then
   dbgecho "expected failure when trying to make hardlink on readonly directory"
   exit 1
fi

#make sure that our own copy is still there
for f in a b ; do
   if [ ! -e "$datadir/readonly.d/$f" ] ; then
      dbgecho "file $f is missing, rdfind should not have removed it!"
      exit 1
   fi
done

# make sure it can be cleaned up
chmod 700 "$datadir/readonly.d/"

dbgecho "all is good in this test!"
