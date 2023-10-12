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

rootdir=$(dirname "$0")
me=$(basename "$0")

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
   /bin/echo -n "$me: using $(basename $1) with standard $2"
   if [ -n "$3" ] ; then
      echo " (with additional CXXFLAGS $3)"
   else
      echo ""
   fi

   if ! ./bootstrap.sh >bootstrap.log 2>&1; then
      echo $me:failed bootstrap - see bootstrap.log
      exit 1
   fi
   if ! ./configure $ASSERT --enable-warnings CXX=$1 CXXFLAGS="-std=$2 $3" >configure.log 2>&1 ; then
      echo $me: failed configure - see configure.log
      exit 1
   fi
   #make sure it compiles
   if [ ! -x /usr/bin/time ] ; then
      echo "$me: please install /usr/bin/time (apt install time)"
      exit 1
   fi
   if ! /usr/bin/time --format=%e --output=time.log make >make.log 2>&1; then
      echo $me: failed make
      exit 1
   fi
   if [ ! -z $MEASURE_COMPILE_TIME ] ; then
      echo $me: "  compile with $(basename $1) $2 took $(cat time.log) seconds"
   fi
   #check for warnings
   if grep -q "warning" make.log; then
      # store as an artifact instead of erroring out
      name=$(cat *.log |sha256sum |head -c 12)
      cp make.log make_${name}.log
      echo $me: found compile warning - see make.log, also stored as make_${name}.log
   fi
   #run the tests
   if ! make check >makecheck.log 2>&1 ; then
      echo $me: failed make check - see makecheck.log
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
      echo $me: this compiler $1 does not understand c++11
      return 0
   fi

   #loop over all standard flags>=11 and try those which work.
   #use the code words.
   for std in 11 1y 1z 2a 2b ; do
      if ! $1 -c x.cpp -std=c++$std >/dev/null 2>&1 ; then
         echo $me: compiler does not understand c++$std, skipping this combination.
      else
         # debug build
         ASSERT=--enable-assert
         compile_and_test_standard $1 c++$std "-Og"

         # release build
         ASSERT=--disable-assert
         #compile_and_test_standard $1 c++$std "-O2"
         compile_and_test_standard $1 c++$std "-O3"
         #compile_and_test_standard $1 c++$std "-Os"
      fi
   done

   rm x.cpp
}
###############################################################################
run_with_sanitizer() {
   echo $me: "running with sanitizer (options $1)"
   #find the latest clang compiler
   latestclang=$(ls $(which clang++)* |grep -v libc |sort -g |tail -n1)
   if [ ! -x $latestclang ] ; then
      echo $me: could not find latest clang $latestclang
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
#This tries to mimic how the debian package is built
run_with_debian_buildflags() {
   echo $me: "running with buildflags from debian dpkg-buildflags"
   if ! which dpkg-buildflags >/dev/null  ; then
      echo $me: dpkg-buildflags not found - skipping
      return 0
   fi
   start_from_scratch
   ./bootstrap.sh >bootstrap.log
   eval $(DEB_BUILD_MAINT_OPTIONS="hardening=+all qa=+all,-canary reproducible=+all" dpkg-buildflags --export=sh)
   ./configure  >configure.log
   make > make.log 2>&1
   #check for warnings
   if grep -q "warning" make.log; then
      # store as an artifact instead of erroring out
      name=$(cat *.log |sha256sum |head -c 12)
      cp make.log make_${name}.log
      echo $me: found compile warnings - see make.log, also stored as make_${name}.log

   fi
   make check >make-check.log 2>&1

   #restore the build environment
   for flag in $(dpkg-buildflags  |cut -f1 -d=) ; do
      unset $flag
   done
}
###############################################################################
run_with_libcpp() {
   # find a clang that works. try the latest version first.
   echo "#include <iostream>
  int main() { std::cout<<\"libc++ works!\";}" >x.cpp
   for clang in $(ls $(which clang++)* |grep -v libc|sort -g --reverse) ; do
      if [ ! -x $clang ] ; then
         continue
      fi

      if ! $clang -std=c++11 -stdlib=libc++ -lc++abi x.cpp >/dev/null 2>&1 || [ ! -x ./a.out ] || ! ./a.out ; then
         echo $me: "debug: $clang could not compile with libc++ - perhaps uninstalled."
         continue
      fi
      compile_and_test_standard $clang c++11 "-stdlib=libc++ -D_LIBCPP_DEBUG=1"
      return
   done
   # we will get here if no working clang could be found. that is not an error,
   # having clang and libc++ installed is optional
   echo $me: no working clang with libc++ found, skipping.
}
###############################################################################

verify_packaging() {
   #make sure the packaging works as intended.
   echo $me: "trying to make a tar ball for release and building it..."
   log="$(pwd)/packagetest.log"
   ./bootstrap.sh >$log
   ./configure  >>$log

   touch dummy
   make dist  >>$log
   TARGZ=$(find "$(pwd)" -newer dummy -name "rdfind*gz" -type f |head -n1)
   rm dummy
   temp=$(mktemp -d)
   cp "$TARGZ" "$temp"
   cd "$temp"
   tar xzf $(basename "$TARGZ")  >>$log
   cd $(basename "$TARGZ" .tar.gz)
   ./configure --prefix=$temp  >>$log
   make  >>$log
   make check  >>$log
   make install  >>$log
   $temp/bin/rdfind --version   >>$log
   #coming here means all went fine, go back to the source dir.
   cd $(dirname "$TARGZ")
   rm -rf "$temp"
}

###############################################################################
verify_self_contained_headers() {
   /bin/echo -n "$me: verify that all header files are self contained..."
   if [ ! -e configure ]; then
      ./bootstrap.sh >bootstrap.log 2>&1
   fi
   if [ ! -e config.h ]; then
      ./configure >configure.log 2>&1
   fi
   for header in *.hh ; do
      cp $header tmp.cc
      if ! g++ -std=c++11 -I. -c tmp.cc -o /dev/null >header.log 2>&1 ; then
         echo "$me: found a header which is not self contained: $header."
         echo "$me: see header.log for details"
         exit 1
      fi
      rm tmp.cc
   done
   echo "OK!"
}

###############################################################################
build_32bit() {
   #compiling to 32 bit, on amd64.
   #apt install libc6-i386 gcc-multilib g++-multilib
   #
   if [ $(uname -m) != x86_64 ] ; then
      echo $me: "not on x64, won't cross compile with -m32"
      return;
   fi
   echo $me: "trying to compile in 32 bit mode with -m32..."
   configureflags="--build=i686-pc-linux-gnu CFLAGS=-m32 CXXFLAGS=-m32 LDFLAGS=-m32"
   here=$(pwd)
   nettleinstall=$here/nettle32bit
   if [ -d "$nettleinstall" ] ; then
      echo $me: "local nettle already seems to be installed"
   else
      mkdir "$nettleinstall"
      cd "$nettleinstall"
      nettleversion=3.7.3
      echo "$me: downloading nettle from gnu.org..."
      wget --quiet https://ftp.gnu.org/gnu/nettle/nettle-$nettleversion.tar.gz
      echo "661f5eb03f048a3b924c3a8ad2515d4068e40f67e774e8a26827658007e3bcf0  nettle-$nettleversion.tar.gz" >checksum
      sha256sum --strict --quiet -c checksum
      tar xzf nettle-$nettleversion.tar.gz
      cd nettle-$nettleversion
      echo $me: trying to configure nettle
      ./configure $configureflags --prefix="$nettleinstall" >$here/nettle.configure.log 2>&1
      make install >$here/nettle.install.log 2>&1
      echo $me: "local nettle install went ok"
      cd $here
   fi
   ./bootstrap.sh >bootstrap.log 2>&1
   echo "$me: attempting configure with 32 bit flags... (see configure.log if it fails)"
   ./configure --build=i686-pc-linux-gnu CFLAGS=-m32 CXXFLAGS="-m32 -I$nettleinstall/include" LDFLAGS="-m32 -L$nettleinstall/lib" >configure.log 2>&1
   echo "$me: building with 32 bit flags... (check make.log if it fails)"
   make >make.log 2>&1
   echo "$me: make check with 32 bit flags... (check make-check.log if it fails)"
   LD_LIBRARY_PATH=$nettleinstall/lib make check >make-check.log 2>&1
   echo "$me: 32 bit tests went fine!"
}
###############################################################################


#this is pretty quick so start with it.
verify_self_contained_headers

#keep track of which compilers have already been tested
echo "">inodes_for_tested_compilers.txt

#try all variants of g++
if which g++ >/dev/null ; then
   for COMPILER in $(ls $(which g++)* |grep -v libc); do
      inode=$(stat --dereference --format=%i $COMPILER)
      if grep -q "^$inode\$" inodes_for_tested_compilers.txt ; then
         echo $me: skipping this compiler $COMPILER - already tested
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
         echo $me: skipping this compiler $COMPILER - already tested
      else
         #echo trying gcc $GCC:$($GCC --version|head -n1)
         echo $inode >>inodes_for_tested_compilers.txt
         compile_and_test $COMPILER
      fi
   done
fi

rm inodes_for_tested_compilers.txt

#run unit tests with sanitizers enabled
ASSERT="--enable-assert"
run_with_sanitizer "-fsanitize=undefined -O3"
run_with_sanitizer "-fsanitize=address -O0"

#build and test with all flags from debian, if available. this increases
#the likelilihood rdfind will build when creating a deb package.
ASSERT=""
run_with_debian_buildflags

#make a test build with debug iterators
ASSERT="--enable-assert"
compile_and_test_standard g++ c++11 "-D_GLIBCXX_DEBUG"

#test run with clang/libc++
ASSERT="--enable-assert"
run_with_libcpp
ASSERT="--disable-assert"
run_with_libcpp

#test build with running through valgrind
if which valgrind >/dev/null; then
   echo $me: running unit tests through valgrind
   ASSERT="--disable-assert"
   compile_and_test_standard g++ c++11 "-O3"
   VALGRIND=valgrind make check >make-check.log
fi

#make sure it is possible to build a tar ball,
#unpack it, build and execute tests, then finally
#installing and running the program.
verify_packaging

#try to compile to 32 bit (downloads nettle and builds it in 32 bit mode)
build_32bit

echo "$me: congratulations, all tests that were possible to run passed!"
