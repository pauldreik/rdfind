/*
files for reading entries from a file structure.

Author Paul Sundvall 2006
see LICENSE for details.
$Revision: 530 $
$Id: Dirlist.hh 530 2008-10-03 09:41:29Z pauls $
 */
#ifndef Dirlist_hh
#define Dirlist_hh

#include <string>
#include <cstring>

//class that traverses a directory
class Dirlist {
public:
  //constructor
  Dirlist(bool followsymlinks,int depth=50) : 
    m_maxdepth(depth),
    m_followsymlinks(followsymlinks)
  {
    m_report_regular_file= &_do_nothing_;
    m_report_failed_on_stat= &_do_nothing_;
    m_report_directory= &_do_nothing_;
    m_report_symlink= &_do_nothing_;
  }

  ~Dirlist() {}

private:
  //the maximum search depth
  int m_maxdepth;

  //follow symlinks or not
  bool m_followsymlinks;

  //where to report found files. this is called for every item in all
  //directories found by walk.
  typedef int (*reportfcntype)(const std::string& , const std::string &, int);
  
  //called when a regular file is encountered
  reportfcntype m_report_regular_file;

  //called when stat fails
  reportfcntype m_report_failed_on_stat;
  
  //called when a directory is encountered
  reportfcntype m_report_directory;
  
  //called when a symbolic link is found (file or directory)
  reportfcntype m_report_symlink;
  
  //a function that does nothing
  static int _do_nothing_(const std::string& , const std::string &, int) {return 0;};

  //a function that is called from walk when a non-directory is encountered
  //for instance,if walk("/path/to/a/file.ext") is called instead of
  //walk("/path/to/a/")
  int handlepossiblefile(const std::string &possiblefile,int recursionlevel);

public:
  //find all files on a specific place
  int walk(const std::string &dir,const int recursionlevel=0);

  //to set the report functions
  void setreportfcn_regular_file(reportfcntype reportfcn)
  {m_report_regular_file=reportfcn;}

  void setreportfcn_failed_on_stat(reportfcntype reportfcn)
  {m_report_failed_on_stat=reportfcn;}

  void setreportfcn_report_directory(reportfcntype reportfcn)
  {m_report_directory=reportfcn;} 

  void setreportfcn_symlink(reportfcntype reportfcn)
  {m_report_symlink=reportfcn;}
};

#endif
