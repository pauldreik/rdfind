#!/bin/sh
# common functionality for the unit tests



#bail out on the first error
set -e

me=$(basename $0)

/bin/echo -n "checking for rdfind ..."
rdfind=$(readlink -f $(dirname $0)/../rdfind)
if [ ! -x "$rdfind" ]; then
    echo "could not find $rdfind"
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
testscriptsdir=$(dirname $(readlink -f $0))


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

[ -d $datadir ]
cd $datadir

reset_teststate() {
    cd /
    rm -rf "$datadir"
    mkdir -p $datadir
    cd "$datadir"
}


verify() {
if ! $@ ; then
  echo "failed asserting $@"
  exit 1
fi
}

