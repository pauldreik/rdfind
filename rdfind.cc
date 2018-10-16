/**
   copyright 20016-2017 Paul Dreik (earlier Paul Sundvall)
   Distributed under GPL v 2.0 or later, at your option.
   See LICENSE for further details.
*/

#include "config.h"

#include <algorithm>
#include <iostream> //cout etc
#include <string>
#include <vector>

#include "CmdlineParser.hh"
#include "Dirlist.hh"     //to find files
#include "Fileinfo.hh"    //file container
#include "RdfindDebug.hh" //debug macro
#include "Rdutil.hh"      //to do some work
#include "config.h"       //header file from autoconf

// global variables

// these vectors hold the information about all files found
std::vector<Fileinfo> filelist1;

/**
 * this contains the command line index for the path currently
 * being investigated. it has to be global, because function pointers
 * are being used.
 */
int current_cmdline_index = 0;

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
      filelist1.push_back(tmp);
    }
  } else {
    std::cerr << "failed to read file info on file \"" << tmp.name() << '\n';
    return -1;
  }
  return 0;
}

static void
usage()
{
  std::cout
    << "Usage: "
    << "rdfind [options] FILE ...\n"
    << '\n'
    << "Finds duplicate files recursively in the given FILEs (directories),\n"
    << "and takes appropriate action (default nothing).\n"
    << "Directories listed first are ranked higher, meaning that if a\n"
    << "file is found on several places, the file found in the directory "
       "first\n"
    << "encountered on the command line is kept, and the others are "
       "considered duplicate.\n"
    << '\n'
    << "options are (default choice within parentheses)\n"
    << '\n'
    << " -ignoreempty      (true)| false  ignore empty files (true implies "
       "-minsize 1, false implies -minsize 0)\n"
    << " -minsize N        (N=1)          ignores files with size less than N "
       "bytes\n"
    << " -followsymlinks    true |(false) follow symlinks\n"
    << " -removeidentinode (true)| false  ignore files with nonunique "
       "device and inode\n"
    << " -checksum           md5 |(sha1)| sha256\n"
    << "                                  checksum type\n"
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

int
main(int narg, const char* argv[])
{
  if (narg == 1) {
    usage();
    return 0;
  }

  // operation mode and default values
  bool makesymlinks = false;   // turn duplicates into symbolic links
  bool makehardlinks = false;  // turn duplicates into hard links
  bool makeresultsfile = true; // write a results file
  Fileinfo::filesizetype minimumfilesize =
    1; // minimum file size to be noticed (0 - include empty files)
  bool deleteduplicates = false;      // delete duplicate files
  bool followsymlinks = false;        // follow symlinks
  bool dryrun = false;                // only dryrun, dont destroy anything
  bool remove_identical_inode = true; // remove files with identical inodes
  bool usemd5 = false;    // use md5 checksum to check for similarity
  bool usesha1 = false;   // use sha1 checksum to check for similarity
  bool usesha256 = false; // use sha256 checksum to check for similarity
  long nsecsleep = 0; // number of nanoseconds to sleep between each file read.

  std::string resultsfile = "results.txt"; // results file name.

  // parse the input arguments
  Parser parser(narg, argv);

  for (; parser.has_args_left(); parser.advance()) {
    // empty strings are forbidden as input since they can not be file names or
    // options
    if (parser.get_current_arg()[0] == '\0') {
      std::cerr << "bad argument " << parser.get_current_index() << '\n';
      return -1;
    }

    // if we reach the end of the argument list - exit the loop and proceed with
    // the file list instead.
    if (parser.get_current_arg()[0] != '-') {
      // end of argument list - exit!
      break;
    }
    if (parser.try_parse_bool("-makesymlinks")) {
      makesymlinks = parser.get_parsed_bool();
    } else if (parser.try_parse_bool("-makehardlinks")) {
      makehardlinks = parser.get_parsed_bool();
    } else if (parser.try_parse_bool("-makeresultsfile")) {
      makeresultsfile = parser.get_parsed_bool();
    } else if (parser.try_parse_string("-outputname")) {
      resultsfile = parser.get_parsed_string();
    } else if (parser.try_parse_bool("-ignoreempty")) {
      if (parser.get_parsed_bool()) {
        minimumfilesize = 1;
      } else {
        minimumfilesize = 0;
      }
    } else if (parser.try_parse_string("-minsize")) {
      const long long minsize = std::stoll(parser.get_parsed_string());
      if (minsize < 0) {
        throw std::runtime_error("negative value of minsize not allowed");
      }
      minimumfilesize = minsize;
    } else if (parser.try_parse_bool("-deleteduplicates")) {
      deleteduplicates = parser.get_parsed_bool();
    } else if (parser.try_parse_bool("-followsymlinks")) {
      followsymlinks = parser.get_parsed_bool();
    } else if (parser.try_parse_bool("-dryrun")) {
      dryrun = parser.get_parsed_bool();
    } else if (parser.try_parse_bool("-n")) {
      dryrun = parser.get_parsed_bool();
    } else if (parser.try_parse_bool("-removeidentinode")) {
      remove_identical_inode = parser.get_parsed_bool();
    } else if (parser.try_parse_string("-checksum")) {
      if (parser.parsed_string_is("md5")) {
        usemd5 = true;
      } else if (parser.parsed_string_is("sha1")) {
        usesha1 = true;
      } else if (parser.parsed_string_is("sha256")) {
        usesha256 = true;
      } else {
        std::cerr << "expected md5/sha1/sha256, not \""
                  << parser.get_parsed_string() << "\"\n";
        return -1;
      }
    } else if (parser.try_parse_string("-sleep")) {
      const auto nextarg = std::string(parser.get_parsed_string());
      if (nextarg == "1ms")
        nsecsleep = 1000000;
      else if (nextarg == "2ms")
        nsecsleep = 2000000;
      else if (nextarg == "3ms")
        nsecsleep = 3000000;
      else if (nextarg == "4ms")
        nsecsleep = 4000000;
      else if (nextarg == "5ms")
        nsecsleep = 5000000;
      else if (nextarg == "10ms")
        nsecsleep = 10000000;
      else if (nextarg == "25ms")
        nsecsleep = 25000000;
      else if (nextarg == "50ms")
        nsecsleep = 50000000;
      else if (nextarg == "100ms")
        nsecsleep = 100000000;
      else {
        std::cerr << "sorry, can only understand a few sleep values for "
                     "now. \""
                  << nextarg << "\" is not among them.\n";
        return -1;
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
      return -1;
    }
  }
  // done with parsing of options. remaining arguments are files and dirs.

  // decide what checksum to use - if no checksum is set, force sha1!
  if (!usemd5 && !usesha1 && !usesha256) {
    usesha1 = true;
  }

  // set the dryrun string
  const std::string dryruntext(dryrun ? "(DRYRUN MODE) " : "");

  // an object to do sorting and duplicate finding
  Rdutil gswd(filelist1);

  // an object to traverse the directory structure
  Dirlist dirlist(followsymlinks);

  // this is what function is called when an object is found on
  // the directory traversed by walk
  dirlist.setreportfcn_regular_file(&report);

  // follow symlinks or not
  if (followsymlinks) {
    dirlist.setreportfcn_symlink(&report);
  }

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

    auto lastsize = filelist1.size();
    std::cout << dryruntext << "Now scanning \"" << file_or_dir << "\"";
    std::cout.flush();
    current_cmdline_index = parser.get_current_index();
    dirlist.walk(file_or_dir, 0);
    std::cout << ", found " << filelist1.size() - lastsize << " files."
              << std::endl;
  }

  std::cout << dryruntext << "Now have " << filelist1.size()
            << " files in total." << std::endl;

  // mark files with a number for correct ranking
  gswd.markitems();

  if (remove_identical_inode) {
    // remove files with identical devices and inodes
    gswd.sortlist(&Fileinfo::compareoninode,
                  &Fileinfo::equalinode,
                  &Fileinfo::compareondevice,
                  &Fileinfo::equaldevice,
                  &Fileinfo::compareoncmdlineindex,
                  &Fileinfo::equalcmdlineindex,
                  &Fileinfo::compareondepth,
                  &Fileinfo::equaldepth);

    // mark duplicates - these must be due to hard links or
    // link that has been followed.
    gswd.marknonuniq(&Fileinfo::equalinode, &Fileinfo::equaldevice);

    // remove non-duplicates
    std::cout << dryruntext << "Removed " << gswd.cleanup()
              << " files due to nonunique device and inode." << std::endl;
  }

  if (minimumfilesize > 0) {
    std::cout << dryruntext << "Now removing files with size<"
              << minimumfilesize << " from the list...";
    std::cout.flush();
    std::cout << "removed " << gswd.remove_small_files(minimumfilesize)
              << " files" << std::endl;
  }

  std::cout << dryruntext << "Total size is " << gswd.totalsizeinbytes()
            << " bytes or ";
  gswd.totalsize(std::cout) << std::endl;

  std::cout << dryruntext << "Now sorting on size:";

  std::sort(filelist1.begin(), filelist1.end(), Fileinfo::compareonsize);

  // mark non-duplicates
  gswd.markuniq(&Fileinfo::equalsize);

  // remove non-duplicates
  std::cout << "removed " << gswd.cleanup()
            << " files due to unique sizes from list.";
  std::cout << filelist1.size() << " files left." << std::endl;

  // ok. we now need to do something stronger. read a few bytes.
  const int nreadtobuffermodes = 5;
  Fileinfo::readtobuffermode lasttype = Fileinfo::readtobuffermode::NOT_DEFINED;
  Fileinfo::readtobuffermode type[nreadtobuffermodes];
  type[0] = Fileinfo::readtobuffermode::READ_FIRST_BYTES;
  type[1] = Fileinfo::readtobuffermode::READ_LAST_BYTES;
  type[2] = (usemd5 ? Fileinfo::readtobuffermode::CREATE_MD5_CHECKSUM
                    : Fileinfo::readtobuffermode::NOT_DEFINED);
  type[3] = (usesha1 ? Fileinfo::readtobuffermode::CREATE_SHA1_CHECKSUM
                     : Fileinfo::readtobuffermode::NOT_DEFINED);
  type[4] = (usesha256 ? Fileinfo::readtobuffermode::CREATE_SHA256_CHECKSUM
                       : Fileinfo::readtobuffermode::NOT_DEFINED);

  for (int i = 0; i < nreadtobuffermodes; i++) {
    if (type[i] != Fileinfo::readtobuffermode::NOT_DEFINED) {
      std::string description;

      switch (type[i]) {
        case Fileinfo::readtobuffermode::READ_FIRST_BYTES:
          description = "first bytes";
          break;
        case Fileinfo::readtobuffermode::READ_LAST_BYTES:
          description = "last bytes";
          break;
        case Fileinfo::readtobuffermode::CREATE_MD5_CHECKSUM:
          description = "md5 checksum";
          break;
        case Fileinfo::readtobuffermode::CREATE_SHA1_CHECKSUM:
          description = "sha1 checksum";
          break;
        case Fileinfo::readtobuffermode::CREATE_SHA256_CHECKSUM:
          description = "sha256 checksum";
          break;
        default:
          description = "--program error!!!---";
          break;
      }
      std::cout << dryruntext << "Now eliminating candidates based on "
                << description << ":";

      std::cout.flush();

      // read bytes (destroys the sorting, for efficiency)
      gswd.fillwithbytes(type[i], lasttype, nsecsleep);

      // sort on size, bytes
      gswd.sortlist(&Fileinfo::compareonsize,
                    &Fileinfo::equalsize,
                    &Fileinfo::compareonbytes,
                    &Fileinfo::equalbytes);

      // mark non-duplicates
      gswd.markuniq(&Fileinfo::equalsize, &Fileinfo::equalbytes);

      // remove non-duplicates
      std::cout << "removed " << gswd.cleanup() << " files from list.";
      std::cout << filelist1.size() << " files left." << std::endl;

      lasttype = type[i];
    }
  }

  // we now know we have only true duplicates left.

  // figure out how to present the material

  // sort, so that duplicates are collected together.
  gswd.sortlist(&Fileinfo::compareonsize,
                &Fileinfo::equalsize,
                &Fileinfo::compareonbytes,
                &Fileinfo::equalbytes,
                &Fileinfo::compareondepth,
                &Fileinfo::equaldepth,
                &Fileinfo::compareonidentity,
                &Fileinfo::equalidentity);

  // mark duplicates with the right tag (will stable sort the list
  // internally on command line index)
  gswd.markduplicates(&Fileinfo::equalsize, &Fileinfo::equalbytes);

  std::cout << dryruntext << "It seems like you have " << filelist1.size()
            << " files that are not unique\n"
            << std::endl;

  std::cout << dryruntext << "Totally, ";
  gswd.saveablespace(std::cout) << " can be reduced." << std::endl;

  // traverse the list and make a nice file with the results
  if (makeresultsfile) {
    std::cout << dryruntext << "Now making results file " << resultsfile
              << std::endl;
    gswd.printtofile(resultsfile);
  }

  // traverse the list and replace with symlinks
  if (makesymlinks) {
    std::cout << dryruntext << "Now making symbolic links. creating "
              << std::endl;
    int tmp = gswd.makesymlinks(dryrun);
    std::cout << "Making " << tmp << " links." << std::endl;
    return 0;
  }

  // traverse the list and replace with hard links
  if (makehardlinks) {
    std::cout << dryruntext << "Now making hard links." << std::endl;
    int tmp = gswd.makehardlinks(dryrun);
    std::cout << "Making " << tmp << " links." << std::endl;
    return 0;
  }

  // traverse the list and delete files
  if (deleteduplicates) {
    std::cout << dryruntext << "Now deleting duplicates:" << std::endl;
    int tmp = gswd.deleteduplicates(dryrun);
    std::cout << "Deleted " << tmp << " files." << std::endl;
    return 0;
  }
  return 0;
}
