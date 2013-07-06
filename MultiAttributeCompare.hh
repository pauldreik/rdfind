/*
some useful templates to sort a list on multiple attributes.

Author Paul Sundvall 2006
see LICENSE for details.
$Revision: 28 $
$Id: MultiAttributeCompare.hh 28 2006-02-25 14:24:52Z pauls $
 */
#ifndef MultiAttributeCompare_hh
#define MultiAttributeCompare_hh

#include <iostream> //for cerr etc

//an object that can compare objects T based on multiple attributes
//second template parameter is the maximum number of attributes, which can
//be set to a high value for convenience.
template<class T,int nfcnsmax>
class MultiAttributeCompare {
 
public:
  static const int m_nfcnsmax=nfcnsmax;//number of compare functions 
  //compare function type
  typedef  bool (*comparefcntype)(const T&, const T&);

  //constructor
  MultiAttributeCompare() {
    //initialize all functions to NULL
    for(int i=0;i<m_nfcnsmax;i++){
      m_lessthan[i]=NULL;//&alwaysfalse;
      m_equal[i]=NULL;//&alwaysfalse;      
    }
    m_nfcns=0;
    m_mode=OPMODE_NOTSELECTED;
  }

private:
  comparefcntype m_lessthan[m_nfcnsmax];
  comparefcntype m_equal[m_nfcnsmax];

  //mode of operation: < or ==
public:
  enum operationmode {
    OPMODE_NOTSELECTED=0,
    OPMODE_LESSTHAN,
    OPMODE_EQUAL
  };

private:
  operationmode m_mode;

private:
  //number of attributes to compare on
  int m_nfcns;
  
public:
  //to add an attribute to compare on
  //if null is added, nothing happens.
  void addattrib(comparefcntype lessthan, comparefcntype equal) {
    using std::cerr;
    using std::endl; 
    if(m_mode!=OPMODE_LESSTHAN && m_nfcns>0){
      cerr<<"you are not allowed to change operation type"<<endl;
      return;
    } else
      m_mode=OPMODE_LESSTHAN;
    
    if(m_nfcns>=m_nfcnsmax) {
      std::cerr<<"you have added too many attribs. increase max level first"<<std::endl;
    }
    if(lessthan && equal) {
      m_lessthan[m_nfcns]=lessthan;
      m_equal[m_nfcns]=equal;
      m_nfcns++;
    }
  }
  
  void addattrib(comparefcntype equal) {
    using std::cerr;
    using std::endl;
    if(m_mode!=OPMODE_EQUAL && m_nfcns>0){
      cerr<<"you are not allowed to change operation type"<<endl;
      return;
    } else
      m_mode=OPMODE_EQUAL;
    
    if(m_nfcns>=m_nfcnsmax) {
      std::cerr<<"you have added too many attribs. increase max level first"<<std::endl;
    }
    if(equal) {
      m_equal[m_nfcns]=equal;
      m_nfcns++;
    }
  }
  
  
public:
  static inline bool alwaysfalse(const T&, const T&) {return false;}
  static inline bool alwaystrue(const T&, const T&) {return true;}

private:
  //works like operator <
  bool compare_recurse(const T &A, const T &B, const int level) const {
    using std::cerr;
    using std::endl;
    
    if(level>=m_nfcns) {
      return false;//all attribs must have been equal to reach this.
      //thus return false, because < returns false if objs. are equal.
    }
    
    //first compare with <
    if((*m_lessthan[level])(A,B))
      return true;
    
    if((*m_equal[level])(A,B)) {
      //attrib is equal. we must compare on next level
      return compare_recurse(A,B,level+1);
    } else
      return false;
  }

  //works like operator ==
  bool equal_recurse(const T &A, const T &B, const int level) const {
    if(level>=m_nfcns) {
      return true;//all attribs must have been equal to reach this.
    }
        
    if((*m_equal[level])(A,B)) {
      //attribs are equal. we must compare on next level.
      return equal_recurse(A,B,level+1);
    } else
      return false;
  }


public:
  bool operator()(const T &A, const T &B) const {
    using std::cerr;
    using std::endl;
    
    
    const bool debug=false;


    //some fault checking. if there are derefernce problems
    //we will hopefully detect them here.
    if(A.m_magicnumber!=771114)
      cerr<<"öpp öpp öpp A"<<endl;  

    if(B.m_magicnumber!=771114)
      cerr<<"öpp öpp öpp B"<<endl;

    if(m_nfcns==0) {
      //the user must use "addattrib" first.
      std::cerr<<"add some attrib first"<<std::endl;
      return false;
    }

    bool retval=false;
    switch(m_mode){
    case OPMODE_LESSTHAN:
      //multi attribute <
      retval=compare_recurse(A,B,0);
      break;
    case OPMODE_EQUAL:
      //multi attribute ==
      retval=equal_recurse(A,B,0);
      break;
    case OPMODE_NOTSELECTED:
      cerr<<"you must add attributes first."<<endl;
      break;
    default:
      cerr<<"does not know that option"<<endl;
    }
    
    if(debug) {
      std::cerr<<"will return "<<(retval?"true":"false")<<" for"<<
	A.name()<<" and "<<B.name()<<std::endl;
    }
    return retval;
  }//operator()
};//class MultiAttributeCompare
#endif
