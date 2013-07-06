/*
  rdfind.cc
  copyright Paul Dreik (earlier Sundvall)

  see LICENSE for details.
  $Revision: 803 $
  $Id: rdfind.cc 803 2013-01-26 04:22:16Z paul $


  version info:
  (moved to Changelog)

*/

#include "config.h"

#include <iostream>//cout etc
#include <vector>
#include <string>
#include <algorithm>

#include "Fileinfo.hh"//file container
#include "Dirlist.hh" //to find files
#include "Rdutil.hh" //to do some work
#include "RdfindDebug.hh" //debug macro

#include "config.h"//header file from autoconf

//function to print things from a vector
template<class T> 
struct print : public std::unary_function<T, void>
{
  print(std::ostream& out) : os(out) {}
  inline void operator() (T x)  { x.printinfo(os)<<std::endl; }
  std::ostream& os;
};


//global variables

//these vectors hold the information about all files found
std::vector<Fileinfo> filelist1;

//this is the priority of the files that are currently added.
int currentpriority=0;



//function to add items to the list of all files
int report(const std::string &path, const std::string &name, int depth){
  
  RDDEBUG("report("<<path.c_str()<<","<<name.c_str()<<","<<depth<<")"
	  <<std::endl);

  //expand the name if the path is nonempty
  std::string expandedname=
    path.empty() ? 
    name :
    (path+"/"+name);

  Fileinfo tmp(expandedname);
  
  tmp.setpriority(currentpriority);
  tmp.setdepth(depth);
  if(tmp.readfileinfo()) {
    if(tmp.isRegularFile()) {
      filelist1.push_back(tmp);
    }
  } else {
    std::cerr<<"failed to read file info on file \""<<tmp.name()<<std::endl;
    return -1;
  }
  return 0;
}




void usage()
{
  using std::cout;
  using std::endl;
  //..<<"..............................80 chars output......................................................."
  cout<<endl<<"usage:"<<endl;
  cout<<"rdfind [options] directory1 directory2 ..."<<endl;
  cout<<endl<<" Finds duplicate files in directories, and takes appropriate actions"<<endl;
  cout<<"Directories listed first are ranked higher, meaning that if a"<<endl;
  cout<<"file is found on several places, the file found in the directory first"<<endl;
  cout<<" encountered on the command line is kept, and the others are considered duplicate."<<endl;
  cout<<" options are (default choice within parantheses) "<<endl<<endl;
  cout<<" -makesymlinks      true |(false) replace duplicate files with symbolic links"<<endl;
  cout<<" -makehardlinks     true |(false) replace duplicate files with hard links"<<endl;
  cout<<" -deleteduplicates  true |(false) delete duplicate files"<<endl;
  cout<<" -ignoreempty      (true)| false  ignore empty files"<<endl;
  cout<<" -removeidentinode (true)| false  ignore files with nonunique device and inode"<<endl;
  cout<<" -makeresultsfile  (true)| false  makes a results file"<<endl;
  cout<<" -outputname  name  sets the results file name to \"name\" (default results.txt)"<<endl;
  cout<<" -followsymlinks    true |(false) follow symlinks"<<endl;
  cout<<" -dryrun|-n         true |(false) print to stdout instead of changing anything"<<endl;
  cout<<" -checksum          (md5)| sha1   checksum type"<<endl;
  cout<<" -sleep              Xms          sleep for X milliseconds between file reads."<<endl;
  cout<<"                                  Default is 0. Currently, only a few values"<<endl;
  cout<<"                                  are supported; 0,1-5,10,25,50,100"<<endl;
  cout<<endl;               
  cout<<"If properly installed, a man page should be available as man rdfind."<<endl;
  cout<<endl;
  cout<<endl
      <<"rdfind is written by Paul Dreik 2006. License: GPL v2 or later (at your option)."<<endl;
  cout<<"svn version of this file is $Revision: 803 $"<<endl;
  cout<<"svn id of this file is $Id: rdfind.cc 803 2013-01-26 04:22:16Z paul $"<<endl;
  cout<<"version is "<<VERSION<<endl;
  cout<<endl;
}






int main(int narg, char *argv[])
{
  using std::cout;
  using std::cerr;
  using std::endl;
  using std::string;
  using std::vector;

  if(narg==1){
    usage();
    return 0;
  }
  
  //operation mode and default values
  bool makesymlinks=false; //turn duplicates into symbolic links
  bool makehardlinks=false; //turn duplicates into hard links
  bool makeresultsfile=true;//write a results file
  bool ignoreempty=true;//ignore empty files
  bool deleteduplicates=false;//delete duplicate files
  bool followsymlinks=false; //follow symlinks
  bool dryrun=false;//only dryrun, dont destroy anything
  bool remove_identical_inode=true;//remove files with identical inodes
  bool usemd5=false;//use md5 checksum to check for similarity
  bool usesha1=false;//use sha1 checksum to check for similarity
  long nsecsleep=0; //number of nanoseconds to sleep between each file read.

  string resultsfile="results.txt";//results file name.

   //a list of where to search
  std::vector<string> pathlist;

  //parse the input args
  for (int n=1;n<narg;n++){
    //decide if this input arg is an option or a directory.
    string arg(argv[n]);
    if(arg.length()<1) {
      cerr<<"bad argument "<<n<<endl;    
      return -1;
    }
    
    //   cout<<"read argument \""<<arg<<"\""<<endl;
    if(arg.at(0)=='-') {
      if (arg=="-makesymlinks" && n<(narg-1)) {
	string nextarg(argv[1+n]);n++;     
	if (nextarg=="true")
	  makesymlinks=true;
	else if (nextarg=="false")
	  makesymlinks=false;
	else {
	  cerr<<"expected true or false, not \""<<nextarg<<"\""<<endl;
	  return -1;
	}
      }          
      else if (arg=="-makehardlinks" && n<(narg-1)) {
	string nextarg(argv[1+n]);n++;     
	if (nextarg=="true")
	  makehardlinks=true;
	else if (nextarg=="false")
	  makehardlinks=false;
	else {
	  cerr<<"expected true or false, not \""<<nextarg<<"\""<<endl;
	  return -1;
	}
      }     
      else if (arg=="-makeresultsfile" && n<(narg-1)) {
	string nextarg(argv[1+n]);n++;     
	if (nextarg=="true")
	  makeresultsfile=true;
	else if (nextarg=="false")
	  makeresultsfile=false;
	else {
	  cerr<<"expected true or false, not \""<<nextarg<<"\""<<endl;
	  return -1;
	}
      }
      else if (arg=="-outputname" && n<(narg-1)) {
	string nextarg(argv[1+n]);n++;     
	resultsfile=nextarg;
      }
      else if (arg=="-ignoreempty" && n<(narg-1)) {
	string nextarg(argv[1+n]);n++;     
	if (nextarg=="true")
	  ignoreempty=true;
	else if (nextarg=="false")
	  ignoreempty=false;
	else {
	  cerr<<"expected true or false, not \""<<nextarg<<"\""<<endl;
	  return -1;
	}
      }
      else if (arg=="-deleteduplicates" && n<(narg-1)) {
	string nextarg(argv[n+1]);n++;
	if (nextarg=="true")
	  deleteduplicates=true;
	else if (nextarg=="false")
	  deleteduplicates=false;
	else {
	  cerr<<"expected true or false, not \""<<nextarg<<"\""<<endl;
	  return -1;
	}
      } 
      else if (arg=="-followsymlinks" && n<(narg-1)) {
	string nextarg(argv[n+1]);n++;
	if (nextarg=="true")
	  followsymlinks=true;
	else if (nextarg=="false")
	  followsymlinks=false;
	else {
	  cerr<<"expected true or false, not \""<<nextarg<<"\""<<endl;
	  return -1;
	}
      }
      else if ((arg=="-dryrun" || arg=="-n") && n<(narg-1)) {
	string nextarg(argv[n+1]);n++;
	if (nextarg=="true")
	  dryrun=true;
	else if (nextarg=="false")
	  dryrun=false;
	else {
	  cerr<<"expected true or false, not \""<<nextarg<<"\""<<endl;
	  return -1;
	}
      }
      else if (arg=="-removeidentinode" && n<(narg-1)) {
	string nextarg(argv[n+1]);n++;
	if (nextarg=="true")
	  remove_identical_inode=true;
	else if (nextarg=="false")
	  remove_identical_inode=false;
	else {
	  cerr<<"expected true or false, not \""<<nextarg<<"\""<<endl;
	  return -1;
	}
      }
      else if (arg=="-checksum" && n<(narg-1)) {
	string nextarg(argv[n+1]);n++;
	if (nextarg=="md5")
	  usemd5=true;
	else if (nextarg=="sha1")
	  usesha1=true;
	else {
	  cerr<<"expected md5 or sha1, not \""<<nextarg<<"\""<<endl;
	  return -1;
	}
      }
      else if (arg=="-sleep"  && n<(narg-1)) {
	string nextarg(argv[n+1]);n++;
	if (nextarg=="1ms")
	  nsecsleep=1000000;
	else if (nextarg=="2ms")
	  nsecsleep=2000000;
	else if (nextarg=="3ms")
	  nsecsleep=3000000;
	else if (nextarg=="4ms")
	  nsecsleep=4000000;
	else if (nextarg=="5ms")
	  nsecsleep=5000000;
	else if (nextarg=="10ms")
	  nsecsleep=10000000;
	else if (nextarg=="25ms")
	  nsecsleep=25000000;
	else if (nextarg=="50ms")
	  nsecsleep=50000000;
	else if (nextarg=="100ms")
	  nsecsleep=100000000;
	else {
	  cerr<<"sorry, can only understand a few sleep values for now."<<endl;
	  return -1;
	}
      }
      else if (arg=="-help" || arg=="-h" || arg=="--help") {
	usage();
	return 0;
      }
      else if (arg=="-version" || arg=="--version" || arg=="-v") {
	cout<<"This is rdfind version "<<VERSION<<endl;
	return 0;
      }
      else {
	cerr<<"did not understand option "<<n<<":\""<<arg<<"\""<<endl;
	return -1;
      }
    }
    else
      {//must be input directory
	//remove trailing /
	while(arg.at(arg.size()-1)=='/' && arg.size()>1) {
	  arg.erase(arg.size()-1);
	}
	pathlist.push_back(arg);	
      }
  }

  //decide what checksum to use - if no checksum is set, force md5!
  if ( !usemd5 && !usesha1) {
    usemd5=true;
  }

  //command line parsing went OK to reach this point.

  //set the dryrun string
  const std::string dryruntext(dryrun?"(DRYRUN MODE) ":"");

  //an object to do sorting and duplicate finding
  Rdutil gswd(filelist1);
  
  //an object to traverse the directory structure
  Dirlist dirlist(followsymlinks);
  
  //this is what function is called when an object is found on 
  //the directory traversed by walk
  dirlist.setreportfcn_regular_file(&report);

  //follow symlinks or not
  if(followsymlinks)
    dirlist.setreportfcn_symlink(&report);


  //now loop over path list and add the files
  currentpriority=0;
  for(vector<string>::iterator it=pathlist.begin();it!=pathlist.end();++it) {  
    currentpriority++;
    int lastsize=filelist1.size();
	  cout<<dryruntext<<"Now scanning \""<<(*it)<<"\"";
    cout.flush();
    int retval=dirlist.walk(*it,0);
    cout<<", found "<<filelist1.size()-lastsize<<" files."<<endl;    
  }
  
  cout<<dryruntext<<"Now have "<<filelist1.size()<<" files in total."<<endl;
  
  

  //mark files with a unique number
  gswd.markitems();

  
  if(remove_identical_inode) {
    //remove files with identical devices and inodes
    gswd.sortlist(&Fileinfo::compareoninode,&Fileinfo::equalinode,
		  &Fileinfo::compareondevice,&Fileinfo::equaldevice,
		  &Fileinfo::compareonpriority,&Fileinfo::equalpriority,
		  &Fileinfo::compareondepth,&Fileinfo::equaldepth);

  if(0) for_each(filelist1.begin(),filelist1.end(),print<Fileinfo>(std::cout));
    
    //mark duplicates - these must be due to hard links or 
    //link that has been followed.
    gswd.marknonuniq(&Fileinfo::equalinode,&Fileinfo::equaldevice);
    
    //remove non-duplicates
    cout<<dryruntext<<"Removed "<<
      gswd.cleanup()<<
      " files due to nonunique device and inode."<<endl;		
  }

  if(0) for_each(filelist1.begin(),filelist1.end(),print<Fileinfo>(std::cout));  



  if(ignoreempty) {
    cout<<dryruntext<<"Now removing files with zero size from list...";
    cout.flush();
    cout<<"removed "<<gswd.remove_if()<<" files"<<endl;
  }

  cout<<dryruntext<<"Total size is "<<gswd.totalsizeinbytes()<<" bytes or ";
  gswd.totalsize(cout)<<endl;
    
  cout<<dryruntext<<"Now sorting on size:";

  sort(filelist1.begin(),
       filelist1.end(),
       Fileinfo::compareonsize);
  
  if(0)  for_each(filelist1.begin(),
		  filelist1.end(),
		  print<Fileinfo>(std::cout));
  
  //mark non-duplicates
  gswd.markuniq(&Fileinfo::equalsize);

  //remove non-duplicates
  cout<<"removed "<<gswd.cleanup()<<" files due to unique sizes from list.";
  cout<<filelist1.size()<<" files left."<<endl;

  if(0) for_each(filelist1.begin(),filelist1.end(), print<Fileinfo>(std::cout));
  
  
  //ok. we now need to do something stronger. read a few bytes.
  const int nreadtobuffermodes=4;
  Fileinfo::readtobuffermode lasttype=Fileinfo::NOT_DEFINED;
  Fileinfo::readtobuffermode type[nreadtobuffermodes];
  type[0]=Fileinfo::READ_FIRST_BYTES;
  type[1]=Fileinfo::READ_LAST_BYTES;
  type[2]=(usemd5 ? Fileinfo::CREATE_MD5_CHECKSUM : Fileinfo::NOT_DEFINED);
  type[3]=(usesha1 ? Fileinfo::CREATE_SHA1_CHECKSUM : Fileinfo::NOT_DEFINED);
  
  for (int i=0;i<nreadtobuffermodes;i++){    
    if(type[i]!=Fileinfo::NOT_DEFINED) {
      string description;
      
      switch(type[i]){
      case Fileinfo::READ_FIRST_BYTES:description="first bytes";break;
      case Fileinfo::READ_LAST_BYTES:description="last bytes";break;
      case Fileinfo::CREATE_MD5_CHECKSUM:description="md5 checksum";break;
      case Fileinfo::CREATE_SHA1_CHECKSUM:description="sha1 checksum";break;
      default:description="--program error!!!---";break;
      }
      cout<<dryruntext<<"Now eliminating candidates based on "<<description<<":";
      
      cout.flush();
      
      //read bytes (destroys the sorting, for efficiency)
      gswd.fillwithbytes(type[i],lasttype,nsecsleep);
      
      //sort on size, bytes
      gswd.sortlist(&Fileinfo::compareonsize,&Fileinfo::equalsize,
		    &Fileinfo::compareonbytes,&Fileinfo::equalbytes);
       
      //mark non-duplicates
      gswd.markuniq(&Fileinfo::equalsize,&Fileinfo::equalbytes);
      
      //remove non-duplicates
      cout<<"removed "<<
	gswd.cleanup()<<
	" files from list.";
      cout<<filelist1.size()<<" files left."<<endl;
      
      lasttype=type[i];
    }
  }
  
  //we now know we have only true duplicates left.

  //figure out how to present the material  

  if(0)  for_each(filelist1.begin(),
		  filelist1.end(),
		  print<Fileinfo>(std::cout));

  //sort, so that duplicates are collected together.    
  gswd.sortlist(&Fileinfo::compareonsize,&Fileinfo::equalsize,
		&Fileinfo::compareonbytes,&Fileinfo::equalbytes,
		&Fileinfo::compareondepth,&Fileinfo::equaldepth,
		&Fileinfo::compareonidentity,&Fileinfo::equalidentity);

  //mark duplicates with the right tag (will stable sort the list
  //internally on priority)
  gswd.markduplicates(&Fileinfo::equalsize,
		      &Fileinfo::equalbytes);
  
  cout<<dryruntext<<"It seems like you have "<<filelist1.size()
      <<" files that are not unique"<<endl;

  cout<<dryruntext<<"Totally, ";
  gswd.saveablespace(cout)<<" can be reduced."<<endl;

  //traverse the list and make a nice file with the results
  if(makeresultsfile) {
    cout<<dryruntext<<"Now making results file "<<resultsfile<<endl;
    gswd.printtofile(resultsfile);
  }

  //traverse the list and replace with symlinks
  if(makesymlinks) {
     cout<<dryruntext<<"Now making symbolic links. creating "<<endl;
    int tmp=gswd.makesymlinks(dryrun);
    cout<<"Making "<<tmp<<" links."<<endl;
    return 0;
  }

  //traverse the list and replace with symlinks
  if(makehardlinks) {
    cout<<dryruntext<<"Now making hard links."<<endl;
    int tmp=gswd.makehardlinks(dryrun);
    cout<<"Making "<<tmp<<" links."<<endl;
    return 0;
  }

 //traverse the list and delete files
  if(deleteduplicates) {
    cout<<dryruntext<<"Now deleting duplicates:"<<endl;
    int tmp=gswd.deleteduplicates(dryrun);
    cout<<"Deleted "<<tmp<<" files."<<endl;
    return 0;
  }
  return 0;
}
