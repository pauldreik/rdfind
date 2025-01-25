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


reset_teststate
makefiles
$rdfind -outputdelimiter "," a* b*

fields=$(grep -m 1 DUPTYPE results.txt | awk -F "," '{print NF}')
verify [ ${fields} -eq 8 ]


reset_teststate
makefiles
$rdfind -outputdelimiter " " a* b*

fields=$(grep -m 1 DUPTYPE results.txt | awk -F " " '{print NF}')
verify [ ${fields} -eq 8 ]
