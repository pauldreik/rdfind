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

for dryrunopt in -dryrun -n ; do
   local_reset
   $rdfind $dryrunopt true -deleteduplicates true a b >rdfind.out
   [ -f a ]
   [ -f b ]
   dbgecho "files still there, good"

   local_reset
   $rdfind $dryrunopt false -deleteduplicates true a b >rdfind.out
   [ -f a ]
   [ ! -e b ]
   dbgecho "b was removed, good"


   local_reset
   $rdfind $dryrunopt true -makesymlinks true a b >rdfind.out
   [ -f a ]
   [ -f b ]
   [ "$(stat -c %i a)" != "$(stat --dereference -c %i b)" ]
   dbgecho "files still there, good"
   $rdfind $dryrunopt false -makesymlinks true a b >rdfind.out
   [ -f a ]
   [ -L b ]
   [ "$(stat -c %i a)" = "$(stat --dereference -c %i b)" ]
   dbgecho "b was replaced with a symlink, good"



   local_reset
   $rdfind $dryrunopt true -makehardlinks true a b >rdfind.out
   [ -f a ]
   [ -f b ]
   [ "$(stat -c %i a)" != "$(stat -c %i b)" ]
   [ "$(stat -c %h a)" -eq 1 ]
   dbgecho "files still there, good"
   $rdfind $dryrunopt false -makehardlinks true a b >rdfind.out
   [ -f a ]
   [ -f b ]
   [ "$(stat -c %i a)" = "$(stat -c %i b)" ]
   [ "$(stat -c %h a)" -eq 2 ]
   [ "$(stat -c %h b)" -eq 2 ]
   dbgecho "b was replaced with a hard link, good"


   #make sure users who forget the boolean argument after
   #dryrun get something comprehensible. see https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=754663
   local_reset
   if $rdfind $dryrunopt a b >rdfind.out 2>&1 ; then
      dbgecho "this should have failed, but did not!"
      exit 1
   fi
   dbgecho "rdfind exited with error status after \"rdfind $dryrunopt a b\", good."
   if ! grep -iq "^expected true or false after $dryrunopt" rdfind.out ; then
      dbgecho "got unexpected response after \"rdfind $dryrunopt a b\":"
      tail rdfind.out
      exit 1
   fi

   #dryrun on it's own: "rdfind -dryrun"
   local_reset
   if $rdfind $dryrunopt >rdfind.out 2>&1 ; then
      dbgecho "this should have failed, but did not!"
      exit 1
   fi
   dbgecho "rdfind exited with error status after \"rdfind $dryrunopt\", good."
   if grep -iq "^did not understand option 1:" rdfind.out ; then
      dbgecho "got the old non-helpful answer:"
      tail rdfind.out
      exit 1
   fi

   #dryrun with single argument: "rdfind -dryrun ."
   local_reset
   if $rdfind $dryrunopt a >rdfind.out 2>&1 ; then
      dbgecho "this should have failed, but did not!"
      exit 1
   fi
   dbgecho "rdfind exited with error status after \"rdfind $dryrunopt\", good."
   if grep -iq "^did not understand option 1:" rdfind.out ; then
      dbgecho "got the old non-helpful answer:"
      tail rdfind.out
      exit 1
   fi
done

dbgecho "all is good for the dryrun tests!"
