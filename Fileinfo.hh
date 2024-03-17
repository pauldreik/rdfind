/*
   copyright 2006-2018 Paul Dreik (earlier Paul Sundvall)
   Distributed under GPL v 2.0 or later, at your option.
   See LICENSE for further details.
*/

#ifndef Fileinfo_hh
#define Fileinfo_hh

#include <array>
#include <cstdint>
#include <string>
#include <vector>

// os specific headers
#include <sys/types.h> //for off_t and others.

/**
 Holds information about a file.
 Keeping this small is probably beneficial for performance, because the
 large vector of all found files will be better cached.
 */
class Fileinfo
{
public:
  // constructor
  Fileinfo(std::string name, int cmdline_index, int depth)
    : m_info()
    , m_filename(std::move(name))
    , m_delete(false)
    , m_duptype(duptype::DUPTYPE_UNKNOWN)
    , m_cmdline_index(cmdline_index)
    , m_depth(depth)
    , m_identity(0)
  {
    m_somebytes.fill('\0');
  }

  /// for storing file size in bytes, defined in sys/types.h
  using filesizetype = off_t;

  // enums used to tell how to read data into the buffer
  enum class readtobuffermode : signed char
  {
    NOT_DEFINED = -1,
    READ_FIRST_BYTES = 0,
    READ_LAST_BYTES = 1,
    CREATE_MD5_CHECKSUM = 2,
    CREATE_SHA1_CHECKSUM,
    CREATE_SHA256_CHECKSUM,
    CREATE_SHA512_CHECKSUM,
  };

  // type of duplicate
  enum class duptype : char
  {
    DUPTYPE_UNKNOWN,
    DUPTYPE_FIRST_OCCURRENCE,
    DUPTYPE_WITHIN_SAME_TREE,
    DUPTYPE_OUTSIDE_TREE
  };

  /**
   * gets a string with duptype
   * @param A
   * @return
   */
  [[gnu::pure]] static const char* getduptypestring(const Fileinfo& A);
  void setduptype(enum duptype duptype_) { m_duptype = duptype_; }

  std::int64_t getidentity() const { return m_identity; }
  static std::int64_t identity(const Fileinfo& A) { return A.getidentity(); }
  void setidentity(std::int64_t id) { m_identity = id; }

  /**
   * reads info about the file, by querying the filesystem.
   * @return false if it was not possible to get the information.
   */
  bool readfileinfo();

  duptype getduptype() const { return m_duptype; }

  /// makes a symlink of "this" that points to A.
  int makesymlink(const Fileinfo& A);

  /// makes a hardlink of "this" that points to A.
  int makehardlink(const Fileinfo& A);

  /**
   * deletes the file from the file system
   * @return zero on success
   */
  int deletefile();

  // makes a symlink of A that points to B
  static int static_makesymlink(Fileinfo& A, const Fileinfo& B);

  // makes a hard link of A that points to B
  static int static_makehardlink(Fileinfo& A, const Fileinfo& B);

  // deletes file A, that is a duplicate of B
  static int static_deletefile(Fileinfo& A, const Fileinfo& B);

  // sets the deleteflag
  void setdeleteflag(bool flag) { m_delete = flag; }

  /// to get the deleteflag
  bool deleteflag() const { return m_delete; }

  /// returns the file size in bytes
  filesizetype size() const { return m_info.stat_size; }

  // returns true if A has size zero
  bool isempty() const { return size() == 0; }

  /// filesize comparison
  bool is_smaller_than(Fileinfo::filesizetype minsize) const
  {
    return size() < minsize;
  }

  // returns the inode number
  unsigned long inode() const { return m_info.stat_ino; }

  // returns the device
  unsigned long device() const { return m_info.stat_dev; }

  // gets the filename
  const std::string& name() const { return m_filename; }

  // gets the filename
  const std::vector<std::string>& aliases() const { return m_aliases; }

  // adds a filename alias
  void add_alias(std::string n) { m_aliases.push_back(n); }

  // gets the command line index this item was found at
  int get_cmdline_index() const { return m_cmdline_index; }

  // gets the depth
  int depth() const { return m_depth; }

  /**
   * fills with bytes from the file. if lasttype is supplied,
   * it is used to see if the file needs to be read again - useful if the file
   * is shorter than the length of the bytes field.
   * @param filltype
   * @param lasttype
   * @return zero on success
   */
  int fillwithbytes(enum readtobuffermode filltype,
                    enum readtobuffermode lasttype);

  /// get a pointer to the bytes read from the file
  const char* getbyteptr() const { return m_somebytes.data(); }

  std::size_t getbuffersize() const { return m_somebytes.size(); }

  /// returns true if file is a regular file. call readfileinfo first!
  bool isRegularFile() const { return m_info.is_file; }

  // returns true if file is a directory . call readfileinfo first!
  bool isDirectory() const { return m_info.is_directory; }

private:
  // to store info about the file
  struct Fileinfostat
  {
    filesizetype stat_size; // size
    unsigned long stat_ino; // inode
    unsigned long stat_dev; // device
    bool is_file;
    bool is_directory;
    Fileinfostat();
  };
  Fileinfostat m_info;

  // to keep the name of the file, including path
  std::string m_filename;

  // to be deleted or not
  bool m_delete;

  // list of hardlinks
  std::vector<std::string> m_aliases;

  duptype m_duptype;

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

  static const int SomeByteSize = 64;

  /// a buffer that will be filled with some bytes of the file or a hash
  std::array<char, SomeByteSize> m_somebytes;
};

#endif
