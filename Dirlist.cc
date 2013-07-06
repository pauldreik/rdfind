/*
files for reading entries from a file structure.

Author Paul Sundvall 2006
see LICENSE for details.
$Revision: 803 $
$Id: Dirlist.cc 803 2013-01-26 04:22:16Z paul $
 */
#include "config.h"
#include "Dirlist.hh"
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <errno.h>//for errno

#include "RdfindDebug.hh" //debug macros

int Dirlist::walk(const std::string &dir,const int recursionlevel)
{
  using namespace std;
  struct stat info;
  
  RDDEBUG("Now in walk with dir="<<dir.c_str()<<" and recursionlevel="
	  <<recursionlevel<<std::endl);

  if(recursionlevel<m_maxdepth) 
    {
      //open the directory
      DIR * dirp=NULL;
      dirp = opendir(dir.c_str());
      if(dirp) {
	//we opened the directory. let us read the content.
	RDDEBUG("opened directory"<<std::endl);
	struct dirent* dp=NULL;
	while(0!=(dp = readdir( dirp ))){ 
	  if(0!=strcmp(".",dp->d_name)) {
	    if(0!=strcmp("..",dp->d_name)) {
	      //take this item, and find out more about it.

	      //investigate what kind of file it is, dont follow any symlinks
	      //when doing this (lstat instead of stat).
	      int statval=lstat((dir+"/"+std::string(dp->d_name)).c_str(),&info);
	      bool dowalk=false;
	      if(statval==0) {
		//investigate what kind of item it was.

		if(S_ISLNK(info.st_mode)) {
		  //cout<<"encountered symbolic link "<<dir<<"/"
		  //<<dp->d_name<<endl;
		  (*m_report_symlink)(dir,
				      string(dp->d_name),
				      recursionlevel);
		  if(m_followsymlinks)
		    dowalk=true;
		}

		if(S_ISDIR(info.st_mode)) {
		  //cout<<"it is a directory!"<<dir<<"/"
		  //<<dp->d_name<<endl;
		  (*m_report_directory)(dir,
					string(dp->d_name),
					recursionlevel);
		  dowalk=true;
		}

		if(S_ISREG(info.st_mode)) {
		  //cout<<"it is a regular file!"<<dir<<"/"
		  //<<dp->d_name<<endl;
		  (*m_report_regular_file)(dir,
					   string(dp->d_name),
					   recursionlevel);
		}

		//try to open directory
		if(dowalk)
		  walk(dir+"/" + dp->d_name , recursionlevel+1);

	      } else {
		//failed to do stat
		(*m_report_failed_on_stat)(dir,
					   string(dp->d_name),
					   recursionlevel);
	      }
	    }
	  }
	}
	//close the directory
	(void) closedir(dirp);
	return 2;//its a directory
      } else {
	//failed to open directory (dirp==NULL)
	RDDEBUG("failed to open directory"<<std::endl);
	//this can be due to rights, or some other error.
	handlepossiblefile(dir,recursionlevel);
	return 1;//its a file (or something else)
      }
      return 0;//recusion limit exceeded
      
    } else {
      cerr<<"recursion limit exceeded"<<endl;
      return -1;
    }
}


//splits inputstring into path and filename. if no / character is found,
//empty string is returned as path and filename is set to inputstring.
int splitfilename(std::string &path,std::string &filename,
		  const std::string &inputstring) {
  using std::string;

string::size_type pos = inputstring.rfind ('/');
 if (pos == string::npos) {
   path="";
   filename=inputstring;
   return -1;
 }

 path=inputstring.substr(0,pos+1);
 filename=inputstring.substr(pos+1,string::npos);
 return 0;
}

//this function is called for files that were believed to be directories,
//or failed re
int Dirlist::handlepossiblefile(const std::string &possiblefile,
				int recursionlevel) {
  using namespace std;
  struct stat info;
  
  RDDEBUG("Now in handlepossiblefile with name "<<possiblefile.c_str()
	  <<" and recursionlevel "<<recursionlevel<<std::endl);
  

  //split filename into path and filename
  string path,filename;
  splitfilename(path,filename,possiblefile);

  RDDEBUG("split filename is path="<<path.c_str()
	  <<" filename="<<filename.c_str()<<std::endl);

  //investigate what kind of file it is, dont follow symlink
  int statval=0;
  do {
    statval=lstat(possiblefile.c_str(),&info);
  } while(statval<0 && errno==EINTR);
  
  if(statval<0) {
    //probably file does not exist, or trouble with rights.
    RDDEBUG("got negative statval "<<statval<<std::endl);
    (*m_report_failed_on_stat)(path,
			       filename,
			       recursionlevel);     
    return -1;
  } else {
    RDDEBUG("got positive statval "<<statval<<std::endl);
  }

  
  /*     cout<<"input="<<possiblefile<<endl;
	 cout<<"path="<<path<<endl;
	 cout<<"filename="<<filename<<endl;
  */
  
  if(S_ISLNK(info.st_mode)) {
    RDDEBUG("found symlink"<<std::endl);
    (*m_report_symlink)(path,
			filename,
			recursionlevel);     
    return 0;
  } else {
    RDDEBUG("not a symlink"<<std::endl);
  }
  
  if(S_ISDIR(info.st_mode)) {  
    cerr<<"Dirlist.cc::handlepossiblefile: This should never happen. FIXME! details on the next row:"<<endl;
    cerr<<"possiblefile=\""<<possiblefile<<"\""<<endl;
    //this should never happen, because this function is only to be called
    //for items that can not be opened with opendir.
    //maybe it happens if someone else is changing the file while we
    //are reading it?
    return -2;
  } else {
    RDDEBUG("not a dir"<<std::endl);
  }
  
  if(S_ISREG(info.st_mode)) {    
    //    cout<<"found regular file"<<endl;
    RDDEBUG("it is a regular file"<<std::endl);
    (*m_report_regular_file)(path,
			     filename,
			     recursionlevel);     
    return 0;
  } else {
    RDDEBUG("not a regular file"<<std::endl);
  }
  cout<<"Dirlist.cc::handlepossiblefile: found something else than a dir or a regular file."<<endl;
  return -1;
}
