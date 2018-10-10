/*
   copyright 2018 Paul Dreik
   Distributed under GPL v 2.0 or later, at your option.
   See LICENSE for further details.
*/
#ifndef RDFIND_UNDOABLEUNLINK_HH_
#define RDFIND_UNDOABLEUNLINK_HH_

#include <string>

/**
 * This is a class that makes an undoable delete. It will move the given
 * file to a temporary filename on construction. If you wish to proceed
 * to actually delete the file (through the temporary file name), call
 * unlink(). The destructor will undo the file, unless you already have called
 * undo() or unlink().
 */
class UndoableUnlink
{
public:
  /**
   * Renames the file to a random name.
   * @param filename
   */
  UndoableUnlink(const std::string& filename);

  /**
   * moves the file back from the random name into the original filename
   */
  void undo();

  /**
   * removes the file with the random name - undo() will not be possible after.
   * @return
   */
  int unlink();

  /**
   * unless undo() or unlink() has been called, will invoke undo(). This means
   * if the object goes out of scope and noone called undo() or unlink(), the
   * file is restored.
   */
  ~UndoableUnlink();

private:
  const std::string& m_filename;
  std::string m_tempfilename;
};

#endif /* RDFIND_UNDOABLEUNLINK_HH_ */
