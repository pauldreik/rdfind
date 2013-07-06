#!/bin/bash

#$Date: 2006-11-26 12:17:24 +0100 (sön, 26 nov 2006) $
#$Rev: 238 $
#$Author: pauls $
#$Id: create_package.sh 238 2006-11-26 11:17:24Z pauls $

echo "this is made from  http://people.connexer.com/~roberto/howtos/debrepository and works, except that it wont be signed."

sourcedirs="$HOME/tmp/debianize_rdfind/"
repdir=$HOME/tmp/debian_repository

mkdir -p $repdir


pushd $repdir

rm packages*
rm *.conf
rm dists/* -rf
rm binary/* -rf
rm source/* -rf
rmdir binary*
rmdir source*
rmdir dists
rm Release Packages Release.gz Packages.gz



#from http://people.connexer.com/~roberto/howtos/debrepository

echo will now create the directory structure

mkdir -p dists
mkdir -p dists/sarge
#mkdir -p dists/sarge/contrib
#mkdir -p dists/sarge/contrib/binary-i386
#mkdir -p dists/sarge/contrib/source
mkdir -p dists/sarge/main
mkdir -p dists/sarge/main/binary-i386
mkdir -p dists/sarge/main/source
#mkdir -p dists/sarge/non-free
#mkdir -p dists/sarge/non-free/binary-i386
#mkdir -p dists/sarge/non-free/source



echo will now move the packages to the correct places

for srcdir in $sourcedirs; do
find $srcdir -type f  -name "*.deb" -exec cp {} dists/sarge/main/binary-i386 \; -print 
find $srcdir -type f  -a \(  -name "*.orig.gz" -o -name "*.diff.gz" -o -name "*.dsc" \) -exec cp {} dists/sarge/main/source \; -print 
done



echo 'APT::FTPArchive::Release::Origin "Your name or organization";
APT::FTPArchive::Release::Label "Descriptive label";
APT::FTPArchive::Release::Suite "stable";
APT::FTPArchive::Release::Codename "sarge";
APT::FTPArchive::Release::Architectures "i386 source";
APT::FTPArchive::Release::Components "main";
APT::FTPArchive::Release::Description "More detailed description";
'>apt-sarge-release.conf

echo 'Dir {
  ArchiveDir ".";
  CacheDir ".";
};

Default {
  Packages::Compress ". gzip bzip2";
  Sources::Compress "gzip bzip2";
  Contents::Compress "gzip bzip2";
};

BinDirectory "dists/sarge/main/binary-i386" {
  Packages "dists/sarge/main/binary-i386/Packages";
  Contents "dists/sarge/Contents-i386";
  SrcPackages "dists/sarge/main/source/Sources";
};

  
Tree "dists/sarge" {
  Sections "main";
  Architectures "i386 source";
};

Default {
  Packages {
    Extensions ".deb";
  };
};
'>apt-ftparchive.conf

apt-ftparchive generate apt-ftparchive.conf
apt-ftparchive -c apt-sarge-release.conf release dists/sarge/ >dists/sarge/Release

gpg -u 0x509CCB46 --sign -ba -o dists/sarge/Release.gpg dists/sarge/Release
exit

ssh atle 'rm -rf /var/www/debian/*'
scp -r * atle:/var/www/debian/ 
