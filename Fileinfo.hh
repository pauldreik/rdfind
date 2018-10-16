/*
   copyright 20016-2017 Paul Dreik (earlier Paul Sundvall)
   Distributed under GPL v 2.0 or later, at your option.
   See LICENSE for further details.
*/

#ifndef Fileinfo_hh
#define Fileinfo_hh

#include <array>
#include <cstring>
#include <iostream>

// os specific headers
#include <sys/types.h> //for off_t and others.

/**
 holds information about a file.
 */
class Fileinfo
{
public:
  // constructor
  Fileinfo(std::string name, int cmdline_index, int depth)
    : m_filename(std::move(name))
    , m_delete(false)
    , m_duptype(DUPTYPE_UNKNOWN)
    , m_cmdline_index(cmdline_index)
    , m_depth(depth)
    , m_identity(0)
  {
    m_somebytes.fill('\0');
  }

  // to store info about the file
  using filesizetype = off_t; // defined in sys/types.h
  struct Fileinfostat
  {
    filesizetype stat_size; // size
    unsigned long stat_ino; // inode
    unsigned long stat_dev; // device
    bool is_file;
    bool is_directory;
    Fileinfostat();
  };
  // enums used to tell how to read data into the buffer
  enum class readtobuffermode
  {
    NOT_DEFINED = -1,
    READ_FIRST_BYTES = 0,
    READ_LAST_BYTES = 1,
    CREATE_MD5_CHECKSUM = 2,
    CREATE_SHA1_CHECKSUM,
    CREATE_SHA256_CHECKSUM,
  };

  // type of duplicate
  enum duptype
  {
    DUPTYPE_UNKNOWN,
    DUPTYPE_FIRST_OCCURRENCE,
    DUPTYPE_WITHIN_SAME_TREE,
    DUPTYPE_OUTSIDE_TREE
  };
  void setduptype(enum duptype duptype_) { m_duptype = duptype_; }

private:
  // to keep the name of the file, including path
  std::string m_filename;

  // to be deleted or not
  bool m_delete;

  duptype m_duptype;

  struct Fileinfostat m_info;

  // If two files are found to be identical, the one with highest ranking is
  // chosen. The rules are listed in the man page.
  // lowest cmdlineindex wins, followed by the lowest depth, then first found.

  /**
   * in which order it appeared on the command line. can't be const, because
   * that means the implicitly defined assignment needed by the stl will be
   * illformed.
   * This is fine to be an int, because that is what argc,argv use.
   */
  int m_cmdline_index;

  /**
   * the directory depth at which this file was found.
   */
  int m_depth;

  /**
   * a number to identify this individual file. used for ranking.
   */
  std::int64_t m_identity;

  enum ByteSize
  {
    SomeByteSize = 64
  };
  /// a buffer that will be filled with some bytes of the file or a hash
  std::array<char, SomeByteSize> m_somebytes;

public:
  std::int64_t getidentity() const { return m_identity; }
  static std::int64_t identity(const Fileinfo& A) { return A.getidentity(); }
  void setidentity(std::int64_t id) { m_identity = id; };

  /**
   * reads info about the file, by querying the filesystem.
   * @return false if it was not possible to get the information.
   */
  bool readfileinfo();

  // prints some file info. most for debugging.
  std::ostream& printinfo(std::ostream& out) const
  {
    out << "cmdline index:" << m_cmdline_index;
    out << " inode:" << inode();
    out << " size:" << size();
    out << " file:" << m_filename;
    return out;
  }

  int getduptype() const { return m_duptype; };

  static std::string getduptypestring(const Fileinfo& A);

  // makes a symlink of "this" that points to A.
  int makesymlink(const Fileinfo& A);

  // makes a hardlink of "this" that points to A.
  int makehardlink(const Fileinfo& A);

  // deletes the file
  int deletefile();

  // deletes file A, that is a duplicate of B
  static int static_deletefile(Fileinfo& A, const Fileinfo& B);

  // makes a symlink of A that points to B
  static int static_makesymlink(Fileinfo& A, const Fileinfo& B);

  // makes a hard link of A that points to B
  static int static_makehardlink(Fileinfo& A, const Fileinfo& B);

  // sets the deleteflag
  void setdeleteflag(bool flag) { m_delete = flag; }

  // to get the deleteflag
  bool deleteflag() const { return m_delete; }

  static bool static_deleteflag(const Fileinfo& A) { return A.deleteflag(); }

  // returns the size
  filesizetype size() const { return m_info.stat_size; }

  // returns true if A has size zero
  static bool isempty(const Fileinfo& A) { return A.size() == 0; }
  /**
   * returns true if file A is smaller than minsize
   * @param A
   * @param minsize
   * @return
   */
  static bool is_smaller_than(const Fileinfo& A, Fileinfo::filesizetype minsize)
  {
    return A.size() < minsize;
  }

  // returns the inode number
  unsigned long inode() const { return m_info.stat_ino; }

  // returns the device
  unsigned long device() const { return m_info.stat_dev; }

  // gets the filename
  const std::string& name() const { return m_filename; }

  // gets the command line index this item was found at
  int get_cmdline_index() const { return m_cmdline_index; }

  // gets the depth
  int depth() const { return m_depth; }

  // returns true if a is smaller than b.
  static bool compareonsize(const Fileinfo& a, const Fileinfo& b)
  {
    return a.size() < b.size();
  }

  // returns true if a has lower inode than b
  static bool compareoninode(const Fileinfo& a, const Fileinfo& b)
  {
    return a.inode() < b.inode();
  }

  // returns true if a has lower device than b
  static bool compareondevice(const Fileinfo& a, const Fileinfo& b)
  {
    return a.device() < b.device();
  }

  // returns true if a has lower command line index than b
  static bool compareoncmdlineindex(const Fileinfo& a, const Fileinfo& b)
  {
    return a.get_cmdline_index() < b.get_cmdline_index();
  }

  // returns true if a has lower identity number than b)
  static bool compareonidentity(const Fileinfo& a, const Fileinfo& b)
  {
    return a.getidentity() < b.getidentity();
  }

  // returns true if a has lower depth than b)
  static bool compareondepth(const Fileinfo& a, const Fileinfo& b)
  {
    return a.depth() < b.depth();
  }

  // fills with bytes from the file. if lasttype is supplied,
  // it is used to see if the file needs to be read again - useful if
  // file is shorter than the length of the bytes field.
  int fillwithbytes(
    enum readtobuffermode filltype,
    enum readtobuffermode lasttype = readtobuffermode::NOT_DEFINED);

  // display the bytes that are read from the file.
  // hmm, this looks suspicious. what about the case where it does not contain a
  // null terminator?
  void displaybytes() const
  {
    std::cout << "bytes are \"" << m_somebytes.data() << "\"\n";
  }

  // get a pointer to the bytes read from the file
  const char* getbyteptr() const { return m_somebytes.data(); }

  static bool compareonbytes(const Fileinfo& a, const Fileinfo& b);

  // compare on size first, then bytes. (useful for sorting)
  static bool compareonsizeandfirstbytes(const Fileinfo& a, const Fileinfo& b);

  // returns true if first bytes are equal. use "fillwithbytes" first!
  static bool equalbytes(const Fileinfo& a, const Fileinfo& b);

  // returns true if sizes are qual
  static bool equalsize(const Fileinfo& a, const Fileinfo& b);

  // returns true is inodes are equal
  static bool equalinode(const Fileinfo& a, const Fileinfo& b)
  {
    return a.inode() == b.inode();
  }

  // returns true if devices are equal
  static bool equaldevice(const Fileinfo& a, const Fileinfo& b)
  {
    return a.device() == b.device();
  }

  // returns true is command line indices are equal
  static bool equalcmdlineindex(const Fileinfo& a, const Fileinfo& b)
  {
    return a.get_cmdline_index() == b.get_cmdline_index();
  }

  // returns true if identities are equal
  static bool equalidentity(const Fileinfo& a, const Fileinfo& b)
  {
    return a.getidentity() == b.getidentity();
  }

  // returns true if detphs are equal
  static bool equaldepth(const Fileinfo& a, const Fileinfo& b)
  {
    return a.depth() == b.depth();
  }

  // returns true if file is a regular file. call readfileinfo first!
  bool isRegularFile() const { return m_info.is_file; }

  // returns true if file is a directory . call readfileinfo first!
  bool isDirectory() const { return m_info.is_directory; }
};

#endif
