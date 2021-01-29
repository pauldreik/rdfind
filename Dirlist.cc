/*
   copyright 20016-2017 Paul Dreik (earlier Paul Sundvall)
   Distributed under GPL v 2.0 or later, at your option.
   See LICENSE for further details.
*/

#include "config.h"

// std
#include <cerrno>
#include <cstring>
#include <iostream>
#include <iterator>
#include <string>
#include <sstream>
#include <vector>

// os
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// project
#include "Dirlist.hh"
#include "RdfindDebug.hh" //debug macros

static const int maxdepth = 50;

int
Dirlist::walk(const std::string& dir, const int recursionlevel)
{

  RDDEBUG("Now in walk with dir=" << dir.c_str() << " and recursionlevel="
                                  << recursionlevel << std::endl);

  if (recursionlevel >= maxdepth) {
    std::cerr << "recursion limit exceeded\n";
    return -1;
  }

  // open the directory
  DIR* dirp = opendir(dir.c_str());
  if (dirp == NULL) {
    // failed to open directory
    RDDEBUG("failed to open directory" << std::endl);
    // this can be due to rights, or some other error.
    handlepossiblefile(dir, recursionlevel);
    return 1; // its a file (or something else)
  }

  // we opened the directory. let us read the content.
  RDDEBUG("opened directory" << std::endl);
  struct dirent* dp = NULL;
  while (NULL != (dp = readdir(dirp))) {
    // is the directory . or ..?
    if (0 == strcmp(".", dp->d_name) || 0 == strcmp("..", dp->d_name)) {
      continue;
    }
    // Does the directory match ignoredirs?
    auto ignored = [&]() {
      for (auto name: m_ignoredirs) {
        if (0 == strcmp(name.c_str(), dp->d_name)) {
          return true;
        }
      }
      return false;
    };
    if (ignored()) {
      continue;
    }
    // investigate what kind of file it is, dont follow any
    // symlinks when doing this (lstat instead of stat).
    struct stat info;
    const int statval =
      lstat((dir + "/" + std::string(dp->d_name)).c_str(), &info);
    if (statval != 0) {
      // failed to do stat
      continue;
    }

    // investigate what kind of item it was.
    bool dowalk = false;

    if (S_ISLNK(info.st_mode)) {
      // symlink
      if (m_followsymlinks) {
        (*m_callback)(dir, std::string(dp->d_name), recursionlevel);
      }
      if (m_followsymlinks) {
        dowalk = true;
      }
    } else if (S_ISDIR(info.st_mode)) {
      // directory
      dowalk = true;
    } else if (S_ISREG(info.st_mode)) {
      // regular file
      (*m_callback)(dir, std::string(dp->d_name), recursionlevel);
    }

    // try to open directory
    if (dowalk) {
      walk(dir + "/" + dp->d_name, recursionlevel + 1);
    }

  } // while

  // close the directory
  (void)closedir(dirp);
  return 2; // its a directory
}

// splits inputstring into path and filename. if no / character is found,
// empty string is returned as path and filename is set to inputstring.
int
splitfilename(std::string& path,
              std::string& filename,
              const std::string& inputstring)
{

  const auto pos = inputstring.rfind('/');
  if (pos == std::string::npos) {
    path = "";
    filename = inputstring;
    return -1;
  }

  path = inputstring.substr(0, pos + 1);
  filename = inputstring.substr(pos + 1, std::string::npos);
  return 0;
}

// this function is called for files that were believed to be directories,
// or failed re
int
Dirlist::handlepossiblefile(const std::string& possiblefile, int recursionlevel)
{
  struct stat info;

  RDDEBUG("Now in handlepossiblefile with name "
          << possiblefile.c_str() << " and recursionlevel " << recursionlevel
          << std::endl);

  // split filename into path and filename
  std::string path, filename;
  splitfilename(path, filename, possiblefile);

  RDDEBUG("split filename is path=" << path.c_str() << " filename="
                                    << filename.c_str() << std::endl);

  // investigate what kind of file it is, dont follow symlink
  int statval = 0;
  do {
    statval = lstat(possiblefile.c_str(), &info);
  } while (statval < 0 && errno == EINTR);

  if (statval < 0) {
    // probably file does not exist, or trouble with rights.
    RDDEBUG("got negative statval " << statval << std::endl);
    //(*m_report_failed_on_stat)(path, filename, recursionlevel);
    return -1;
  } else {
    RDDEBUG("got positive statval " << statval << std::endl);
  }

  if (S_ISLNK(info.st_mode)) {
    RDDEBUG("found symlink" << std::endl);
    if (m_followsymlinks) {
      (*m_callback)(path, filename, recursionlevel);
    }
    return 0;
  } else {
    RDDEBUG("not a symlink" << std::endl);
  }

  if (S_ISDIR(info.st_mode)) {
    std::cerr << "Dirlist.cc::handlepossiblefile: This should never happen. "
                 "FIXME! details on the next row:\n";
    std::cerr << "possiblefile=\"" << possiblefile << "\"\n";
    // this should never happen, because this function is only to be called
    // for items that can not be opened with opendir.
    // maybe it happens if someone else is changing the file while we
    // are reading it?
    return -2;
  } else {
    RDDEBUG("not a dir\n");
  }

  if (S_ISREG(info.st_mode)) {
    RDDEBUG("it is a regular file" << std::endl);
    (*m_callback)(path, filename, recursionlevel);
    return 0;
  } else {
    RDDEBUG("not a regular file" << std::endl);
  }
  std::cout
    << "Dirlist.cc::handlepossiblefile(): found something else than a dir or "
       "a regular file."
    << std::endl;
  return -1;
}

// parse a space separated string of directory names to be ignored into a
// vector of strings
void
Dirlist::parseignoredirs(std::string ignoredirs) {
  std::istringstream ss{ignoredirs};
  using iter = std::istream_iterator<std::string>;
  std::vector<std::string> dirs{iter{ss}, iter{}};
  m_ignoredirs = std::move(dirs);
}
