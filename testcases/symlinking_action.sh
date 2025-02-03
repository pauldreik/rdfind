#!/bin/sh
# Investigate what happen when symlinking fails.
#

set -e
. "$(dirname "$0")/common_funcs.sh"

reset_teststate

#make identical files.
files="first subdir/b c some/deeply/nested/subdir/d"
for n in $files; do
  mkdir -p "$(dirname "$datadir/$n")"
  echo "hello symlink" >"$datadir/$n"
done

#eliminate them.
$rdfind -makesymlinks true "$datadir/first" "$datadir/"

#make sure the first one is untouched (it has the highest rank), and the rest are symlinks.
export LANG=

for n in $files; do
  if [ "$n" = "first" ]; then
    inodeforfirst=$(stat -c %i "$datadir/first")
    if [ x"$(stat -c %F "$datadir/first")" != x"regular file" ]; then
      dbgecho "expected first to be a regular file"
      exit 1
    fi
  else
    if [ x"$(stat -c %F "$datadir/$n")" != x"symbolic link" ]; then
      dbgecho "expected file $n to be a symbolic link"
      exit 1
    fi
    inodeforn=$(stat --dereference -c %i "$datadir/$n")
    if [ "$inodeforfirst" != "$inodeforn" ]; then
      dbgecho "$n does not refer to first - inode mismatch $inodeforfirst vs $inodeforn"
      exit 1
    fi

  fi
done
dbgecho passed the happy path

# try to make a symlink somewhere where it fails.

reset_teststate
mkdir -p "$datadir/readonly.d/"
echo xxx >"$datadir/readonly.d/a"
echo xxx >"$datadir/readonly.d/b"
chmod 500 "$datadir/readonly.d/"

if [ "$(id -u)" -eq 0 ]; then
  # if running as root, directory rights are not respected. drop the capability
  # for doing that (requires capsh from package libcap2-bin)
  MAYBEDROP="capsh --drop=CAP_DAC_OVERRIDE -- -c"
else
  MAYBEDROP="/bin/sh -c"
fi
$MAYBEDROP "$rdfind -makesymlinks true $datadir/readonly.d/" 2>&1 | tee rdfind.out
if ! grep -iq "failed" rdfind.out; then
  dbgecho "expected failure when trying to make symlink on readonly directory"
  exit 1
fi

# make sure that our own copy is still there
for f in a b; do
  if [ ! -e "$datadir/readonly.d/$f" ]; then
    dbgecho "file $f is missing, rdfind should not have removed it!"
    exit 1
  fi
done

# make sure it can be cleaned up
chmod 700 "$datadir/readonly.d/"

dbgecho "passed the test with trying to write to a readonly directory"

#This test tries to provoke errors in relative paths, path simplification
# etc.
# argument 1 is path to file 1. argument 2 is path to file 2.
pathsimplification() {
  reset_teststate
  mkdir -p "$(dirname "$1")" && echo "simplification test" >"$1"
  mkdir -p "$(dirname "$2")" && echo "simplification test" >"$2"

  #dbgecho "state before (args  $1 $2)"
  #tree

  $rdfind -makesymlinks true "$1" "$2" 2>&1 | tee rdfind.out
  # $2 should be a symlink to $1
  if [ x"$(stat -c %F "$1")" != x"regular file" ]; then
    dbgecho "expected file $1 to be a regular file"
    exit 1
  fi
  if [ x"$(stat -c %F "$2")" != x"symbolic link" ]; then
    dbgecho "expected file $1 to be a symbolic link"
    exit 1
  fi
  inodefor1=$(stat -c %i "$1")
  inodefor2=$(stat --dereference -c %i "$2")
  if [ "$inodefor1" != "$inodefor2" ]; then
    dbgecho "inode mismatch $inodefor1 vs $inodefor2"
    exit 1
  fi
  #switching directory should still give the correct answer
  cd "$(dirname "$2")"
  inodefor2="$(stat --dereference -c %i "$(basename "$2")")"
  if [ "$inodefor1" != "$inodefor2" ]; then
    dbgecho "inode mismatch $inodefor1 vs $inodefor2"
    exit 1
  fi
  #dbgecho "state after $1 $2"
  #sync
  #tree
  echo -----------------------------------------------------------
}

pathsimplification a b
pathsimplification a subdir/b
pathsimplification subdir/a b
pathsimplification subdir1/a subdir2/b
pathsimplification subdir1/../a subdir2/b
pathsimplification subdir1/../a subdir2/./././b
pathsimplification subdir2/./././b subdir1/../a
pathsimplification a subdir2/./././b
pathsimplification "$(pwd)/a" b
pathsimplification a "$(pwd)/b"
pathsimplification "$(pwd)/a" "$(pwd)/b"
pathsimplification "$(pwd)/subdir/../a" "$(pwd)/b"
pathsimplification ./a b
pathsimplification ./a ./b
pathsimplification a ./b
pathsimplification a .//////////b

pathsimplification ./a b
pathsimplification .//a b
pathsimplification .///a b
pathsimplification a ./b
pathsimplification a .//b
pathsimplification a .///b

pathsimplification subdir/////a subdir2///./b

dbgecho passed the test with variants of paths

dbgecho "all is good for the symlinks test!"
