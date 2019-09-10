#!/bin/sh
# a script to get the source up and running with automake
#
# copyright 20016-2017 Paul Dreik (earlier Paul Sundvall)
# Distributed under GPL v 2.0 or later, at your option.
# See LICENSE for further details.

#bail out on error
set -e

me=$(basename $0)

for prog in aclocal autoheader automake autoconf make; do
    if ! which $prog >/dev/null 2>&1 ; then
	echo $me: please install $prog
	exit 1
    fi
done

aclocal
autoheader
automake --add-missing
autoconf

echo "it seems like everything went fine. now try 
./configure && make"

