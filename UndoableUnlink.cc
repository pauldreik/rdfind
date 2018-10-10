/*
   copyright 2018 Paul Dreik
   Distributed under GPL v 2.0 or later, at your option.
   See LICENSE for further details.
*/

#include "UndoableUnlink.hh"
#include "EasyRandom.hh"
#include <unistd.h> //for unlink etc.

UndoableUnlink::UndoableUnlink(const std::string& filename)
  : m_filename(filename)
{
  // make a random filename, to avoid getting ENAMETOOLONG
  // we try to replace the existing filename, instead of appending to it.
  // this will fail if the directory name is really long, but there is not
  // much to do about that without going into parsing mount points etc.
  const auto last_sep = m_filename.find_last_of('/');
  if (last_sep == std::string::npos) {
    // not found. must be a bare filename.
    m_tempfilename = EasyRandom().makeRandomFileString();
  } else {
    // found. replace the filename.
    m_tempfilename =
      m_filename.substr(0, last_sep + 1) + EasyRandom().makeRandomFileString();
  }

  // move the file to a temporary name
  if (0 != rename(m_filename.c_str(), m_tempfilename.c_str())) {
    // failed rename. what should we do?
    m_tempfilename.resize(0);
  }
}

void
UndoableUnlink::undo()
{
  if (m_tempfilename.empty()) {
    return;
  }
  // move the file back again.
  if (0 != rename(m_tempfilename.c_str(), m_filename.c_str())) {
    // fail.
  }
  m_tempfilename.resize(0);
}

int
UndoableUnlink::unlink()
{
  int ret = 0;
  if (m_tempfilename.empty()) {
    return ret;
  }
  ret = ::unlink(m_tempfilename.c_str());
  m_tempfilename.resize(0);
  return ret;
}

UndoableUnlink::~UndoableUnlink()
{
  if (!m_tempfilename.empty()) {
    undo();
  }
}
