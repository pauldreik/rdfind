#!/bin/sh
# Ensures the exclusion of empty files work as intended.
#


set -e
. "$(dirname "$0")/common_funcs.sh"



makefiles() {
#make pairs of files, with specific sizes
for i in $(seq 0 4) ; do
  head -c$i /dev/zero >a$i
  head -c$i /dev/zero >b$i
done
}


#negative value should be reported as misusage
reset_teststate
makefiles
if $rdfind -deleteduplicates true -maxsize -1 a* b* ; then
  dbgecho "that should have failed"
fi
dbgecho "passed negative value test"

reset_teststate
makefiles
#try eliminate them, but they are correctly ignored.
$rdfind -ignoreempty false -deleteduplicates true a* b*
[ -e a0 ]
[ -e b0 ]
for i in $(seq 1 4) ; do
  [ -e a$i ]
  [ ! -e b$i ]
done

dbgecho "passed ignoreempty true test case"


dbgecho "all is good for the max filesize test!"

