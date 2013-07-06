#This is the spec file for rdfind. It has been prepared following the guide
#at  http://club.mandriva.com/xwiki/bin/KB/MandrivaRpmHowTo
#
# this spec-file is in cvs. $Revision: 91 $
%define name    rdfind 
%define version 1.2.1
%define release %mkrel 1

Name: %{name} 
Summary: This is a program that finds duplicate files
Version: %{version} 
Release: %{release} 
Source0: http://www.paulsundvall.net/rdfind/%{name}-%{version}.tar.bz2 
URL: http://www.paulsundvall.net

Group: File tools 
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot 
License: GPL 
Packager: Paul Sundvall <rdfind@paulsundvall.net>

%description
This is a small program that searches for duplicate files using
a lazy algoithm to gain speed. Optionally, duplicate files can be
removed or replaced with links (hard or symbolic). 

%prep 
%setup -q -a 0 

%build 
%configure 
%make

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
* Sun Mar 19 2006 Paul Sundvall <rdfind@paulsundvall.net> 1.2.1-0.1.20060mdk
- updated to 1.2.1.
* Sun Mar 19 2006 Paul Sundvall <rdfind@paulsundvall.net> 1.2.0-0.1.20060mdk
- Initial creation

