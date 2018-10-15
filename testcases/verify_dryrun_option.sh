#!/bin/sh
# Ensures that dryrun does not modify anything
#


set -e
. "$(dirname "$0")/common_funcs.sh"

local_reset() {
reset_teststate
echo "dryrun" >a
echo "dryrun" >b
}

local_reset
$rdfind -dryrun true -deleteduplicates true a b >rdfind.out
[ -f a ]
[ -f b ]
dbgecho "files still there, good"

local_reset
$rdfind -dryrun false -deleteduplicates true a b >rdfind.out
[ -f a ]
[ ! -e b ]
dbgecho "b was removed, good"


local_reset
$rdfind -dryrun true -makesymlinks true a b >rdfind.out
[ -f a ]
[ -f b ]
[ $(stat -c %i a) != $(stat --dereference -c %i b) ]
dbgecho "files still there, good"
$rdfind -dryrun false -makesymlinks true a b >rdfind.out
[ -f a ]
[ -L b ]
[ $(stat -c %i a) = $(stat --dereference -c %i b) ]
dbgecho "b was replaced with a symlink, good"



local_reset
$rdfind -dryrun true -makehardlinks true a b >rdfind.out
[ -f a ]
[ -f b ]
[ $(stat -c %i a) != $(stat -c %i b) ]
[ $(stat -c %h a) -eq 1 ]
dbgecho "files still there, good"
$rdfind -dryrun false -makehardlinks true a b >rdfind.out
[ -f a ]
[ -f b ]
[ $(stat -c %i a) = $(stat -c %i b) ]
[ $(stat -c %h a) -eq 2 ]
[ $(stat -c %h b) -eq 2 ]
dbgecho "b was replaced with a hard, good"


dbgecho "all is good for the dryrun tests!"

