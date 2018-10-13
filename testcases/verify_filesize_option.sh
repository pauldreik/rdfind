#!/bin/sh
# Ensures the exclusion of empty files work as intended.
#


set -e
. "$(dirname "$0")/common_funcs.sh"

reset_teststate

#two empty files
touch a
touch b

#try eliminate them, but they are correctly ignored.
$rdfind -ignoreempty true -deleteduplicates true a b
[ -e a ]
[ -e b ]

#eliminate them.
$rdfind -ignoreempty false -deleteduplicates true a b
[ -e a ]
[ ! -e b ]



dbgecho "all is good for the symlinks test!"

