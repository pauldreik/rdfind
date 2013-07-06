#This is the spec file for rdfind. It has been prepared following the guide
#at  http://club.mandriva.com/xwiki/bin/KB/MandrivaRpmHowTo
# To make it work with redhat, %make had to be replaced with make.
#
# this spec-file is in cvs. $Revision: 666 $
#
#to build with rpm on debian (or fedora, I guess):
#
#rpmbuild -ba SPECS/rdfind.spec --sign --target i586 --target i386
%define name    rdfind 
%define version 1.2.2
%define release 1

Name: %{name} 
Summary: This is a program that finds duplicate files
Version: %{version} 
Release: %{release} 
Source0: http://rdfind.paulsundvall.net/rdfind/%{name}-%{version}.tar.gz
URL: http://rdfind.paulsundvall.net

Group: File tools 
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot 
License: GPL 
Packager: Paul Dreik <rdfind@paulsundvall.net>

%description
This is a small program that searches for duplicate files using
a lazy algoithm to gain speed. Optionally, duplicate files can be
removed or replaced with links (hard or symbolic). 

%prep 
%setup -q -a 0 

%build 
%configure 
make

%install 
rm -rf $RPM_BUILD_ROOT 
%makeinstall

%clean 
rm -rf $RPM_BUILD_ROOT

%files 
%defattr(-,root,root,0755) 
%doc README NEWS COPYING AUTHORS 
%{_mandir}/man1/rdfind.1* 
%{_bindir}/rdfind 

%changelog 
* Sun Mar 26 2006 Paul Sundvall <rdfind@paulsundvall.net> 1.2.2-1
- updated to 1.2.2
- error in symlinks treated
- examples in the manual
* Sun Mar 19 2006 Paul Sundvall <rdfind@paulsundvall.net> 1.2.1-0.1.20060mdk
- updated to 1.2.1.
* Sun Mar 19 2006 Paul Sundvall <rdfind@paulsundvall.net> 1.2.0-0.1.20060mdk
- Initial creation

