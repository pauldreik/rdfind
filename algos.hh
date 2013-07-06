/*
some algorithms that may come in handy.
Author Paul Sundvall 2006
see LICENSE for details.
$Revision: 26 $
$Id: algos.hh 26 2006-02-25 14:14:20Z pauls $
 */

#ifndef algos_hh
#define algos_hh

#include <vector>

template<class T> 
int find_duplicate_regions(typename std::vector<T> &lista,
			   typename std::vector<T>::iterator &start,
			   typename std::vector<T>::iterator &stop,
			   typename std::vector<T>::iterator &regionstart,
			   typename std::vector<T>::iterator &regionstop,
			   bool (*func1)(const T&, const T&),
			   bool (*func2)(const T&, const T&))
{
  //looks for regions of duplicates and returns iterators to those  places.
  //returns the number of members.

  typename std::vector<T>::iterator it1,it2,it3;
  it1=start;
  it2=it1;
  //   int b=((*it1).*func)(); 
  bool foundregion=false;
  int length=0;
  while(foundregion==false && it1!=stop) {
    it2++;
    //if(((*it1).*func)()==((*it2).*func)()) {
    if(it2!=stop && (*func1)(*it1,*it2) && (*func2)(*it1,*it2) ){
      foundregion=true;
      length=2;
    } else {
      it1=it2;
    }    
  }
  
  if(foundregion){//hooray! we found two equal. can we find more?
    it3=it2;
    it3++;
    while(it3!=stop && 
	  ( (*func1)(*it1,*it3) && (*func2)(*it1,*it3) ) 
	  ){
      it3++;
      length++;
    }
    it2=it3;
    //now it1 is the first, and it2 is the last one(+1) in this region.
    regionstart=it1;
    regionstop=it2;
    //    cout<<"found region with length "<<length<<endl;
  } else {
    //    cout<<"did not find region"<<endl;
  }
  return length;
}


//finds duplicate regions, reported equal by function/functor f.
//list must be sorted.
//the first region of duplicates is returned in regionstart, regionstop.
template<class T, typename Function> 
int find_duplicate_regions(typename std::vector<T>::iterator &start,
			   typename std::vector<T>::iterator &stop,
			   typename std::vector<T>::iterator &regionstart,
			   typename std::vector<T>::iterator &regionstop,
			   Function f)
{  //looks for regions of duplicates and returns iterators to those places.
  //returns the number of members.

  typename std::vector<T>::iterator it1,it2;
  it1=start;
  it2=it1;
  bool foundregion=false;
  int length=0;

  while(foundregion==false && it1!=stop) {
    it2++;
    if(it2!=stop && f(*it1,*it2) ){
      //duplicate found!
      foundregion=true;
      length=2;
    } else {
      //proceed searching
      it1=it2;
    }    
  }
  
  if(foundregion){//hooray! we found two equal. can we find more?
    it2++;
    while(it2!=stop && f(*it1,*it2) ){
      it2++;
      length++;
    }
    //now it1 is the first, and it2 is the last one(+1) in this region.
    regionstart=it1;
    regionstop=it2;
    //    cout<<"found region with length "<<length<<endl;
  } else {
    //    cout<<"did not find region"<<endl;
  }
  return length;
}



//template for class being called at when a duplicate region is found.
template<class T>
class ApplyOnDuplicateFunction {
public:
  void operator()(typename std::vector<T>::iterator start,
		  typename std::vector<T>::iterator stop)
  {    
    typename std::vector<T>::iterator it=start;
    it->setdeleteflag(false);
    ++it;
    for( ; it!=stop ; ++it ){
      it->setdeleteflag(true);
    }
  }
};

//finds duplicate regions, reported equal by function/functor f.
//list must be sorted.
//When an equal region is found, apf(it_start, it_end) is called.
//this is done until all list is gone through.
template<class T, typename EqualFunction, typename ApplyFunction> 
int apply_on_duplicate_regions(typename std::vector<T>::iterator &start,
			       typename std::vector<T>::iterator &stop,
			       EqualFunction eqf, 
			       ApplyFunction apf)
{  //looks for regions of duplicates and returns iterators to those places.
  //returns the number of members.

  typename std::vector<T>::iterator it1,it2,tmp1,tmp2;
  it1=start;
  it2=it1;
  int ntimesinvoked=0; 

  while(it2!=stop){
    bool foundregion=false;
    
    
    while(foundregion==false && it1!=stop) {
      it2++;
      if(it2!=stop && eqf(*it1,*it2) ){
	//duplicate found!
	foundregion=true;
      } else {
	//proceed searching
	it1=it2;
      }    
    }
    
    if(foundregion){
      //hooray! we found two equal. can we find more?
      it2++;
      while(it2!=stop && eqf(*it1,*it2) ){
	it2++;
      }
      //now it1 is the first, and it2 is the last one(+1) in this region.

      //apply function apf
      tmp1=it1;
      tmp2=it2;
      apf(tmp1,tmp2);
      ntimesinvoked++;

      //continue searching
      it1=it2;
    } else {
      //did not find region.
    }
  }

  return ntimesinvoked;
}

#endif
