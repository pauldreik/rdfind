/*
a class to hold information about a file.

Author Paul Sundvall 2006
see LICENSE for details.
$Revision: 765 $
$Id: Fileinfo.hh 765 2012-04-19 20:05:09Z pauls $
 */

#ifndef Fileinfo_hh
#define Fileinfo_hh


//c++ headers
#include <iostream> //for cout etc.
#include <cstring>

//os specific headers
#include <sys/types.h> //for off_t and others.

using std::cout;
using std::endl;

class Fileinfo {
public:
  // constructor
  Fileinfo(const std::string& name, int priority=0) 
    : m_magicnumber(771114), m_filename(name), m_delete(false), 
      m_duptype(DUPTYPE_UNKNOWN), m_priority(priority), m_identity(0),m_depth(0) 
  {
    memset(m_somebytes,0,sizeof(m_somebytes));
  }
  
  //destructor
  ~Fileinfo(){};

  //for fault checking - has no meaning for other purposes than
  //to detect mispointing references. you can ignore this one for the
  //functionality of the class.
  int m_magicnumber;

  //to keep the name of the file, including path
  std::string m_filename;
  
  //to be deleted or not
  bool m_delete;

  //type of duplicate
  enum duptype {
    DUPTYPE_UNKNOWN,
    DUPTYPE_FIRST_OCCURRENCE,
    DUPTYPE_WITHIN_SAME_TREE,
    DUPTYPE_OUTSIDE_TREE
  };

  duptype m_duptype;

  //enums used to tell how to read data into the buffer
  enum readtobuffermode {
    NOT_DEFINED=-1,
    READ_FIRST_BYTES=0,
    READ_LAST_BYTES=1,
    CREATE_MD5_CHECKSUM=2,
    CREATE_SHA1_CHECKSUM,
  };

  //to store info about the file
  typedef off_t filesizetype; //defined in sys/types.h
  struct Fileinfostat {
    filesizetype st_size;//size
    unsigned long st_ino;//inode
    unsigned long st_dev;//device
    bool is_file;
    bool is_directory;
    Fileinfostat();
  };
  struct Fileinfostat m_info;

  //some bytes of the file, good for comparision.
  static const int m_nbytes=64;
  char m_somebytes[m_nbytes];

  //This is a number that ranks this particular file on how important it is.
  //If two files are found to be identical, the one with most positive priority
  //will be kept. 
  int m_priority;

  //a number to identify this file.
  int m_identity;
  

  //the directory depth at which this file was found.
  int m_depth;

  int identity() const {return m_identity;}
  static int identity(const Fileinfo &A) {return A.identity();}
  void setidentity(int id) {m_identity=id;};
 
  //to set the priority of the file.
  void setpriority(int pri){m_priority=pri;}

  //reads the info about the file.
  //returns false if it was not possible to get the information.
  bool readfileinfo();

  //prints some file info. most for debugging.
  std::ostream& printinfo(std::ostream& out) const {    
    out<<"priority:"<<m_priority;
    out<<" inode:"<<inode();
    out<<" storlek:"<<size();
    out<<" fil:"<<m_filename; 
    return out;
  }

  int getduptype() const {return m_duptype;};

  static const std::string getduptypestring(const Fileinfo &A);

  //makes a symlink of "this" that points to A.
  int makesymlink(const Fileinfo &A);

  //makes a hardlink of "this" that points to A.
  int makehardlink(const Fileinfo &A);

  //deletes the file
  int deletefile();

  //deletes file A, that is a duplicate of B
  static int static_deletefile(Fileinfo &A, const Fileinfo &B);

  //makes a symlink of A that points to B
  static int static_makesymlink(Fileinfo &A, const Fileinfo &B);

  //makes a hard link of A that points to B
  static int static_makehardlink(Fileinfo &A, const Fileinfo &B);

  //sets the deleteflag
  void setdeleteflag(bool flag) {m_delete=flag;}
  
  //to get the deletflag
  bool deleteflag() const {return m_delete;};
  static bool static_deleteflag(const Fileinfo &A){return A.deleteflag();}

  //returns the size
  filesizetype size() const {return m_info.st_size;}

  //returns true if A has size zero
  static bool isempty(const Fileinfo &A){return A.size()==0;}

  //returns the inode number
  unsigned long inode() const {return m_info.st_ino;}

  //returns the device
  unsigned long device() const {return m_info.st_dev;}

  //gets the filename
  const std::string& name() const {return m_filename;}

  //gets the priority
  const int priority() const {return m_priority;}

  //gets the depth
  const int depth() const {return m_depth;}
  
  //sets the depth
  void setdepth(int depth) {m_depth=depth;}

  //returns true if a is smaller than b.
  static bool compareonsize(const Fileinfo &a, const Fileinfo &b) 
  { return (a.size()<b.size()); }
  
  //returns true if a has lower inode than b
  static bool compareoninode(const Fileinfo &a, const Fileinfo &b)
  {return (a.inode() < b.inode());}

  //returns true if a has lower device than b
  static bool compareondevice(const Fileinfo &a, const Fileinfo &b)
  {return (a.device() < b.device());}

  //returns true if a has lower priority number than b
  static bool compareonpriority(const Fileinfo &a, const Fileinfo &b)
  {return (a.priority() < b.priority());}

  //returns true if a has lower identity number than b)
  static bool compareonidentity(const Fileinfo &a, const Fileinfo &b)
  {return (a.identity() < b.identity());}

  //returns true if a has lower depth than b)
  static bool compareondepth(const Fileinfo &a, const Fileinfo &b)
  {return (a.depth() < b.depth());}

  //fills with bytes from the file. if lasttype is supplied,
  //it is used to see if the file needs to be read again - useful if
  //file is shorter than the length of the bytes field.
  int fillwithbytes(enum readtobuffermode filltype,
		    enum readtobuffermode lasttype=NOT_DEFINED);

  //display the bytes that are read from the file.
  void displaybytes() {cout<<"bytes are \""<<m_somebytes<<"\""<<endl;}

  //get a specific byte of the buffer
  char getreadbyte(int n) const {return m_somebytes[n];}

  //get a pointer to the bytes read from the file
  const char* getbyteptr() const {return m_somebytes;} 

  static bool compareonbytes(const Fileinfo &a, const Fileinfo &b);

  //compare on size first, then bytes. (useful for sorting)
  static bool compareonsizeandfirstbytes(const Fileinfo &a, const Fileinfo &b);

  //returns true if first bytes are equal. use "fillwithbytes" first!
  static bool equalbytes(const Fileinfo &a, const Fileinfo &b);
  
  //returns true if sizes are qual
  static bool equalsize(const Fileinfo &a, const Fileinfo &b);
  
  //returns true is inodes are equal
  static bool equalinode(const Fileinfo &a, const Fileinfo &b)
  {return (a.inode()==b.inode());}
  
  //returns true if devices are equal
  static bool equaldevice(const Fileinfo &a, const Fileinfo &b)
  {return (a.device()==b.device());}

  //returns true is priorities are equal
  static bool equalpriority(const Fileinfo &a, const Fileinfo &b)
  {return (a.priority()==b.priority());}
  
  //returns true if identities are equal
  static bool equalidentity(const Fileinfo &a, const Fileinfo &b)
  {return (a.identity()==b.identity());}

  //returns true if detphs are equal
  static bool equaldepth(const Fileinfo &a, const Fileinfo &b)
  {return (a.depth()==b.depth());}

  //returns true if file is a regular file. call readfileinfo first!
  bool isRegularFile() {return m_info.is_file;}

  //returns true if file is a directory . call readfileinfo first!
  bool isDirectory()  {return m_info.is_directory;} 
};








#endif
