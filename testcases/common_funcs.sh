#!/bin/sh
# common functionality for the unit tests



#bail out on the first error
set -e

me=$(basename "$0")


/bin/echo -n "$me: checking for rdfind ..."
rdfind=$PWD/rdfind
if [ ! -x "$rdfind" ]; then
   echo "could not find $rdfind"
   exit 1
fi
echo " OK."

/bin/echo -n "checking for valgrind ..."
if [ -z $VALGRIND ] ; then
   echo "not used."
else
   echo "active! here is the command: $VALGRIND"
fi

rdfind="$VALGRIND $rdfind"

#where is the test scripts dir?
testscriptsdir=$(dirname "$(readlink -f "$0")")


dbgecho() {
   echo "$0 debug: " "$@"
}


echo -n "checking for mktemp ..."
which mktemp >/dev/null
echo " OK."

#create a temporary directory, which is automatically deleted
#on exit
datadir=$(mktemp -d -t rdfindtestcases.d.XXXXXXXXXXXX)
dbgecho "temp dir is $datadir"




cleanup () {
   cd /
   rm -rf "$datadir"
}

if [ -z $KEEPTEMPDIR ] ; then
   trap cleanup INT QUIT EXIT
fi

[ -d "$datadir" ]
cd "$datadir"

reset_teststate() {
   cd /
   rm -rf "$datadir"
   mkdir -p "$datadir"
   cd "$datadir"
}


verify() {
   if ! "$@" ; then
      echo "failed asserting $*"
      exit 1

   fi
}

# where to mount disorderfs for the determinism tests
DISORDERED_MNT=$datadir/disordered_mnt
DISORDERED_ROOT=$datadir/disordered_root

# do we have a working disorder fs?
hasdisorderfs=false
if which disorderfs fusermount >/dev/null 2>&1; then
   mkdir -p "$DISORDERED_MNT" "$DISORDERED_ROOT"
   if disorderfs "$DISORDERED_ROOT" "$DISORDERED_MNT" >/dev/null 2>&1 ; then
      # "Sälj inte skinnet förrän björnen är skjuten - Don't count your chickens until they're hatched"
      fusermount -z -u "$DISORDERED_MNT"
      hasdisorderfs=true
   fi
fi
