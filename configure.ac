dnl configure.in for rdfind.
dnl copyright 2016-2019 Paul Dreik (earlier Paul Sundvall)
dnl Distributed under GPL v 2.0 or later, at your option.
dnl See LICENSE for further details.

VERSION="1.7.0"
AC_INIT([rdfind],[1.7.0])
AC_CONFIG_SRCDIR([rdfind.cc])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE


dnl find and test the C++ compiler
AC_PROG_CXX
AC_LANG([C++])

AC_PROG_MAKE_SET

dnl see if we should run a debug build with lots of output
showdebugmessages=no
AC_ARG_ENABLE(debug,
        AS_HELP_STRING(
            [--enable-debug],
            [enable debug printouts in program (default=no)]
        ),
        showdebugmessages="$enableval"
)
AC_MSG_RESULT($showdebugmessages)
if test x"$showdebugmessages" = x"yes"; then
        AC_DEFINE([RDFIND_DEBUG],[1],[Enable debug printouts in program])
fi

dnl for assert() support
AC_HEADER_ASSERT

dnl test for nettle
AC_CHECK_HEADER(nettle/sha1.h,,[AC_MSG_ERROR([
 nettle header files missing. Please install nettle
 first. If you have already done so and get this error message
 anyway, it may be installed somewhere else, maybe because you
 don't have root access. Pass CPPFLAGS=-I/your/path/to/nettle to configure
 and try again. The path should be so that \#include "nettle/sha1.h" works.
 On Debian-ish systems, use "apt-get install nettle-dev" to get a system
 wide nettle install.
])])
AC_CHECK_LIB(nettle,nettle_pbkdf2_hmac_sha256,,[AC_MSG_ERROR([
 Could not link to libnettle. Please install nettle
 first. If you have already done so; please run ldconfig
 as root or check whether the path libnettle was installed
 to is in your LD_LIBRARY_PATH. If you have nettle
 somewhere else, maybe because you don't have root
 access, pass LDFLAGS=-L/your/path/to/nettle to configure
 and try again.
])])

dnl xxh hashing is optional:
dnl   --with-xxhash requires it
dnl   --without-xxhash does not use it
dnl   not saying anything enables it if it can be found
AC_ARG_WITH([xxhash],
            [AS_HELP_STRING([--with-xxhash],
              [support xxhash @<:@default=check@:>@])],
            [],
            [with_xxhash=check])

          LIBXXHASH=
          AS_IF([test "x$with_xxhash" != xno],
            [AC_CHECK_LIB([xxhash], [XXH3_128bits],
              [AC_SUBST([LIBXXHASH], ["-lxxhash"])
               AC_DEFINE([HAVE_LIBXXHASH], [1],
                         [Define if you have libxxhash])
              ],
              [if test "x$with_xxhash" != xcheck; then
                 AC_MSG_FAILURE([
                   --with-xxhash was given, but test for xxhash failed.
                   Please install xxhash first. If you have already done so and get this error message
                   anyway, it may be installed somewhere else, maybe because you
                   don't have root access. Pass CPPFLAGS=-I/your/path/to/xxhash to configure
                   and try again. The path should be so that \#include "xxhash.h" works.
                   On Debian-ish systems, use "apt-get install libxxhash-dev" to get a system
                   wide xxhash install.
                   On RedHat-ish systems, use "dnf install xxhash-devel" to get a system
                   wide xxhash install.
                   If you have xxhash somewhere else, maybe because you don't have root
                   access, pass LDFLAGS=-L/your/path/to/xxhash to configure and try again.
                 ])
               fi
              ],)])

dnl test for some specific functions
AC_CHECK_FUNC(stat,,AC_MSG_ERROR(oops! no stat ?!?))

dnl check for 64 bit support
AC_SYS_LARGEFILE

dnl make sure we have c++17 or better,
AC_MSG_CHECKING([for C++17 support or better])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([int f() { if constexpr(true) {auto a=1;return a;} else {return 2;}}])],
                  [AC_MSG_RESULT([yes])],
		  [AC_MSG_ERROR([no c++17 support, please set CXXFLAGS properly])])

AC_SUBST(VERSION)


dnl check for how to set warning flags
set_more_warnings=no
AC_ARG_ENABLE(warnings,
        AS_HELP_STRING(
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
                         -Wlifetime \
                         -Wsuggest-attribute=const \
                         -Wsuggest-attribute=pure \
                         -Wsuggest-final-types \
                         -Wsuggest-final-methods \
                         -Wsuggest-override \
                         -Wcast-qual -Wcast-align=strict \
                         -Wzero-as-null-pointer-constant \
                         -Wparentheses \
                         -Wdate-time \
                         -Wextra-semi \
                         -Wbiznafronck"

        #these warnings were tried but deemed unuseful for this project:
        #                -Wunsafe-loop-optimizations \

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

dnl check for fallthrough. this is tricky to use, because it may give a warning
dnl on some compilers (clang in c++11, pedantic mode with warnings enabled)
dnl therefore, do not use this for now.
AC_MSG_CHECKING([check for fallthrough support])
define([str],[[[[fallthrough]]]])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
 int f() {
  switch(1){
  case 1:
  [[fallthrough]];
  case 2:
  return 2;
  };
 }
]], [])],
            [has_fallthrough=yes],
            [has_fallthrough=no])
AC_MSG_RESULT($has_fallthrough)
if test $has_fallthrough = yes; then
AC_DEFINE(FALLTHROUGH, [str], "support for c++17 fallthrough")
else
AC_DEFINE(FALLTHROUGH, [], "support for c++17 fallthrough")
fi

dnl valgrind support
dnl unfortunately it makes checking much slower
dnl even if valgrind is not used, so leave it inactive.
dnl AX_VALGRIND_CHECK
dnl
dnl instead, just use "VALGRIND=valgrind make check" to test with valgrind
dnl read Makefile.in and write Makefile
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
