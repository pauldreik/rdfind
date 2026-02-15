#include "config.h" //header file from autoconf, must come first to make large file support work properly

#include <iostream>
#include <limits>

#include "CmdlineParser.hh"
#include "Options.hh"

namespace {
constexpr auto usagetext = R"(
Usage: rdfind [options] FILE ...

Finds duplicate files recursively in the given FILEs (directories), and takes
appropriate action (by default, nothing). Directories listed first are ranked
higher, meaning that if a file is found on several places, the file found in
the directory first encountered on the command line is kept, and the others are
considered duplicate.

options are (default choice within parentheses)

 Searching options:

 -ignoreempty      (true)| false  ignore empty files (true implies -minsize 1,
                                  false implies -minsize 0)
 -minsize N        (N=1)          ignores files with size less than N bytes
 -maxsize N        (N=0)          ignores files with size N bytes and larger
                                  (use 0 to disable this check).
 -followsymlinks    true |(false) follow symlinks
 -removeidentinode (true)| false  ignore files with nonunique device and inode

 Processing options:

 -checksum          none | md5 |(sha1)| sha256 | sha512 | xxh128
                                  checksum type
                                  xxh128 is very fast, but is noncryptographic.
 -buffersize N                    chunksize in bytes when calculating the
                                  checksum. The default is 1 MiB, can be up
                                  to 128 MiB.
 -deterministic    (true)| false  makes results independent of order
                                  from listing the filesystem

 Action options:

 -makeresultsfile  (true)| false  makes a results file
 -makesymlinks      true |(false) replace duplicate files with symbolic links
 -makehardlinks     true |(false) replace duplicate files with hard links
 -deleteduplicates  true |(false) delete duplicate files
                                  Default is 0. Only a few values
                                  are supported; 0, 1-5, 10, 25, 50, 100
 -dryrun|-n         true |(false) print to stdout instead of changing anything

 General options:

 -outputname NAME                 sets the results file name to NAME,
                                  default is results.txt
 -sleep             Xms           sleep for X milliseconds between file reads.
 -progress          true |(false) output progress information
 -h|-help|--help                  show this help and exit
 -v|--version                     display version number and exit

If properly installed, a man page should be available as man rdfind.

rdfind is written by Paul Dreik 2006 onwards.
License: GPL v2 or later (at your option).
)";

}

void
usage()
{
  std::cout << usagetext + 1 << "version is " << VERSION << '\n';
}

Options
parseOptions(Parser& parser)
{
  Options o;
  for (; parser.has_args_left(); parser.advance()) {
    // empty strings are forbidden as input since they can not be file names or
    // options
    if (parser.get_current_arg()[0] == '\0') {
      std::cerr << "bad argument " << parser.get_current_index() << '\n';
      std::exit(EXIT_FAILURE);
    }

    // if we reach the end of the argument list - exit the loop and proceed with
    // the file list instead.
    if (parser.get_current_arg()[0] != '-') {
      // end of argument list - exit!
      break;
    }
    if (parser.try_parse_bool("-makesymlinks")) {
      o.makesymlinks = parser.get_parsed_bool();
    } else if (parser.try_parse_bool("-makehardlinks")) {
      o.makehardlinks = parser.get_parsed_bool();
    } else if (parser.try_parse_bool("-makeresultsfile")) {
      o.makeresultsfile = parser.get_parsed_bool();
    } else if (parser.try_parse_string("-outputname")) {
      o.resultsfile = parser.get_parsed_string();
    } else if (parser.try_parse_bool("-ignoreempty")) {
      if (parser.get_parsed_bool()) {
        o.minimumfilesize = 1;
      } else {
        o.minimumfilesize = 0;
      }
    } else if (parser.try_parse_string("-minsize")) {
      const long long minsize = std::stoll(parser.get_parsed_string());
      if (minsize < 0) {
        throw std::runtime_error("negative value of minsize not allowed");
      }
      o.minimumfilesize = minsize;
    } else if (parser.try_parse_string("-maxsize")) {
      const long long maxsize = std::stoll(parser.get_parsed_string());
      if (maxsize < 0) {
        throw std::runtime_error("negative value of maxsize not allowed");
      }
      o.maximumfilesize = maxsize;
    } else if (parser.try_parse_bool("-deleteduplicates")) {
      o.deleteduplicates = parser.get_parsed_bool();
    } else if (parser.try_parse_bool("-followsymlinks")) {
      o.followsymlinks = parser.get_parsed_bool();
    } else if (parser.try_parse_bool("-dryrun")) {
      o.dryrun = parser.get_parsed_bool();
    } else if (parser.try_parse_bool("-n")) {
      o.dryrun = parser.get_parsed_bool();
    } else if (parser.try_parse_bool("-removeidentinode")) {
      o.remove_identical_inode = parser.get_parsed_bool();
    } else if (parser.try_parse_bool("-deterministic")) {
      o.deterministic = parser.get_parsed_bool();
    } else if (parser.try_parse_string("-checksum")) {
      if (parser.parsed_string_is("md5")) {
        o.usemd5 = true;
      } else if (parser.parsed_string_is("sha1")) {
        o.usesha1 = true;
      } else if (parser.parsed_string_is("sha256")) {
        o.usesha256 = true;
      } else if (parser.parsed_string_is("sha512")) {
        o.usesha512 = true;
      } else if (parser.parsed_string_is("xxh128")) {
#ifdef HAVE_LIBXXHASH
        o.usexxh128 = true;
#else
        std::cerr << "not compiled with xxhash, to make use of xxh128 please "
                     "reconfigure and rebuild '--with-xxhash'\n";
        std::exit(EXIT_FAILURE);
#endif
      } else if (parser.parsed_string_is("none")) {
        std::cout
          << "DANGER! -checksum none given, will skip the checksumming stage\n";
        o.nochecksum = true;
      } else {
        std::cerr << "expected none/md5/sha1/sha256/sha512/xxh128, not \""
                  << parser.get_parsed_string() << "\"\n";
        std::exit(EXIT_FAILURE);
      }
    } else if (parser.try_parse_string("-buffersize")) {
      const long buffersize = std::stoll(parser.get_parsed_string());
      constexpr long max_buffersize = 128 << 20;
      if (buffersize <= 0) {
        std::cerr << "a negative or zero buffersize is not allowed\n";
        std::exit(EXIT_FAILURE);
      } else if (buffersize > max_buffersize) {
        std::cerr << "a maximum of " << (max_buffersize >> 20)
                  << " MiB buffersize is allowed, got " << (buffersize >> 20)
                  << " MiB\n";
        std::exit(EXIT_FAILURE);
      }
      o.buffersize = static_cast<std::size_t>(buffersize);
    } else if (parser.try_parse_string("-sleep")) {
      const auto nextarg = std::string(parser.get_parsed_string());
      if (nextarg == "1ms") {
        o.nsecsleep = 1000000;
      } else if (nextarg == "2ms") {
        o.nsecsleep = 2000000;
      } else if (nextarg == "3ms") {
        o.nsecsleep = 3000000;
      } else if (nextarg == "4ms") {
        o.nsecsleep = 4000000;
      } else if (nextarg == "5ms") {
        o.nsecsleep = 5000000;
      } else if (nextarg == "10ms") {
        o.nsecsleep = 10000000;
      } else if (nextarg == "25ms") {
        o.nsecsleep = 25000000;
      } else if (nextarg == "50ms") {
        o.nsecsleep = 50000000;
      } else if (nextarg == "100ms") {
        o.nsecsleep = 100000000;
      } else {
        std::cerr << "sorry, can only understand a few sleep values for "
                     "now. \""
                  << nextarg << "\" is not among them.\n";
        std::exit(EXIT_FAILURE);
      }
    } else if (parser.try_parse_bool("-progress")) {
      o.showprogress = parser.get_parsed_bool();
    } else if (parser.current_arg_is("-help") || parser.current_arg_is("-h") ||
               parser.current_arg_is("--help")) {
      usage();
      std::exit(EXIT_SUCCESS);
    } else if (parser.current_arg_is("-version") ||
               parser.current_arg_is("--version") ||
               parser.current_arg_is("-v")) {
      std::cout << "This is rdfind version " << VERSION << '\n';
      std::exit(EXIT_SUCCESS);
    } else {
      std::cerr << "did not understand option " << parser.get_current_index()
                << ":\"" << parser.get_current_arg() << "\"\n";
      std::exit(EXIT_FAILURE);
    }
  }

  // fix default values
  if (o.maximumfilesize == 0) {
    o.maximumfilesize = std::numeric_limits<decltype(o.maximumfilesize)>::max();
  }

  // verify conflicting arguments
  if (!(o.minimumfilesize < o.maximumfilesize)) {
    std::cerr << "maximum filesize " << o.maximumfilesize
              << " must be larger than minimum filesize " << o.minimumfilesize
              << "\n";
    std::exit(EXIT_FAILURE);
  }

  // done with parsing of options. remaining arguments are files and dirs.

  // decide what checksum to use, default to sha1
  if (!o.usemd5 && !o.usesha1 && !o.usesha256 && !o.usesha512 && !o.usexxh128 &&
      !o.nochecksum) {
    o.usesha1 = true;
  }
  return o;
}
