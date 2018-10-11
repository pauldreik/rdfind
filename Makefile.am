# copyright 20016-2017 Paul Dreik (earlier Paul Sundvall)
# Distributed under GPL v 2.0 or later, at your option.
# See LICENSE for further details.
AUTOMAKE_OPTIONS = gnu # I would like dist-bzip2 here, but automake complains
bin_PROGRAMS = rdfind
rdfind_SOURCES = rdfind.cc Checksum.cc  Dirlist.cc  Fileinfo.cc  Rdutil.cc \
                 EasyRandom.cc UndoableUnlink.cc

#these are the test scripts to execute - I do not know how to glob here,
#feedback welcome.
TESTS=testcases/largefilesupport.sh \
      testcases/md5collisions.sh \
      testcases/hardlink_fails.sh \
      testcases/symlinking_action.sh 


EXTRA_DIST = \
  algos.hh Dirlist.hh MultiAttributeCompare.hh Checksum.hh  Fileinfo.hh \
  Rdutil.hh bootstrap.sh RdfindDebug.hh EasyRandom.hh UndoableUnlink.hh \
  $(TESTS) \
  rdfind.1 LICENSE \
  ./do_clang_format.sh .clang-format

man_MANS = rdfind.1


#for formatting the source
format:
	./do_clang_format.sh

