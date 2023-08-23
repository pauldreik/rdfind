/**
   copyright 2006-2017 Paul Dreik (earlier Paul Sundvall)
   Distributed under GPL v 2.0 or later, at your option.
   See LICENSE for further details.
*/

#include "config.h" //header file from autoconf

// std
#include <algorithm>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <limits>

// project
#include "CmdlineParser.hh"
#include "Dirlist.hh"     //to find files
#include "Fileinfo.hh"    //file container
#include "RdfindDebug.hh" //debug macro
#include "Rdutil.hh"      //to do some work

// global variables

// this vector holds the information about all files found
std::vector<Fileinfo> filelist;
struct Options;
const Options* global_options{};

/**
 * this contains the command line index for the path currently
 * being investigated. it has to be global, because function pointers
 * are being used.
 */
int current_cmdline_index = 0;

static void
usage()
{
  std::cout
    << "Usage: "
    << "rdfind [options] FILE ...\n"
    << '\n'
    << "Finds duplicate files recursively in the given FILEs (directories),\n"
    << "and takes appropriate action (by default, nothing).\n"
    << "Directories listed first are ranked higher, meaning that if a\n"
    << "file is found on several places, the file found in the directory "
       "first\n"
    << "encountered on the command line is kept, and the others are "
       "considered duplicate.\n"
    << '\n'
    << "options are (default choice within parentheses)\n"
    << '\n'
    << " -ignoreempty      (true)| false  ignore empty files (true implies "
       "-minsize 1,\n"
    << "                                  false implies -minsize 0)\n"
    << " -minsize N        (N=1)          ignores files with size less than N "
       "bytes\n"
    << " -maxsize N        (N=0)          ignores files with size N "
       "bytes and larger (use 0 to disable this check).\n"
    << " -followsymlinks    true |(false) follow symlinks\n"
    << " -removeidentinode (true)| false  ignore files with nonunique "
       "device and inode\n"
    << " -checksum           md5 |(sha1)| sha256 | sha512\n"
    << "                                  checksum type\n"
    << " -deterministic    (true)| false  makes results independent of order\n"
    << "                                  from listing the filesystem\n"
    << " -makesymlinks      true |(false) replace duplicate files with "
       "symbolic links\n"
    << " -makehardlinks     true |(false) replace duplicate files with "
       "hard links\n"
    << " -makeresultsfile  (true)| false  makes a results file\n"
    << " -outputname  name  sets the results file name to \"name\" "
       "(default results.txt)\n"
    << " -deleteduplicates  true |(false) delete duplicate files\n"
    << " -sleep              Xms          sleep for X milliseconds between "
       "file reads.\n"
    << "                                  Default is 0. Only a few values\n"
    << "                                  are supported; 0,1-5,10,25,50,100\n"
    << " -dryrun|-n         true |(false) print to stdout instead of "
       "changing anything\n"
    << " -h|-help|--help                  show this help and exit\n"
    << " -v|--version                     display version number and exit\n"
    << '\n'
    << "If properly installed, a man page should be available as man rdfind.\n"
    << '\n'
    << "rdfind is written by Paul Dreik 2006 onwards. License: GPL v2 or "
       "later (at your option).\n"
    << "version is " << VERSION << '\n';
}

struct Options
{
  // operation mode and default values
  bool makesymlinks = false;   // turn duplicates into symbolic links
  bool makehardlinks = false;  // turn duplicates into hard links
  bool makeresultsfile = true; // write a results file
  Fileinfo::filesizetype minimumfilesize =
    1; // minimum file size to be noticed (0 - include empty files)
  Fileinfo::filesizetype maximumfilesize =
    0; // if nonzero, files this size or larger are ignored
  bool deleteduplicates = false;      // delete duplicate files
  bool followsymlinks = false;        // follow symlinks
  bool dryrun = false;                // only dryrun, don't destroy anything
  bool remove_identical_inode = true; // remove files with identical inodes
  bool usemd5 = false;       // use md5 checksum to check for similarity
  bool usesha1 = false;      // use sha1 checksum to check for similarity
  bool usesha256 = false;    // use sha256 checksum to check for similarity
  bool usesha512 = false;    // use sha512 checksum to check for similarity
  bool deterministic = true; // be independent of filesystem order
  long nsecsleep = 0; // number of nanoseconds to sleep between each file read.
  std::string resultsfile = "results.txt"; // results file name.
  std::string results_delimiter = " "; // fields delimiter in results file
};

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
    } else if (parser.try_parse_string("-outputdelimiter")){
      o.results_delimiter = parser.get_parsed_string();
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
      } else {
        std::cerr << "expected md5/sha1/sha256/sha512, not \""
                  << parser.get_parsed_string() << "\"\n";
        std::exit(EXIT_FAILURE);
      }
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

  // decide what checksum to use - if no checksum is set, force sha1!
  if (!o.usemd5 && !o.usesha1 && !o.usesha256 && !o.usesha512) {
    o.usesha1 = true;
  }
  return o;
}

// function to add items to the list of all files
static int
report(const std::string& path, const std::string& name, int depth)
{

  RDDEBUG("report(" << path.c_str() << "," << name.c_str() << "," << depth
                    << ")" << std::endl);

  // expand the name if the path is nonempty
  std::string expandedname = path.empty() ? name : (path + "/" + name);

  Fileinfo tmp(std::move(expandedname), current_cmdline_index, depth);
  if (tmp.readfileinfo()) {
    if (tmp.isRegularFile()) {
      const auto size = tmp.size();
      if (size >= global_options->minimumfilesize &&
          size < global_options->maximumfilesize) {
        filelist.emplace_back(std::move(tmp));
      }
    }
  } else {
    std::cerr << "failed to read file info on file \"" << tmp.name() << '\n';
    return -1;
  }
  return 0;
}

int
main(int narg, const char* argv[])
{
  if (narg == 1) {
    usage();
    return 0;
  }

  // parse the input arguments
  Parser parser(narg, argv);

  const Options o = parseOptions(parser);

  // set the dryrun string
  const std::string dryruntext(o.dryrun ? "(DRYRUN MODE) " : "");

  // an object to do sorting and duplicate finding
  Rdutil gswd(filelist);

  // an object to traverse the directory structure
  Dirlist dirlist(o.followsymlinks);

  // this is what function is called when an object is found on
  // the directory traversed by walk. Make sure the pointer to the
  // options is set as well.
  global_options = &o;
  dirlist.setcallbackfcn(&report);

  // now loop over path list and add the files

  // done with arguments. start parsing files and directories!
  for (; parser.has_args_left(); parser.advance()) {
    // get the next arg.
    const std::string file_or_dir = [&]() {
      std::string arg(parser.get_current_arg());
      // remove trailing /
      while (arg.back() == '/' && arg.size() > 1) {
        arg.erase(arg.size() - 1);
      }
      return arg;
    }();

    auto lastsize = filelist.size();
    std::cout << dryruntext << "Now scanning \"" << file_or_dir << "\"";
    std::cout.flush();
    current_cmdline_index = parser.get_current_index();
    dirlist.walk(file_or_dir, 0);
    std::cout << ", found " << filelist.size() - lastsize << " files."
              << std::endl;

    // if we want deterministic output, we will sort the newly added
    // items on depth, then filename.
    if (o.deterministic) {
      gswd.sort_on_depth_and_name(lastsize);
    }
  }

  std::cout << dryruntext << "Now have " << filelist.size()
            << " files in total." << std::endl;

  // mark files with a number for correct ranking. The only ordering at this
  // point is that files found on early command line index are earlier in the
  // list.
  gswd.markitems();

  if (o.remove_identical_inode) {
    // remove files with identical devices and inodes from the list
    std::cout << dryruntext << "Removed " << gswd.removeIdenticalInodes()
              << " files due to nonunique device and inode." << std::endl;
  }

  std::cout << dryruntext << "Total size is " << gswd.totalsizeinbytes()
            << " bytes or ";
  gswd.totalsize(std::cout) << std::endl;

  std::cout << "Removed " << gswd.removeUniqueSizes()
            << " files due to unique sizes from list. ";
  std::cout << filelist.size() << " files left." << std::endl;

  // ok. we now need to do something stronger to disambiguate the duplicate
  // candidates. start looking at the contents.
  std::vector<std::pair<Fileinfo::readtobuffermode, const char*>> modes{
    { Fileinfo::readtobuffermode::NOT_DEFINED, "" },
    { Fileinfo::readtobuffermode::READ_FIRST_BYTES, "first bytes" },
    { Fileinfo::readtobuffermode::READ_LAST_BYTES, "last bytes" },
  };
  if (o.usemd5) {
    modes.emplace_back(Fileinfo::readtobuffermode::CREATE_MD5_CHECKSUM,
                       "md5 checksum");
  }
  if (o.usesha1) {
    modes.emplace_back(Fileinfo::readtobuffermode::CREATE_SHA1_CHECKSUM,
                       "sha1 checksum");
  }
  if (o.usesha256) {
    modes.emplace_back(Fileinfo::readtobuffermode::CREATE_SHA256_CHECKSUM,
                       "sha256 checksum");
  }
  if (o.usesha512) {
    modes.emplace_back(Fileinfo::readtobuffermode::CREATE_SHA512_CHECKSUM,
                       "sha512 checksum");
  }

  for (auto it = modes.begin() + 1; it != modes.end(); ++it) {
    std::cout << dryruntext << "Now eliminating candidates based on "
              << it->second << ": " << std::flush;

    // read bytes (destroys the sorting, for disk reading efficiency)
    gswd.fillwithbytes(it[0].first, it[-1].first, o.nsecsleep);

    // remove non-duplicates
    std::cout << "removed " << gswd.removeUniqSizeAndBuffer()
              << " files from list. ";
    std::cout << filelist.size() << " files left." << std::endl;
  }

  // What is left now is a list of duplicates, ordered on size.
  // We also know the list is ordered on size, then bytes, and all unique
  // files are gone so it contains sequences of duplicates. Go ahead and mark
  // them.
  gswd.markduplicates();

  std::cout << dryruntext << "It seems like you have " << filelist.size()
            << " files that are not unique\n";

  std::cout << dryruntext << "Totally, ";
  gswd.saveablespace(std::cout) << " can be reduced." << std::endl;

  // traverse the list and make a nice file with the results
  if (o.makeresultsfile) {
    std::cout << dryruntext << "Now making results file " << o.resultsfile
              << std::endl;
    gswd.printtofile(o.resultsfile, o.results_delimiter);
  }

  // traverse the list and replace with symlinks
  if (o.makesymlinks) {
    std::cout << dryruntext << "Now making symbolic links. creating "
              << std::endl;
    const auto tmp = gswd.makesymlinks(o.dryrun);
    std::cout << "Making " << tmp << " links." << std::endl;
    return 0;
  }

  // traverse the list and replace with hard links
  if (o.makehardlinks) {
    std::cout << dryruntext << "Now making hard links." << std::endl;
    const auto tmp = gswd.makehardlinks(o.dryrun);
    std::cout << dryruntext << "Making " << tmp << " links." << std::endl;
    return 0;
  }

  // traverse the list and delete files
  if (o.deleteduplicates) {
    std::cout << dryruntext << "Now deleting duplicates:" << std::endl;
    const auto tmp = gswd.deleteduplicates(o.dryrun);
    std::cout << dryruntext << "Deleted " << tmp << " files." << std::endl;
    return 0;
  }
  return 0;
}
