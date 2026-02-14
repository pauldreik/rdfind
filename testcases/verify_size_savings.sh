#!/bin/sh
# Ensures the calculation of saved space is correct.
#

set -e
. "$(dirname "$0")/common_funcs.sh"

#make pairs of files, with specific sizes
makefiles() {
  for i in $(seq 0 4); do
    head -c"$i" /dev/zero >"a$i"
    head -c"$i" /dev/zero >"b$i"
  done
}

reset_teststate
makefiles

$rdfind -deleteduplicates false a* b* \
  | grep "can be reduced" >output.log

# the expected saving is 1+2+3+4 byte
verify [ "$(cat output.log)" = "Totally, 10 B can be reduced." ]

# make a hardlink
ln a4 c4

$rdfind -deleteduplicates false a* b* c* \
  | grep "can be reduced" >output.log
# the expected saving is still 1+2+3+4 byte, the added hardlink should
# have been ignored.
verify [ "$(cat output.log)" = "Totally, 10 B can be reduced." ]

# make a symlink
ln -s a4 d4

$rdfind -deleteduplicates false a* b* d* \
  | grep "can be reduced" >output.log
# the expected saving is still 1+2+3+4 byte, the added symlink should
# have been ignored.
verify [ "$(cat output.log)" = "Totally, 10 B can be reduced." ]

dbgecho "all is good for the min filesize test!"
