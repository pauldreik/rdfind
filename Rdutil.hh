/*
this file contains functions and templates that implement most of the
functionality in rdfind.

Author Paul Sundvall 2006
see LICENSE for details.
$Revision: 719 $
$Id: Rdutil.hh 719 2011-07-24 12:17:18Z pauls $
 */
#ifndef rdutil_hh
#define rdutil_hh

#include <fstream> //for file writing
#include <algorithm>

#include "MultiAttributeCompare.hh"//for sorting on multiple attributes

#include "Fileinfo.hh"//file container

#include "algos.hh"//to find duplicates in a vector

class Rdutil {
public:
  Rdutil(std::vector<Fileinfo> &list) : 
    m_list(list) {};
  ~Rdutil(){};
  std::vector<Fileinfo> &m_list;
  //  std::vector<std::vector<Fileinfo> > &m_duplist;
  
public:
  //print file names to a file, with extra information.
  int printtofile(const std::string &filename);
  
  //mark files with a unique number
  int markitems();

  //sort list on multiple attributes.
  int sortlist(  bool (*lessthan1)(const Fileinfo&, const Fileinfo&),
		 bool (*equal1)(const Fileinfo&, const Fileinfo&),
		 bool (*lessthan2)(const Fileinfo&, const Fileinfo&)=NULL,
		 bool (*equal2)(const Fileinfo&, const Fileinfo&)=NULL,
		 bool (*lessthan3)(const Fileinfo&, const Fileinfo&)=NULL,
		 bool (*equal3)(const Fileinfo&, const Fileinfo&)=NULL,
		 bool (*lessthan4)(const Fileinfo&, const Fileinfo&)=NULL,
		 bool (*equal4)(const Fileinfo&, const Fileinfo&)=NULL);
 
  //cleans up, by removing all items that have the deleteflag set to true.
  int cleanup();

  //marks non unique elements for deletion. list must be sorted first.
  //this is good to eliminate duplicates on inode, to prevent from 
  //reading hardlinked files, or repeated input arguments to the main program.
  int marknonuniq(bool (*equal1)(const Fileinfo&, const Fileinfo&),
		  bool (*equal2)(const Fileinfo&, const Fileinfo&)=NULL,
		  bool (*equal3)(const Fileinfo&, const Fileinfo&)=NULL,
		  bool (*equal4)(const Fileinfo&, const Fileinfo&)=NULL);

  //marks uniq elements for deletion (remember, this is a duplicate finder!)
  // list must be sorted first, before calling this.
  int markuniq(bool (*equal1)(const Fileinfo&, const Fileinfo&),
	       bool (*equal2)(const Fileinfo&, const Fileinfo&)=NULL,
	       bool (*equal3)(const Fileinfo&, const Fileinfo&)=NULL,
	       bool (*equal4)(const Fileinfo&, const Fileinfo&)=NULL);

  //marks duplicates with tags, depending on their nature. 
  //shall be used when everything is done, and sorted.
  int markduplicates(bool (*equal1)(const Fileinfo&, const Fileinfo&),
		     bool (*equal2)(const Fileinfo&, const Fileinfo&)=NULL,
		     bool (*equal3)(const Fileinfo&, const Fileinfo&)=NULL,
		     bool (*equal4)(const Fileinfo&, const Fileinfo&)=NULL);
  
  //subfunction to above
  int marksingle(std::vector<Fileinfo>::iterator start,
		 std::vector<Fileinfo>::iterator stop);
  

  //removes items that rem returns true for.
  //returns the number of removed elements.
  int remove_if();

  //read some bytes. note! destroys the order of the list.
  //if lasttype is supplied, it does not reread files if they are shorter
  //than the file length. (unnecessary!). if -1, feature is turned off.
  //and file is read anyway. 
  //if there is trouble with too much disk reading, sleeping for nsecsleep
  //nanoseconds can be made between each file.
  int fillwithbytes(enum Fileinfo::readtobuffermode type,
		    enum Fileinfo::readtobuffermode lasttype=Fileinfo::NOT_DEFINED,
		    long nsecsleep=0);
  
  //make symlinks of duplicates.
  int makesymlinks(bool dryrun);

  //make hardlinks of duplicates.
  int makehardlinks(bool dryrun);

  //delete duplicates.
  int deleteduplicates(bool dryrun);

  //a little helper class 
  class adder_helper {
  public:
    adder_helper() : m_sum(0) {};
    unsigned long long int m_sum;
    void operator()(const Fileinfo &A) {m_sum+=A.size();}
    unsigned long long int getsize(void) const {return m_sum;}
  };

  //gets the total size, in bytes.
  //opmode=0 just add everything
  //opmode=1 only elements with m_duptype=Fileinfo::DUPTYPE_FIRST_OCCURRENCE
  unsigned long long int totalsizeinbytes(int opmode=0);

  //outputs a nicely formatted string "45 bytes" or "3 Gibytes"
  //where 1024 is used as base
  std::ostream& totalsize(std::ostream &out, int opmode=0);

  //outputs the saveable amount of space
  std::ostream& saveablespace(std::ostream &out);
};

#endif
