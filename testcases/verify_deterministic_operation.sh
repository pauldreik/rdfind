#!/bin/sh
# Ensures that the deterministic flag works as intended.
#


set -e
. "$(dirname "$0")/common_funcs.sh"

if $hasdisorderfs ; then
   echo "$me: found a working disorderfs setup. unit test will be properly executed"
else
   echo "$me: please install disorderfs to execute this test properly!"
   echo "$me: falsely exiting with success now"
   exit 0
fi

#unmount disordered
unmount_disordered() {
   if [ -d "$DISORDERED_MNT" ]; then
      if ! fusermount --quiet -u "$DISORDERED_MNT" ; then
         dbgecho failed unmounting disordered
      fi
   fi
}

DISORDERED_FLAGS_RANDOM="--shuffle-dirents=yes --sort-dirents=no --reverse-dirents=no"
DISORDERED_FLAGS_ASC="--shuffle-dirents=no --sort-dirents=yes --reverse-dirents=no"
DISORDERED_FLAGS_DESC="--shuffle-dirents=no --sort-dirents=yes --reverse-dirents=yes"
DISORDERED_FLAGS=$DISORDERED_FLAGS_RANDOM
mount_disordered() {
   mkdir -p "$DISORDERED_MNT"
   mkdir -p "$DISORDERED_ROOT"
   disorderfs $DISORDERED_FLAGS "$DISORDERED_ROOT" "$DISORDERED_MNT" >/dev/null
}

#create
cr8() {
   while [ $# -gt 0 ] ; do
      mkdir -p "$(dirname "$1")"
      # make sure the file is longer than what fits in the byte buffer
      head -c1000 /dev/zero >"$1"
      shift
   done
}
local_reset() {
   unmount_disordered
   reset_teststate
   mount_disordered
   cr8 "$@"
}

#sets global variable outcome to which file was preserved, a or b.
#$1 - value of -deterministic flag (true or false)
run_outcome() {
   local_reset "$DISORDERED_MNT/a" "$DISORDERED_MNT/b"
   $rdfind -deterministic $1 -deleteduplicates true "$DISORDERED_MNT" >rdfind.out
   if [ -f "$DISORDERED_MNT/a" -a ! -e "$DISORDERED_MNT/b" ] ; then
      outcome=a
   elif  [ ! -e "$DISORDERED_MNT/a" -a -f "$DISORDERED_MNT/b" ] ; then
      outcome=b
   else
      dbgecho "bad result! test failed!"
      exit 1
   fi
}

trap "unmount_disordered;cleanup" INT QUIT EXIT

#verify that with deterministic disabled, results depend on ordering.
DISORDERED_FLAGS=$DISORDERED_FLAGS_ASC
run_outcome false
outcome_asc=$outcome

DISORDERED_FLAGS=$DISORDERED_FLAGS_DESC
run_outcome false
outcome_desc=$outcome

if [ $outcome_desc = $outcome_asc ] ; then
   dbgecho "fail! \"-deterministic false\" should have given the same outcome regardless of ordering"
   exit 1
fi

dbgecho "tests for deterministic false passed ok (non-randomized)"

#verify that with deterministic ordering off, we may get different results
#depending on the output from the file system
DISORDERED_FLAGS=$DISORDERED_FLAGS_RANDOM
run_outcome false
last_outcome=$outcome
for i in $(seq 128) ; do
   run_outcome false
   if [  $last_outcome != $outcome ] ; then
      #proved that both outcomes can happen. good!
      dbgecho "got a different outcome after $i random tries"
      break
   else
      if [ $i -eq 64 ] ; then
         dbgecho "reached max number of iterations without getting different results".
         exit 1
      fi
   fi
   last_outcome=$outcome
done

dbgecho "tests for \"-deterministic false\" passed ok on randomized filesystem order"


#verify that with deterministic enabled, we get the same results regardless of ordering
DISORDERED_FLAGS=$DISORDERED_FLAGS_ASC
run_outcome true
outcome_asc=$outcome

DISORDERED_FLAGS=$DISORDERED_FLAGS_DESC
run_outcome true
outcome_desc=$outcome

if [ $outcome_desc != $outcome_asc ] ; then
   dbgecho "fail! \"-deterministic true\" should have given the same outcome regardless of ordering"
   exit 1
fi
dbgecho "tests for deterministic true passed ok"




dbgecho "all is good for the ranking tests!"
