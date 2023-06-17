#!/bin/sh
# Test that selection of checksum works as expected.


set -e
. "$(dirname "$0")/common_funcs.sh"





for checksumtype in md5 sha1 sha256 sha512; do
   reset_teststate
   dbgecho "trying checksum $checksumtype"
   echo checksumtest >a
   echo checksumtest >b
   $rdfind  -checksum $checksumtype -deleteduplicates true a b
   [ -e a ]
   [ ! -e b ]
done

dbgecho "all is good in this test!"
