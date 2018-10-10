dnl configure.in for rdfind.
dnl copyright 20016-2017 Paul Dreik (earlier Paul Sundvall)
dnl Distributed under GPL v 2.0 or later, at your option.
dnl See LICENSE for further details.

VERSION="1.4.0alpha"
AC_INIT(rdfind,1.4.0alpha)
AC_CONFIG_SRCDIR(rdfind.cc)
AC_CONFIG_HEADERS(config.h)       

AM_INIT_AUTOMAKE


dnl find and test the C compiler
AC_PROG_CXX
AC_LANG_CPLUSPLUS
        
AC_PROG_MAKE_SET
               
AC_HEADER_STDC

dnl see if we should run a debug build with lots of output
showdebugmessages=no
AC_ARG_ENABLE(debug,
 	AC_HELP_STRING(
 	    [--enable-debug],
 	    [enable debug printouts in program (default=no)]
 	),
   	showdebugmessages="$enableval"
)
AC_MSG_RESULT($showdebugmessages)
if test x"$showdebugmessages" = x"yes"; then
	AC_DEFINE([RDFIND_DEBUG],[1],[Enable debug printouts in program])
dnl	CXXFLAGS="$CXXFLAGS -g -Wall -Werror"
fi	


dnl test for nettle
AC_CHECK_HEADER(nettle/sha.h,,[AC_MSG_ERROR([
 nettle header files missing. Please install nettle
 first. If you have already done so and get this error message 
 anyway, it may be installed somewhere else, maybe because you 
 don't have root access. Pass CPPFLAGS=-I/your/path/to/nettle to configure
 and try again. The path should be so that #include \"nettle/sha.h\" works.
 On Debian-ish systems, use \"apt-get install nettle-dev\" to get a system
 wide nettle install.
])]) 
AC_CHECK_LIB(nettle,main,,[AC_MSG_ERROR([
 Could not link to libnettle. Please install nettle
 first. If you have already done so; please run ldconfig
 as root or check whether the path libnettle was installed
 to is in your LD_LIBRARY_PATH. If you have nettle 
 somewhere else, maybe because you don't have root 
 access, pass LDFLAGS=-L/your/path/to/nettle to configure
 and try again.
])])

dnl test for some specific functions
AC_CHECK_FUNC(stat,,AC_MSG_ERROR(oops! no stat ?!?))
AC_CHECK_FUNC(nanosleep,,AC_MSG_WARN(cant do nanosleep ?))

AC_CHECK_LIB(stdc++, main,,AC_MSG_ERROR(rdfind requires libstdc++))          

AC_CHECK_HEADERS(string,,AC_MSG_WARN(STL classes missing ?))
AC_CHECK_HEADERS(vector,,AC_MSG_WARN(STL classes missing ?))
AC_CHECK_HEADERS(time.h,,AC_MSG_WARN(nanosleep header time.h missing?))

dnl check for 64 bit support
AC_SYS_LARGEFILE

dnl make sure we have c++11 or better,
dnl allow for disabling this check on older systems like Ubuntu 14.04
AC_ARG_ENABLE(cppstandardcheck,
	[AS_HELP_STRING([--disable-cppstandardcheck], [disable checking for C++11 and trust CXX and CXXFLAGS instead ])],
	[ ENABLECPPSTANDARDCHECK=$enableval ],
	[ ENABLECPPSTANDARDCHECK=yes] )
if test "$ENABLECPPSTANDARDCHECK" = "yes"; then
   AC_MSG_NOTICE([checking for c++ 11, disable with --disable-cppstandardcheck])
   dnl AX_CXX_COMPILE_STDCXX_11([noext], [mandatory])
   AX_CXX_COMPILE_STDCXX([11],[noext],[mandatory])
else
   AC_MSG_NOTICE([not checking for c++11 support.])
fi



AC_SUBST(VERSION)


dnl check for how to set warning flags
set_more_warnings=no
AC_ARG_ENABLE(warnings,
 	AC_HELP_STRING(
 	    [--enable-warnings],
 	    [enable compiler warnings (default=no)]
 	),
   	set_more_warnings="$enableval"
)
dnl the following warning related paragraph is from the gnumeric project, which
dnl is gpl-v2 licensed. I modified and expanded it a bit.
if test "x$set_more_warnings" != xno; then
	dnl Clang needs this option, or else it will appear to support any
	dnl warning option, only to spew warnings about them later.
	uwoption="-Werror=unknown-warning-option"
	SAVE_CXXFLAGS="$CXXFLAGS"
	CXXFLAGS="$CXXFLAGS $uwoption"
	AC_MSG_CHECKING([whether compiler understands $uwoption])
	AC_COMPILE_IFELSE(
		[AC_LANG_PROGRAM([], [])],
		[has_unknown_warning_option=yes],
		[has_unknown_warning_option=no; uwoption=""])
	AC_MSG_RESULT($has_unknown_warning_option)
	CXXFLAGS="$SAVE_CXXFLAGS"

	dnl see https://github.com/lefticus/cppbestpractices/blob/master/02-Use_the_Tools_Available.md#compilers for a list of suggestions. Let's start
	dnl small to begin with, and gradually increase.
	warning_options="-Wall -Wextra -pedantic \
	                 -Wshadow \
			 -Wold-style-cast \
			 -Wnon-virtual-dtor \
			 -Wold-style-cast \
			 -Wcast-align \
			 -Wunused \
			 -Woverloaded-virtual \
			 -Wconversion \
			 -Wsign-conversion \
			 -Wmisleading-indentation \
			 -Wduplicated-cond \
			 -Wduplicated-branches \
			 -Wlogical-op \
			 -Wnull-dereference \
			 -Wuseless-cast \
			 -Wdouble-promotion \
			 -Wformat=2 \
	                 -Wbiznafronck"	
	for option in $warning_options ; do
		SAVE_CXXFLAGS="$CXXFLAGS"
		CXXFLAGS="$CXXFLAGS $uwoption $option"
		AC_MSG_CHECKING([whether compiler understands $option])
		dnl Include a system header so we ignore Werror=... flags
		dnl that cause trouble.
		AC_COMPILE_IFELSE(
			[AC_LANG_PROGRAM([[#include <stdio.h>]], [])],
			[has_option=yes],
			[has_option=no])
		CXXFLAGS="$SAVE_CXXFLAGS"
		if test $has_option = yes; then
			CXXFLAGS="$CXXFLAGS $option"
		fi
		AC_MSG_RESULT(got result $has_option)
		unset has_option
		unset SAVE_CXXFLAGS
	done
	unset option
fi


dnl read Makefile.in and write Makefile
AC_OUTPUT(Makefile)
