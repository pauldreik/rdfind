#!/bin/sh
#
# ensures rdfind builds for x86 (32 bit)

set -eux

srcroot="$(
  cd "$(dirname "$0")"
  cd ..
  pwd -P
)"
me="$(basename "$0")"

if [ "$(uname -m)" != x86_64 ]; then
  echo "$me: not on x64, won't cross compile with -m32"
  return
fi

echo "$me: trying to compile in 32 bit mode with -m32..."
configureflags="--build=i686-pc-linux-gnu CFLAGS=-m32 CXXFLAGS=-m32 LDFLAGS=-m32"

WD=$(mktemp -d rdfind.nettle.XXXXXXXXXX)
cd "$WD"
here=$(pwd)
echo "*" >.gitignore

nettleversion=4.0
nettleinstall="$(pwd)/nettle-$nettleversion"
mkdir "$nettleinstall"
cd "$nettleinstall"
echo "$me: downloading nettle from gnu.org..."
wget --quiet https://ftp.gnu.org/gnu/nettle/nettle-$nettleversion.tar.gz
echo "3addbc00da01846b232fb3bc453538ea5468da43033f21bb345cb1e9073f5094  nettle-$nettleversion.tar.gz" >checksum
sha256sum --strict --quiet -c checksum
tar xzf nettle-$nettleversion.tar.gz
cd nettle-$nettleversion
echo "$me: trying to configure nettle"
# shellcheck disable=SC2086
./configure $configureflags --prefix="$nettleinstall" 2>&1 | tee "$nettleinstall/nettle.configure.log"
make -j "$(nproc)" install >"$nettleinstall/nettle.install.log" 2>&1
echo "$me: local nettle install went ok"
cd "$here"

(
  cd "$srcroot"
  ./bootstrap.sh
)
"$srcroot/configure" --build=i686-pc-linux-gnu CFLAGS=-m32 CXXFLAGS="-m32 -I $nettleinstall/include" LDFLAGS="-L $nettleinstall/lib"
make -j "$(nproc)"

LD_LIBRARY_PATH="$nettleinstall/lib" make check

echo "$me: all is good"

rm -rf "$WD"
