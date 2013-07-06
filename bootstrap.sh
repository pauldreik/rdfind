#!/bin/sh
# a scripts to get the cvs source up and running
# $Revision: 722 $

#bail out on error
set -e

aclocal
autoheader
automake --add-missing
autoconf

echo "it seems like everything went fine. now try 
./configure && make"
