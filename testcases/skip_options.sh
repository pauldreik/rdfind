#!/bin/sh
# Test that selection of skip works as expected.


set -e
. "$(dirname "$0")/common_funcs.sh"





for skiptype in firstbytes lastbytes; do
   reset_teststate
   dbgecho "trying skip $skiptype"
   echo skiptype >a
   echo skiptype >b
   $rdfind -skip $skiptype -deleteduplicates true a b
   [ -e a ]
   [ ! -e b ]
done

dbgecho "all is good in this test!"
