/*
   copyright 2018 Paul Dreik
   Distributed under GPL v 2.0 or later, at your option.
   See LICENSE for further details.
*/

#include "config.h"

// std
#include <iostream>
#include <stdexcept>

// os
#include <unistd.h> //for unlink etc.

// project
#include "EasyRandom.hh"
#include "UndoableUnlink.hh"

UndoableUnlink::UndoableUnlink(const std::string& filename)
  : m_filename(filename)
{
  // make a random filename, to avoid getting ENAMETOOLONG
  // we try to replace the existing filename, instead of appending to it.
  // this will fail if the directory name is really long, but there is not
  // much to do about that without going into parsing mount points etc.
  const auto last_sep = m_filename.find_last_of('/');
  if (last_sep == std::string::npos) {
    // bare filename - replace it.
    m_tempfilename = EasyRandom().makeRandomFileString();
  } else {
    // found. keep the directory, switch out the filename.
    m_tempfilename =
      m_filename.substr(0, last_sep + 1) + EasyRandom().makeRandomFileString();
  }

  // move the file to a temporary name
  if (0 != rename(m_filename.c_str(), m_tempfilename.c_str())) {
    // failed rename.
    std::cerr << "Failed moving " + m_filename + " to a temporary file\n";
    m_state = state::FAILED_MOVE_TO_TEMPORARY;
  } else {
    m_state = state::MOVED_TO_TEMPORARY;
  }
}

int
UndoableUnlink::undo()
{
  if (m_state != state::MOVED_TO_TEMPORARY) {
    throw std::runtime_error(
      "api misuse - calling undo() now is a programming error");
  }

  if (0 != rename(m_tempfilename.c_str(), m_filename.c_str())) {
    // failed rename.
    m_state = state::FAILED_UNDO;
    std::cerr << "Failed moving file from temporary back to " + m_filename +
                   '\n';
    return 1;
  }

  m_state = state::UNDONE;
  return 0;
}

int
UndoableUnlink::unlink()
{
  if (m_state != state::MOVED_TO_TEMPORARY) {
    throw std::runtime_error(
      "api misuse - calling unlink() now is a programming error");
  }
  if (0 != ::unlink(m_tempfilename.c_str())) {
    std::cerr << "Failed unlinking temporary file made from " + m_filename +
                   '\n';
    m_state = state::FAILED_UNLINK;
    return 1;
  }
  m_state = state::UNLINKED;
  return 0;
}

UndoableUnlink::~UndoableUnlink()
{
  if (m_state == state::MOVED_TO_TEMPORARY) {
    undo();
  }
}
