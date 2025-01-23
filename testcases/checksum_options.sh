#!/bin/sh
# Test that selection of checksum works as expected.


set -e
. "$(dirname "$0")/common_funcs.sh"



allchecksumtypes="md5 sha1 sha256 sha512"


for checksumtype in $allchecksumtypes; do
   reset_teststate
   dbgecho "trying checksum $checksumtype with small files"
   echo checksumtest >a
   echo checksumtest >b
   $rdfind  -checksum "$checksumtype" -deleteduplicates true a b
   [ -e a ]
   [ ! -e b ]
done

for checksumtype in $allchecksumtypes; do
   reset_teststate
   dbgecho "trying checksum $checksumtype with large files"
   head -c 1000000 /dev/zero >a
   head -c 1000000 /dev/zero >b
   $rdfind  -checksum "$checksumtype" -deleteduplicates true a b
   [ -e a ]
   [ ! -e b ]
done

for checksumtype in $allchecksumtypes; do
   reset_teststate
   dbgecho "trying checksum $checksumtype with large files that differ only in the middle"
   ( head -c 1000000 /dev/zero; echo =====a=====; head -c 1000000 /dev/zero) >a
   ( head -c 1000000 /dev/zero; echo =====b=====; head -c 1000000 /dev/zero) >b
   $rdfind  -checksum "$checksumtype" -deleteduplicates true a b
   [ -e a ]
   [ -e b ]
done

dbgecho "all is good in this test!"
