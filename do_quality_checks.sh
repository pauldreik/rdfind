#!/bin/sh
#
# this script tries to do some quality checks
# automatically. It counts compiler warnings,
# builds in both debug/release mode, test multiple
# compilers etc.
#
# To get the most out of it, install as many variants of gcc and clang
# as you can. If g++ is found, it will look for g++-* in the same directory.
# If clang++ is found, it will look for clang++-* in the same directory.
# This means you need to have either system wide installs, or your PATH is
# setup in such a way that g++/clang++ points to the same location as the compilers
# you want to test.
#
# If clang is available, builds with address and undefined sanitizer will be made.
#
# If clang is available and possible to use with libc++, it will be built. On Ubuntu,
# install libc++abi-dev and libc++-dev
#
# If valgrind is available, it will be added as one thing to test.
#
# If dpkg-buildflags is available, a test build will be added with the flags
# coming from that tool.
#
# A build with debug iterators (https://gcc.gnu.org/onlinedocs/libstdc++/manual/debug_mode.html)
# is made.
#
# All compiles are checked to be warning free, all unit tests should pass.
#
# LICENSE: GPLv2 or later, at your option.
# by Paul Dreik 20181014

set -e

export LANG=

rootdir=$(dirname $0)

#flags to configure, for assert.
ASSERT=

###############################################################################

start_from_scratch() {
cd $rootdir
if [ -e Makefile ] ; then
   make distclean >/dev/null 2>&1
fi

}
###############################################################################
#argument 1 is the compiler
#argument 2 is the c++ standard
#argument 3 (optional) is appended to CXXFLAGS
compile_and_test_standard() {
start_from_scratch
/bin/echo -n "using $(basename $1) with standard $2"
if [ -n "$3" ] ; then
  echo " (with additional CXXFLAGS $3)"
else
  echo ""
fi

if ! ./bootstrap.sh >bootstrap.log 2>&1; then
  echo failed bootstrap - see bootstrap.log
  exit 1
fi
if ! ./configure $ASSERT --enable-warnings CXX=$1 CXXFLAGS="-std=$2 $3" >configure.log 2>&1 ; then
  echo failed configure - see configure.log
  exit 1
fi
#make sure it compiles
if ! /usr/bin/time --format=%e --output=time.log make >make.log 2>&1; then
echo failed make
exit 1
fi
if [ ! -z $MEASURE_COMPILE_TIME ] ; then
  echo "  compile with $(basename $1) $2 took $(cat time.log) seconds"
fi
#check for warnings
if grep -q "warning" make.log; then
 echo found warning - see make.log
 exit 1
fi
#run the tests
if ! make check >makecheck.log 2>&1 ; then
   echo failed make check - see makecheck.log
   exit 1
fi
}
###############################################################################
#argument 1 is the compiler
compile_and_test() {
#this is the test program to compile, so we know the compiler and standard lib
#works. clang 4 with c++2a does not.
/bin/echo -e "#include <iostream>">x.cpp
#does the compiler understand c++11? That is mandatory.
if ! $1 -c x.cpp -std=c++11 >/dev/null 2>&1 ; then
  echo this compiler $1 does not understand c++11
  return 0
fi

#loop over all standard flags>=11 and try those which work.
#use the code words.
for std in 11 1y 1z 2a ; do
if ! $1 -c x.cpp -std=c++$std >/dev/null 2>&1 ; then
  echo compiler does not understand c++$std, skipping this combination.
else
    # debug build
    ASSERT=--enable-assert
    compile_and_test_standard $1 c++$std "-Og"

    # release build
    ASSERT=--disable-assert
    compile_and_test_standard $1 c++$std "-O2"
    compile_and_test_standard $1 c++$std "-O3"
    compile_and_test_standard $1 c++$std "-Os"
fi
done
}
###############################################################################

run_with_sanitizer() {
echo "running with sanitizer (options $1)"
#find the latest clang compiler
latestclang=$(ls $(which clang++)* |grep -v libc |sort -g |tail -n1)
if [ ! -x $latestclang ] ; then
  echo could not find latest clang $latestclang
  return 0
fi

start_from_scratch
./bootstrap.sh >bootstrap.log
./configure $ASSERT CXX=$latestclang CXXFLAGS="-std=c++1y $1"   >configure.log
make > make.log 2>&1
export UBSAN_OPTIONS="halt_on_error=true exitcode=1"
export ASAN_OPTIONS="halt_on_error=true exitcode=1"
make check >make-check.log 2>&1
unset UBSAN_OPTIONS
unset ASAN_OPTIONS
}
###############################################################################
#This tries to mimick how the debian package is built
run_with_debian_buildflags() {
echo "running with buildflags from debian dpkg-buildflags"
if ! which dpkg-buildflags >/dev/null  ; then
  echo dpkg-buildflags not found - skipping
  return 0
fi
start_from_scratch
./bootstrap.sh >bootstrap.log
eval $(DEB_BUILD_MAINT_OPTIONS="hardening=+all qa=+all,-canary reproducible=+all" dpkg-buildflags --export=sh)
./configure  >configure.log
make > make.log 2>&1
#check for warnings
if grep -q "warning" make.log; then
 echo "found warning(s) - see make.log"
 exit 1
fi
make check >make-check.log 2>&1

#restore the build environment
for flag in $(dpkg-buildflags  |cut -f1 -d=) ; do
  unset $flag
done 
}
###############################################################################
run_with_libcpp() {
latestclang=$(ls $(which clang++)* |grep -v libc|sort -g |tail -n1)
if [ ! -x $latestclang ] ; then
  echo could not find latest clang - skipping test with libc++
  return 0
fi
#make a test program to make sure it works.
echo "#include <iostream>
int main() { std::cout<<\"libc++ works!\";}" >x.cpp
if ! $latestclang -std=c++11 -stdlib=libc++ -lc++abi x.cpp >/dev/null 2>&1 && [ -x ./a.out ] && ./a.out ; then
  echo "$latestclang could not compile with libc++ - perhaps uninstalled."
  return 0
fi
#echo using $latestclang with libc++
compile_and_test_standard $latestclang c++11 "-stdlib=libc++ -D_LIBCPP_DEBUG=1"
}
###############################################################################

#keep track of which compilers have already been tested
echo "">inodes_for_tested_compilers.txt

#try all variants of g++
if which g++ >/dev/null ; then
  for COMPILER in $(ls $(which g++)* |grep -v libc); do
    inode=$(stat --dereference --format=%i $COMPILER)
    if grep -q "^$inode\$" inodes_for_tested_compilers.txt ; then
       echo skipping this compiler $COMPILER - already tested
    else
      #echo trying gcc $GCC:$($GCC --version|head -n1)
       echo $inode >>inodes_for_tested_compilers.txt
       compile_and_test $COMPILER
    fi
  done
fi

#try all variants of clang
if which clang++ >/dev/null ; then
  for COMPILER in $(ls $(which clang++)* |grep -v libc); do
    inode=$(stat --dereference --format=%i $COMPILER)
    if grep -q "^$inode\$" inodes_for_tested_compilers.txt ; then
       echo skipping this compiler $COMPILER - already tested
    else
      #echo trying gcc $GCC:$($GCC --version|head -n1)
       echo $inode >>inodes_for_tested_compilers.txt
       compile_and_test $COMPILER
    fi
  done
fi

#run unit tests with sanitizers enabled
ASSERT="--enable-asserts"
run_with_sanitizer "-fsanitize=undefined -O3"
run_with_sanitizer "-fsanitize=address -O0"

#build and test with all flags from debian, if available. this increases
#the likelilihood rdfind will build when creating a deb package.
ASSERT=""
run_with_debian_buildflags

#make a test build with debug iterators
ASSERT="--enable-asserts"
compile_and_test_standard g++ c++11 "-D_GLIBCXX_DEBUG"

#test run with clang/libc++
ASSERT="--enable-asserts"
run_with_libcpp
ASSERT="--disable-asserts"
run_with_libcpp

#test build with running through valgrind
if which valgrind >/dev/null; then
  echo running unit tests through valgrind
  ASSERT="--disable-asserts"
  compile_and_test_standard g++ c++11 "-O3"
  VALGRIND=valgrind make check >make-check.log
fi

echo "$(basename $0): congratulations, all tests that were possible to run passed!"


