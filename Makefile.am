AUTOMAKE_OPTIONS = gnu # I would like dist-bzip2 here, but automake complains
bin_PROGRAMS = rdfind
rdfind_SOURCES = rdfind.cc Checksum.cc  Dirlist.cc  Fileinfo.cc  Rdutil.cc
TESTS=testcases/*.sh
EXTRA_DIST = algos.hh Dirlist.hh MultiAttributeCompare.hh Checksum.hh  Fileinfo.hh Rdutil.hh rdfind.1 bootstrap.sh RdfindDebug.hh $(TESTS)

man_MANS = rdfind.1



