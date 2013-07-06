/*
this file contains functions and templates that implement most of the
functionality in rdfind.

Author Paul Sundvall 2006
see LICENSE for details.
$Revision: 803 $
$Id: Rdutil.cc 803 2013-01-26 04:22:16Z paul $
 */
#include "config.h"

#include "Rdutil.hh"
#include <cassert>
#include <fstream> //for file writing
#include <ostream> //for output
#include <string> //for easier passing of string arguments
#include "MultiAttributeCompare.hh"//for sorting on multiple attributes
#include "Fileinfo.hh"//file container
#include "algos.hh"//to find duplicates in a vector
#include <time.h> //to be able to call nanosleep properly.

//its ok to use these declarations, since this is a cc file.
using std::cout;
using std::endl;
using std::cerr;
using std::string;

int Rdutil::printtofile(const std::string &filename) {  
  //open a file to print to
  std::ofstream f1;
  f1.open(filename.c_str(),std::ios_base::out);
  if(!f1.is_open()) {
    cerr<<"could not open file \""<<filename<<"\""<<endl;
    return -1;
  }
  
  //exchange f1 for cout to write to terminal instead of file
  std::ostream &output(f1);
  
  output<<"# Automatically generated"<<endl;
  output<<"# duptype id depth size device inode priority name"<<endl;
  
  std::vector<Fileinfo>::iterator it;
  for(it=m_list.begin(); it!=m_list.end();++it){
    output<<Fileinfo::getduptypestring(*it)<<" "
	  <<it->identity()<<" "
	  <<it->depth()<<" "
	  <<it->size()<<" "
	  <<it->device()<<" "
	  <<it->inode()<<" "
	  <<it->priority()<<" "
	  <<it->name()
	  <<endl;
  }
  output<<"# end of file"<<endl;
  f1.close();
  return 0;
}

//applies int f(duplicate,const original) on every duplicate.
//if f returns nonzero, something is wrong.
//returns how many times the function was invoked.
template<typename Function>
int applyactiononfile(std::vector<Fileinfo> &m_list, Function f) {

    std::vector<Fileinfo>::iterator it,src;
    src=m_list.end();

    int ntimesapplied=0;

    //loop over files
    for(it=m_list.begin(); it!=m_list.end();++it){
      if(it->getduptype()==Fileinfo::DUPTYPE_FIRST_OCCURRENCE) {
	src=it;

	if(src->identity()<=0)
	  cerr<<"hmm. this file should have positive identity."<<endl;

      } else if(it->getduptype()==Fileinfo::DUPTYPE_OUTSIDE_TREE
		||it->getduptype()==Fileinfo::DUPTYPE_WITHIN_SAME_TREE ) {
	//double check that "it" shall be ~linked to "src"
	if(it->identity()== -src->identity()) {
	  //everything is in order. we may now ~link it to src.
	  if(f(*it,*src))
	    cerr<<"Rdutil.cc: Failed to apply function f on it."<<endl;
	  else
	    ntimesapplied++;
	} else
	  cerr<<"hmm. is list badly sorted?"<<endl;
      }
    }

    return ntimesapplied;
}

//helper for dryruns
template<class Outputobject>
class dryrun_helper {
public:
  dryrun_helper(Outputobject &out,string m1, string m2, string m3,int retval=0)
    :m_m1(m1),m_m2(m2),m_m3(m3),
     m_retval(retval),m_out(out),
     m_outputAname(true),m_outputBname(true) {};
  
  string m_m1,m_m2,m_m3;
  int m_retval;
  Outputobject &m_out;
  bool m_outputAname;
  bool m_outputBname;

  void disableAname(void) {m_outputAname=false;}
  void disableBname(void) {m_outputBname=false;}

  bool operator()(const Fileinfo &A, const Fileinfo &B) {
    string retstring= m_m1;
    if(m_outputAname)
           retstring+= A.name();

    retstring+=m_m2;

    if(m_outputBname)
      retstring+= B.name();

    retstring+=m_m3;

    m_out<<retstring<<endl;
      
    return m_retval;
  }
};

int Rdutil::deleteduplicates(bool dryrun) {
  if(dryrun){
    dryrun_helper<std::ostream> obj(cout,"delete ","","");
    obj.disableBname();
    return applyactiononfile(m_list,obj);
  }
  else
    return applyactiononfile(m_list,&Fileinfo::static_deletefile);
}

int Rdutil::makesymlinks(bool dryrun) {
  if(dryrun){
    dryrun_helper<std::ostream> obj(cout,"symlink "," to ","");
    return applyactiononfile(m_list,obj);
  }
  else
  return applyactiononfile(m_list,&Fileinfo::static_makesymlink);
}

int Rdutil::makehardlinks(bool dryrun) {
  if(dryrun){
    dryrun_helper<std::ostream> obj(cout,"hardlink "," to ","");
    return applyactiononfile(m_list,obj);
  }
  else
  return applyactiononfile(m_list,&Fileinfo::static_makehardlink);
}

//mark files with a unique number
int Rdutil::markitems() {
  std::vector<Fileinfo>::iterator it;
  int fileno=1;
  for(it=m_list.begin(); it!=m_list.end();++it){
    it->setidentity(fileno);
    fileno++;
  }
  return m_list.size();
}



//sort list
int Rdutil::sortlist(bool (*lessthan1)(const Fileinfo&, const Fileinfo&),
		     bool (*equal1)(const Fileinfo&, const Fileinfo&),
		     bool (*lessthan2)(const Fileinfo&, const Fileinfo&),
		     bool (*equal2)(const Fileinfo&, const Fileinfo&),
		     bool (*lessthan3)(const Fileinfo&, const Fileinfo&),
		     bool (*equal3)(const Fileinfo&, const Fileinfo&),
		     bool (*lessthan4)(const Fileinfo&, const Fileinfo&),
		     bool (*equal4)(const Fileinfo&, const Fileinfo&))
{
  MultiAttributeCompare<Fileinfo,10> comp;
  comp.addattrib(lessthan1,equal1);
  comp.addattrib(lessthan2,equal2);
  comp.addattrib(lessthan3,equal3);
  comp.addattrib(lessthan4,equal4);    
  
  sort(m_list.begin(),m_list.end(),comp);
  
  return 0;
}


//cleans up, by removing all items that have the deleteflag set to true.
int Rdutil::cleanup(){
  std::vector<Fileinfo>::iterator it;
  it=std::remove_if(m_list.begin(),m_list.end(),Fileinfo::static_deleteflag);
  
  int removed=m_list.end()-it;
  
  if(0)
    cout<<"will remove "<<removed<<" items"<<endl;
  
  m_list.erase(it,m_list.end());
  return removed;
}

//removes items
int Rdutil::remove_if()
{
  //  remove_if_helper hlp(rem);
  std::vector<Fileinfo>::iterator it;  
  it=std::remove_if(m_list.begin(),m_list.end(),&Fileinfo::isempty);
  int removed=m_list.end()-it; 
  m_list.erase(it,m_list.end());
  return removed;
}

//total size
//opmode=0 just add everything
//opmode=1 only elements with m_duptype=Fileinfo::DUPTYPE_FIRST_OCCURRENCE
unsigned long long Rdutil::totalsizeinbytes(int opmode) const
{
  //for some reason, for_each does not work. 
  Rdutil::adder_helper adder;
  std::vector<Fileinfo>::iterator it;
  if(opmode==0) {
    for(it=m_list.begin();it!=m_list.end();++it) {
      adder(*it);
    }
  } else if(opmode==1) {
    for(it=m_list.begin();it!=m_list.end();++it) {
      if(it->getduptype()==Fileinfo::DUPTYPE_FIRST_OCCURRENCE) {
	adder(*it);
      }
    }
  } else {
    assert(!"bad input, mode should be 0 or 1");
  }
  
  return adder.getsize();
}
namespace littlehelper {
  //helper to make "size" into a more readable form.
  int calcrange(unsigned long long int &size) {
    int range=0;     
    unsigned long long int tmp=0ULL;
    while(size > 1024ULL) {      
      tmp=size>>9;
      size=(tmp>>1);
      ++range;
    }

    //round up if necessary
    if(tmp & 0x1ULL)
      size++;

    return range;
  }
  
  //helper
  std::string byteprefix(int range) {
    switch (range) {
    case 0: return "b";
    case 1: return "kib";
    case 2: return "Mib";
    case 3: return "Gib";
    case 4: return "Tib";//Tebibyte
    case 5: return "Pib";//Pebibyte
    case 6: return "Eib";//exbibyte
    default: return "!way too much!";
    }
  }
}
std::ostream& Rdutil::totalsize(std::ostream &out,int opmode) {
  unsigned long long int size;
  size=totalsizeinbytes(opmode);
  //  out<<"original size is "<<size<<endl;
  int range=littlehelper::calcrange(size);
  //    out<<"then size is "<<size<<endl;

  //print size
  //  out<<" range is "<<range<<" and size is "<<size<<endl;
  out<<size<<" "<<littlehelper::byteprefix(range);
  return out;
}

std::ostream& Rdutil::saveablespace(std::ostream &out) {
  unsigned long long int size=totalsizeinbytes(0)-totalsizeinbytes(1); 
  int range=littlehelper::calcrange(size);
  out<<size<<" "<<littlehelper::byteprefix(range);
  return out;
}

//marks non unique elements for deletion. list must be sorted first.
int Rdutil::marknonuniq(bool (*equal1)(const Fileinfo&, const Fileinfo&),
			bool (*equal2)(const Fileinfo&, const Fileinfo&),
			bool (*equal3)(const Fileinfo&, const Fileinfo&),
			bool (*equal4)(const Fileinfo&, const Fileinfo&))
{ 

  //create on object that can compare two files
  MultiAttributeCompare<Fileinfo,10> comp;
  comp.addattrib(equal1);
  comp.addattrib(equal2);
  comp.addattrib(equal3);
  comp.addattrib(equal4); 

  //an object to apply on duplicate regions
  ApplyOnDuplicateFunction<Fileinfo> apf;

  //set all delete flags to false
  std::vector<Fileinfo>::iterator it,start,stop;

  start=m_list.begin();
  stop=m_list.end();
  

  for(it=m_list.begin();it!=m_list.end();++it){
    it->setdeleteflag(false);
  }

  apply_on_duplicate_regions<Fileinfo>(start,
				   stop,
				   comp,
				   apf);

  return 0;
}
    





  //marks uniq elements for deletion. list must be sorted first, before calling this.
int Rdutil::markuniq(bool (*equal1)(const Fileinfo&, const Fileinfo&),
		     bool (*equal2)(const Fileinfo&, const Fileinfo&),
		     bool (*equal3)(const Fileinfo&, const Fileinfo&),
		     bool (*equal4)(const Fileinfo&, const Fileinfo&))
{
  //create on object that can compare two files
  MultiAttributeCompare<Fileinfo,10> comp;
  comp.addattrib(equal1);
  comp.addattrib(equal2);
  comp.addattrib(equal3);
  comp.addattrib(equal4);    
  
  //identify the regions with duplicates
  int a=1;
  int ndup=0;
  std::vector<Fileinfo>::iterator start,stop,segstart,segstop;
  start=m_list.begin();
  stop=m_list.end();
  
  std::vector<Fileinfo>::iterator it;
  
  for(it=start;it!=stop;++it){
    it->setdeleteflag(true);
  }
  
  while(a>0) {
    //find the first region
    a=find_duplicate_regions<Fileinfo,MultiAttributeCompare<Fileinfo,10> >(start,stop,
								   segstart,segstop,
								   comp);
    
    if (a>0){       
      //found region. 
      //cout<<"found region with "<<a<<" duplicates."<<endl;
      
      //let the duplicate search start at a suitable place next time.
      start=segstop;
      
      //apply something on the objects that are duplicates
      for(it=segstart;it!=segstop;++it){
	it->setdeleteflag(false);
      }
    }
    ndup+=a;
  }//end while a>0
  
  return 0;
}







//marks duplicates
int Rdutil::markduplicates(bool (*equal1)(const Fileinfo&, const Fileinfo&),
			   bool (*equal2)(const Fileinfo&, const Fileinfo&),
			   bool (*equal3)(const Fileinfo&, const Fileinfo&),
			   bool (*equal4)(const Fileinfo&, const Fileinfo&))
{
  //create on object that can compare two files
  MultiAttributeCompare<Fileinfo,10> comp;
  comp.addattrib(equal1);
  comp.addattrib(equal2);
  comp.addattrib(equal3);
  comp.addattrib(equal4);    
  
  //identify the regions with duplicates
  int a=1;
  int ndup=0;
  std::vector<Fileinfo>::iterator start,stop,segstart,segstop;
  start=m_list.begin();
  stop=m_list.end();
  while(a>0) {
    //find the first region
    a=find_duplicate_regions<Fileinfo,MultiAttributeCompare<Fileinfo,10> >(start,stop,
								   segstart,segstop,
								   comp);
    
    if (a>0){       
      //found region
      //let the duplicate search start at a suitable place next time.
      start=segstop;
      
      //mark the file
      marksingle(segstart, segstop);
    }
    ndup+=a;
  }//end while a>0
  
  return 0;
}








  //formats output in a nice way.
int Rdutil::marksingle(std::vector<Fileinfo>::iterator start,
		       std::vector<Fileinfo>::iterator stop){
  //sort on priority - keep the other ordering (stable_sort instead of sort)
  stable_sort(start,stop,&Fileinfo::compareonpriority);

  std::vector<Fileinfo>::iterator it;
  int outputmode=1;
  switch (outputmode) {
  case 0://just informative, on the screen.
    //    output<<"# duplicate: size, inode, priority, name"<<endl;
    
    for(it=start; it!=stop;++it){

// 	    <<it->inode()<<" "
// 	    <<it->priority()<<" "
// 	    <<it->name()
// 	    <<endl;
    }    
//   output<<endl;
    break;
  case 1:
    //only considering a file belonging to a different priority as a
    // duplicates 
    
    for(it=start; it!=stop;++it){
      if(it==start) {
	//	output<<"#FILE:";
	it->m_duptype=Fileinfo::DUPTYPE_FIRST_OCCURRENCE;
      } else {
	//point out the file that it is a copy of
	it->setidentity(-Fileinfo::identity(*start));
	if(it->priority()==start->priority() && it!=start){
	  //output<<"#WITHIN SAME TREE:";
	  it->m_duptype=Fileinfo::DUPTYPE_WITHIN_SAME_TREE;
	} else {
	  //output<<"#DUPLICATE:";
	  it->m_duptype=Fileinfo::DUPTYPE_OUTSIDE_TREE;
	}
      }
    }    
    //    output<<endl;
    break;
  default:
    std::cerr<<"does not know that one"<<endl;
  }
  
  return 0;
}




//read some bytes. note! destroys the order of the list.
int Rdutil::fillwithbytes(enum Fileinfo::readtobuffermode type,
			  enum Fileinfo::readtobuffermode lasttype,
			  long nsecsleep) { 

  //first sort on inode (to read efficently from harddrive) 
  sortlist(&Fileinfo::compareoninode, &Fileinfo::equalinode);
  
  std::vector<Fileinfo>::iterator it;
  if(nsecsleep<=0)
    for(it=m_list.begin(); it!=m_list.end();++it){
      it->fillwithbytes(type,lasttype);    
    }
  else {
    //shall we do sleep between each file or not
    struct timespec time;
    time.tv_sec=0;
    time.tv_nsec=nsecsleep;
        
    for(it=m_list.begin(); it!=m_list.end();++it){
      it->fillwithbytes(type,lasttype);  
      nanosleep(&time,0);
    }
  }

  return 0;
}


