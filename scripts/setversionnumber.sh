#!/bin/sh

#loop through all files with a revision number in and
#replace it

if [ $# -ne 2 ] ; then
   echo wants exactly two input args
   exit 1
fi

oldrev=$1
newrev=$2

echo "will substitute old revision $oldrev with $newrev"

for file in configure.in rdfind.1; do
   echo now looking at $file
   sed -e "s/$oldrev/$newrev/g" <$file >$file.tmp
   mv $file.tmp $file
done
