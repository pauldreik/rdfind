/*
   copyright 2006-2017 Paul Dreik (earlier Paul Sundvall)
   Distributed under GPL v 2.0 or later, at your option.
   See LICENSE for further details.

   this file contains functions and templates that implement most of the
   functionality in rdfind.
 */
#ifndef rdutil_hh
#define rdutil_hh

#include <vector>

#include "Fileinfo.hh" //file container

class Rdutil
{
public:
  explicit Rdutil(std::vector<Fileinfo>& list)
    : m_list(list)
  {}

  /**
   * print file names to a file, with extra information.
   * @param filename
   * @return zero on success
   */
  int printtofile(const std::string& filename) const;

  /**
   * print file names to a file, with extra information.
   * @param filename
   * @param delimiter
   * @return zero on success
   */
  int printtofile(const std::string& filename, const std::string& delimiter) const;

  /// mark files with a unique number
  void markitems();

  /**
   * sorts the list on device and inode. not guaranteed to be stable.
   * @return
   */
  int sortOnDeviceAndInode();

  /**
   * sorts from the given index to the end on depth, then name.
   * this is useful to be independent of the filesystem order.
   */
  void sort_on_depth_and_name(std::size_t index_of_first);

  /**
   * for each group of identical inodes, only keep the one with the highest
   * rank.
   * @return number of elements removed
   */
  std::size_t removeIdenticalInodes();

  /**
   * remove files with unique size from the list.
   * @return
   */
  std::size_t removeUniqueSizes();

  /**
   * remove files with unique combination of size and buffer from the list.
   * @return
   */
  std::size_t removeUniqSizeAndBuffer();

  /**
   * Assumes the list is already sorted on size, and all elements with the same
   * size have the same buffer. Marks duplicates with tags, depending on their
   * nature. Shall be used when everything is done, and sorted.
   * For each sequence of duplicates, the original will be placed first but no
   * other guarantee on ordering is given.
   *
   */
  void markduplicates();

  /// removes all items from the list, that have the deleteflag set to true.
  std::size_t cleanup();

  /**
   * Removes items with file size less than minsize
   * @return the number of removed elements.
   */
  std::size_t remove_small_files(Fileinfo::filesizetype minsize);

  // read some bytes. note! destroys the order of the list.
  // if lasttype is supplied, it does not reread files if they are shorter
  // than the file length. (unnecessary!). if -1, feature is turned off.
  // and file is read anyway.
  // if there is trouble with too much disk reading, sleeping for nsecsleep
  // nanoseconds can be made between each file.
  int fillwithbytes(enum Fileinfo::readtobuffermode type,
                    enum Fileinfo::readtobuffermode lasttype =
                      Fileinfo::readtobuffermode::NOT_DEFINED,
                    long nsecsleep = 0);

  /// make symlinks of duplicates.
  std::size_t makesymlinks(bool dryrun) const;

  /// make hardlinks of duplicates.
  std::size_t makehardlinks(bool dryrun) const;

  /// delete duplicates from file system.
  std::size_t deleteduplicates(bool dryrun) const;

  /**
   * gets the total size, in bytes.
   * @param opmode 0 just add everything, 1 only elements with
   * m_duptype=Fileinfo::DUPTYPE_FIRST_OCCURRENCE
   * @return
   */
  [[gnu::pure]] Fileinfo::filesizetype totalsizeinbytes(int opmode = 0) const;

  /**
   * outputs a nicely formatted string "45 bytes" or "3 Gibytes"
   * where 1024 is used as base
   * @param out
   * @param opmode
   * @return
   */
  std::ostream& totalsize(std::ostream& out, int opmode = 0) const;

  /// outputs the saveable amount of space
  std::ostream& saveablespace(std::ostream& out) const;

private:
  std::vector<Fileinfo>& m_list;
};

#endif
