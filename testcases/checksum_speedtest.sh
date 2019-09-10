#!/bin/sh
# Performance test for checksumming. Not meant
# to be run for regular testing.


set -e
. "$(dirname "$0")/common_funcs.sh"

reset_teststate


if [ ! -d speedtest ] ; then
   mkdir -p speedtest
fi

if [ ! -e speedtest/largefile1 ] ; then
   head -c1000000000 /dev/zero >speedtest/largefile1
   cp -al speedtest/largefile1 speedtest/largefile2
   #warm up the cache
   md5sum speedtest/largefile1 speedtest/largefile2
fi


for checksumtype in md5 sha1 sha256; do
   dbgecho "trying checksum $checksumtype"
   time $rdfind  -removeidentinode false -checksum $checksumtype speedtest/largefile1 speedtest/largefile2 > rdfind.out
done

dbgecho "all is good in this test!"
