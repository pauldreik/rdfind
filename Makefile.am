# copyright 2006-2018 Paul Dreik (earlier Paul Sundvall)
# Distributed under GPL v 2.0 or later, at your option.
# See LICENSE for further details.
AUTOMAKE_OPTIONS = gnu # I would like dist-bzip2 here, but automake complains
bin_PROGRAMS = rdfind
rdfind_SOURCES = rdfind.cc Checksum.cc  Dirlist.cc  Fileinfo.cc  Rdutil.cc \
                 EasyRandom.cc UndoableUnlink.cc CmdlineParser.cc

#these are the test scripts to execute - I do not know how to glob here,
#feedback welcome.
TESTS=testcases/largefilesupport.sh \
      testcases/hardlink_fails.sh \
      testcases/symlinking_action.sh \
      testcases/verify_filesize_option.sh \
      testcases/verify_maxfilesize_option.sh \
      testcases/verify_dryrun_option.sh \
      testcases/verify_ranking.sh \
      testcases/verify_deterministic_operation.sh \
      testcases/checksum_options.sh \
      testcases/md5collisions.sh \
      testcases/sha1collisions.sh \
      testcases/verify_outputdelimiter_option.sh

AUXFILES=testcases/common_funcs.sh \
         testcases/md5collisions/letter_of_rec.ps \
         testcases/md5collisions/order.ps \
         testcases/sha1collisions/coll.tar.bz2.b64 \
         testcases/sha1collisions/README.txt


#valgrind support. unfortunately it makes checking much slower
#even if valgrind is not used, so leave it inactive.
#@VALGRIND_CHECK_RULES@
#TEST_EXTENSIONS = .sh
#LOG_COMPILER = $(VALGRIND)
#TESTS_ENVIRONMENT =  VALGRIND='$(VALGRIND)'

EXTRA_DIST = \
  Dirlist.hh Checksum.hh  Fileinfo.hh \
  Rdutil.hh bootstrap.sh RdfindDebug.hh EasyRandom.hh UndoableUnlink.hh \
  CmdlineParser.hh \
  $(TESTS) \
  $(AUXFILES) \
  rdfind.1 LICENSE \
  ./do_clang_format.sh .clang-format

man_MANS = rdfind.1


#for formatting the source
format:
	./do_clang_format.sh

