/*
   copyright 20016-2017 Paul Dreik (earlier Paul Sundvall)
   Distributed under GPL v 2.0 or later, at your option.
   See LICENSE for further details.
*/

#ifndef Dirlist_hh
#define Dirlist_hh

#include <string>
#include <vector>

/// class that traverses a directory
class Dirlist
{
public:
  // constructor
  explicit Dirlist(bool followsymlinks, std::string ignoredirs)
    : m_followsymlinks(followsymlinks)
    , m_callback(nullptr)
  {
    parseignoredirs(ignoredirs);
  }

private:
  // follow symlinks or not
  bool m_followsymlinks;

  // directory names to ignore
  std::vector<std::string> m_ignoredirs;

  // where to report found files. this is called for every item in all
  // directories found by walk.
  typedef int (*reportfcntype)(const std::string&, const std::string&, int);

  // called when a regular file or a symlink is encountered
  reportfcntype m_callback;

  // a function that is called from walk when a non-directory is encountered
  // for instance,if walk("/path/to/a/file.ext") is called instead of
  // walk("/path/to/a/")
  int handlepossiblefile(const std::string& possiblefile, int recursionlevel);

  // parse a space separated string of directory names to be ignored into a
  // vector of strings
  void parseignoredirs(std::string ignoredirs);

public:
  // find all files on a specific place
  int walk(const std::string& dir, const int recursionlevel = 0);

  // to set the report functions
  void setcallbackfcn(reportfcntype reportfcn) { m_callback = reportfcn; }
};

#endif
