#!/bin/sh
# Ensures the exclusion of empty files work as intended.
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

#try eliminate them, but they are correctly ignored.
$rdfind -ignoreempty true -deleteduplicates true a* b*
verify [ -e "a0" ]
verify [ -e "b0" ]
for i in $(seq 1 4); do
  verify [ -e "a$i" ]
  verify [ ! -e "b$i" ]
done

dbgecho "passed ignoreempty true test case"

reset_teststate
makefiles
#eliminate also the empty ones
$rdfind -ignoreempty false -deleteduplicates true a* b*
verify [ -e a0 ]
verify [ ! -e b0 ]
for i in $(seq 1 4); do
  verify [ -e "a$i" ]
  verify [ ! -e "b$i" ]
done
dbgecho "passed ignoreempty false test case"

reset_teststate
makefiles
$rdfind -minsize 0 -deleteduplicates true a* b*
verify [ -e a0 ]
verify [ ! -e b0 ]
for i in $(seq 1 4); do
  verify [ -e "a$i" ]
  verify [ ! -e "b$i" ]
done
dbgecho "passed -minsize 0 test case"

reset_teststate
makefiles

$rdfind -minsize 1 -deleteduplicates true a* b*
verify [ -e a0 ]
verify [ -e b0 ]
for i in $(seq 1 4); do
  verify [ -e "a$i" ]
  verify [ ! -e "b$i" ]
done
dbgecho "passed -minsize 1 test case"

for cutoff in $(seq 0 4); do
  reset_teststate
  makefiles
  $rdfind -minsize "$cutoff" -deleteduplicates true a* b*
  for i in $(seq 0 4); do
    verify [ -e "a$i" ]
    if [ "$i" -lt "$cutoff" ]; then
      verify [ -e "b$i" ]
    else
      verify [ ! -e "b$i" ]
    fi
  done
  dbgecho "passed -minsize $cutoff test case"
done

dbgecho "all is good for the min filesize test!"
