#!/bin/sh
#
# Ensures that ranking works as intended.
#


set -e
. "$(dirname "$0")/common_funcs.sh"

#unmount disordered
unmount_disordered() {
   if ! $hasdisorderfs ; then
      return
   fi
   if [ -d "$DISORDERED_MNT" ]; then
      if ! fusermount --quiet -z -u "$DISORDERED_MNT" ; then
         dbgecho failed unmounting disordered
      fi
   fi
}

mount_disordered() {
   mkdir -p "$DISORDERED_MNT" "$DISORDERED_ROOT"
   if ! $hasdisorderfs ; then
      return
   fi
   disorderfs --sort-dirents=yes --reverse-dirents=no "$DISORDERED_ROOT" "$DISORDERED_MNT" >/dev/null
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


#enforce the rules form RANKING in the man page.

if $hasdisorderfs ; then
   echo "$me: found a working disorderfs setup. unit test will be properly executed"
else
   echo "$me: no working disorderfs setup, unit test will be partially executed"
fi

trap "unmount_disordered;cleanup" INT QUIT EXIT

#Rule 1: If A was found while scanning an input argument earlier than B, A is higher ranked.

local_reset a b
$rdfind -deleteduplicates true a b >rdfind.out
[ -f a ]
[ ! -f b ]

local_reset a sd0/a
$rdfind -deleteduplicates true a sd0/a >rdfind.out
[ -f a ]
[ ! -e sd0/a ]

local_reset a sd0/a
$rdfind -deleteduplicates true sd0/a a >rdfind.out
[ ! -e a ]
[ -e sd0/a ]

local_reset a sd0/sd1/sd2/a
$rdfind -deleteduplicates true sd0/sd1/sd2/a a >rdfind.out
[ ! -e a ]
[ -e sd0/sd1/sd2/a ]

dbgecho "tests for rule 1 passed ok"

#Rule 2: If A was found at a depth lower than B, A is higher ranked (A closer to the root)
local_reset sd0/a sd0/sd1/sd2/a
$rdfind -deleteduplicates true sd0 >rdfind.out
[ -f sd0/a ]
[ ! -e sd0/sd1/sd2/a ]

local_reset sd0/a sd0/sd1/b0 sd0/sd1/b1 sd0/sd1/sd2/c
$rdfind -deleteduplicates true sd0>rdfind.out
[ -f sd0/a ]
[ ! -e sd0/sd1/sd2/a ]

dbgecho "tests for rule 2 passed ok"

#Rule 3: If A was found earlier than B, A is higher ranked.
#We will have to test this using a tool from the reproducible builds project.
#apt install disorderfs, and make sure you are member of the fuse group.
if $hasdisorderfs ; then

   local_reset "$DISORDERED_MNT/a" "$DISORDERED_MNT/b"
   $rdfind -deleteduplicates true "$DISORDERED_MNT" >rdfind.out
   [ -f "$DISORDERED_MNT/a" ]
   [ ! -e "$DISORDERED_MNT/b" ]
   dbgecho "tests for rule 3 passed ok"

   local_reset "$DISORDERED_MNT/b" "$DISORDERED_MNT/a"
   $rdfind -deleteduplicates true "$DISORDERED_MNT" >rdfind.out
   [ -f "$DISORDERED_MNT/a" ]
   [ ! -e "$DISORDERED_MNT/b" ]
   dbgecho "tests for rule 3 passed ok"
else
   dbgecho "could not execute tests for rule 3 - please install disorderfs"
fi

dbgecho "all is good for the ranking tests!"
