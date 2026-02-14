#!/bin/sh
# Ensures the skip checksumming stage works as intended.
#

set -e
. "$(dirname "$0")/common_funcs.sh"

# make a file which is longer than "first bytes" and "last bytes" together,
# so we can make two files that differ only in the middle and will
# need checksumming to see they are different.
makefiles() {
  FIRSTBYTES=1000
  MIDDLEBYTES=1000
  LASTBYTES=1000
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

# with no checksum, we should falsely believe the files are equal
$rdfind -checksum none a* b* \
  | grep "files that are not unique" >output.log

verify [ "$(cat output.log)" = "It seems like you have 2 files that are not unique" ]

# with checksumming (the default) the files should not be considered equal.
$rdfind -checksum sha1 a* b* \
  | grep "files that are not unique" >output.log

verify [ "$(cat output.log)" = "It seems like you have 0 files that are not unique" ]

dbgecho "all is good for the checksum=none test!"
