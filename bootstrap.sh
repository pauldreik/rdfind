#!/bin/sh
# a script to get the source up and running with automake
#
# copyright 20016-2017 Paul Dreik (earlier Paul Sundvall)
# Distributed under GPL v 2.0 or later, at your option.
# See LICENSE for further details.

#bail out on error
set -e

if [ `lsb_release -i 2>/dev/null | grep -c "Ubuntu")` -eq 1 ] ;
then
    echo "Detected Ubuntu OS."
    UBUNTU_OS=1
    RASPBIAN_OS=0
else
    if [ `lsb_release -i 2>/dev/null | grep -c "Raspbian")` -eq 1 ] ;
    then
        echo "Detected Raspbian OS."
        UBUNTU_OS=0
        RASPBIAN_OS=1
    else
        echo "OS is not Ubuntu or Raspbian. Check AX_CXX_COMPILE_STDCXX macros in configure.ac if you have problems running ./congfigure."
        echo "You will need the equivalent packages of Ubuntu's automake, autoconf-archive, g++, and nettle-dev on your OS."
        UBUNTU_OS=0
        RASPBIAN_OS=0
    fi
fi
if [ $UBUNTU_OS -eq 1 ] || [ $RASPBIAN_OS -eq 1 ];
then
    PACKAGE_LIST="automake g++ autoconf-archive nettle-dev"
    for PKG in ${PACKAGE_LIST} ; do
	if [ $(dpkg-query -W -f='${Status}' ${PKG} 2>/dev/null | grep -c "ok installed") -eq 0 ] ;
	then
	    echo "${PKG} not installed or not on path.";
	    echo "Run:  apt-get install ${PKG}";
	    exit
	else
	    echo "Found required package: ${PKG}";
	fi
    done

    # The following code is for older Ubuntu and Raspbian OS's which do not ship with the AX_CXX_COMPILE_STDCXX() macro in autoconf-archive"
    #
    # FRAGILE:  This code depends on the following two lines being present in configure.ac:
    #   dnl AX_CXX_COMPILE_STDCXX_11([noext], [mandatory])
    #   AX_CXX_COMPILE_STDCXX([11],[noext],[mandatory])

    if ([ $UBUNTU_OS -eq 1 ] && [ `lsb_release -rs | cut -d. -f 1` -lt "18" ]) || \
       ([ $RASPBIAN_OS -eq 1 ] && [ `lsb_release -rs | cut -d. -f 1` -lt "10" ]);
    then
	# less than Ubuntu 18:  Tested with 14.04 and 16.04, unsure of 17.X releases
	# less than Raspbian 10:  Tested with 8.0, unsure of 9.X releases
	echo "adjusting configure.ac file to use older C++ macro test. Preserving original in configure.ac.orig" ;
	if [ -e configure.ac.orig ] ;
	then
	    echo "configure.ac.orig exists, preserving that copy."
	else
	    echo "copying configure.ac to configure.ac.orig"
	    cp -p configure.ac configure.ac.orig # preserve original
	fi
	echo -n "rolling back m4 macro test in configure.ac... "
	sed -i.bak -r 's/^(\s+)dnl\s+AX_CXX_COMPILE_STDCXX_11\(\[noext\], \[mandatory\]\)/\1AX_CXX_COMPILE_STDCXX_11\(\[noext\], \[mandatory\]\)/; s/AX_CXX_COMPILE_STDCXX\(\[11\],\[noext\],\[mandatory\]\)/dnl AX_CXX_COMPILE_STDCXX\(\[11\],\[noext\],\[mandatory\]\)/' configure.ac
	echo "Done."
	echo "If you make manual changes to configure.ac, rerun this script before running ./configure"

    else
	echo "Configure.ac needs no adjustment." ;
    fi
fi

aclocal
autoheader
automake --add-missing
autoconf

echo "it seems like everything went fine. now try 
./configure && make"

