/**
   copyright 2006-2017 Paul Dreik (earlier Paul Sundvall)
   Distributed under GPL v 2.0 or later, at your option.
   See LICENSE for further details.
*/

#include "config.h" //header file from autoconf

static_assert(__cplusplus >= 201703L,
              "this code requires a C++17 capable compiler!");

// std
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// project
#include "CmdlineParser.hh"
#include "Dirlist.hh"     //to find files
#include "Fileinfo.hh"    //file container
#include "Options.hh"     //
#include "RdfindDebug.hh" //debug macro
#include "Rdutil.hh"      //to do some work

// global variables

// this vector holds the information about all files found
std::vector<Fileinfo> filelist;
const Options* global_options{};

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
      const auto size = tmp.size();
      if (size >= global_options->minimumfilesize &&
          size < global_options->maximumfilesize) {
        filelist.emplace_back(std::move(tmp));
      }
    }
  } else {
    std::cerr << "failed to read file info on file \"" << tmp.name() << "\"\n";
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

  std::cout << dryruntext << "Removed " << gswd.removeUniqueSizes()
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
  if (o.usexxh128) {
    modes.emplace_back(Fileinfo::readtobuffermode::CREATE_XXH128_CHECKSUM,
                       "xxh128 checksum");
  }

  std::function<void(std::size_t)> progress_callback;

  for (auto it = modes.begin() + 1; it != modes.end(); ++it) {
    std::cout << dryruntext << "Now eliminating candidates based on "
              << it->second << ": " << std::flush;

    if (o.showprogress) {
      progress_callback = []() {
        // format the total count only once, not each iteration.
        std::ostringstream oss;
        oss << "/" << filelist.size() << ")"
            << "\033[u"; // Restore the cursor to the saved position;
        return [suffix = oss.str()](std::size_t completed) {
          std::cout
            << "\033[s\033[K" // Save the cursor position & clear following text
            << "(" << completed << suffix << std::flush;
        };
      }();
    }

    // read bytes (destroys the sorting, for disk reading efficiency)
    gswd.fillwithbytes(
      it[0].first, it[-1].first, o.nsecsleep, o.buffersize, progress_callback);

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
    gswd.printtofile(o.resultsfile);
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
