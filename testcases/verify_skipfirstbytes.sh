#!/bin/sh
# Ensures the skip first bytes step checks
#

set -e
. "$(dirname "$0")/common_funcs.sh"

FIRSTBYTES=1000
MIDDLEBYTES=1000
LASTBYTES=1000

# make a file which is longer than "first bytes" and "last bytes" together,
# so we can make two files that differ only in the middle and will
# need checksumming to see they are different.
makefiles() {
  for f in a b; do
    (
      head -c$FIRSTBYTES </dev/zero
      head -c$MIDDLEBYTES </dev/urandom
      head -c$LASTBYTES </dev/zero
    ) >$f
  done
}

reset_teststate
makefiles

defaultfirst="-firstbytessize 64"
defaultlast="-lastbytessize 64"

# with no checksum, we should falsely believe the files are equal
# shellcheck disable=SC2086
$rdfind -checksum none $defaultfirst $defaultlast a* b* \
  | grep "files that are not unique" >output.log
verify [ "$(cat output.log)" = "It seems like you have 2 files that are not unique" ]

# if we set the first bytes size to be very large, we will detect it
# shellcheck disable=SC2086
$rdfind -checksum none -firstbytessize $((FIRSTBYTES + MIDDLEBYTES)) $defaultlast a* b* \
  | grep "files that are not unique" >output.log
verify [ "$(cat output.log)" = "It seems like you have 0 files that are not unique" ]

# if we set the last bytes size to be very large, we will also detect it
# shellcheck disable=SC2086
$rdfind -checksum none $defaultfirst -lastbytessize $((MIDDLEBYTES + LASTBYTES)) a* b* \
  | grep "files that are not unique" >output.log
verify [ "$(cat output.log)" = "It seems like you have 0 files that are not unique" ]

dbgecho "all is good for the skip first bytes step check!"
