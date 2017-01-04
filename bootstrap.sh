#!/bin/sh
# a script to get the source up and running with automake
#
# copyright 20016-2017 Paul Dreik (earlier Paul Sundvall)
# Distributed under GPL v 2.0 or later, at your option.
# See LICENSE for further details.

#bail out on error
set -e

aclocal
autoheader
automake --add-missing
autoconf

echo "it seems like everything went fine. now try 
./configure && make"

