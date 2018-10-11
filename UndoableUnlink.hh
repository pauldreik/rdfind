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
 *
 * The correct api usage sequence is:
 * - construct
 * - if file_is_moved() returns true, you may optionally call one of undo() or
 * unlink() once.
 * - destroy
 *
 * In case of API misuse, the class throws.
 */
class UndoableUnlink
{
  enum class state
  {
    // these states are handled by the constructor
    UNINITIALIZED,
    FAILED_MOVE_TO_TEMPORARY,
    MOVED_TO_TEMPORARY,

    // undo() expects state MOVED_TO_TEMPORARY, and moves to these:
    FAILED_UNDO,
    UNDONE,

    // unlink() expects state MOVED_TO_TEMPORARY, and moves to these:
    FAILED_UNLINK,
    UNLINKED,
  };

public:
  /// checks if object is ok for calling undo() or unlink()
  bool file_is_moved() const { return m_state == state::MOVED_TO_TEMPORARY; }

  /**
   * Renames the file to a random name. If it fails, file_is_moved() will return
   * false.
   * @param filename
   */
  explicit UndoableUnlink(const std::string& filename);

  /**
   * moves the file back from the random name into the original filename
   * precondition: file_is_moved()
   * api misuse will lead to an exception thrown.
   * @return zero on success.
   */
  int undo();

  /**
   * removes the moved file.
   * precondition: file_is_moved()
   * api misuse will lead to an exception thrown.
   * @return zero on success
   */
  int unlink();

  /**
   * If possible, will restore the moved file to it's original place.
   * This is to make sure the object is restored in case of failure.
   */
  ~UndoableUnlink();

private:
  state m_state{ state::UNINITIALIZED };
  const std::string& m_filename;
  std::string m_tempfilename;
};

#endif /* RDFIND_UNDOABLEUNLINK_HH_ */
