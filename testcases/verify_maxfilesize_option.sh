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
   dbgecho "negative value should have been detected"
   exit 1
fi
dbgecho "passed negative value test"

#conflict between min and max should be reported as misusage
reset_teststate
makefiles
if $rdfind -deleteduplicates true -minsize 123 -maxsize 123 a* b* ; then
   dbgecho "conflicting values should have been detected"
   exit 1
fi
dbgecho "passed conflicting value test"


reset_teststate
makefiles
#try eliminate them, but they are correctly ignored.
$rdfind -deleteduplicates true -minsize 2 -maxsize 3 a* b*
verify [ -e a2 ]
verify [ ! -e b2 ]
for i in $(seq 0 1) $(seq 3 4); do
   verify [ -e a$i ]
   verify [ -e b$i ]
done

dbgecho "passed specific size test"


dbgecho "all is good for the max filesize test!"
