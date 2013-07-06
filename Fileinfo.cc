/*
a class to hold information about a file.

cvs info:
$Revision: 765 $
$Id: Fileinfo.cc 765 2012-04-19 20:05:09Z pauls $
Author Paul Sundvall 2006
See LICENSE for details.
*/

#include "Fileinfo.hh"

#include <fstream>//for file reading
#include <iostream>//for cout etc
#include <sys/stat.h>//for file info
#include <errno.h>//for errno
#include <unistd.h>//for unlink etc.

#include "Checksum.hh" //checksum calculation



using std::cerr;
using std::endl;

int Fileinfo::fillwithbytes(enum readtobuffermode filltype, 
			    enum readtobuffermode lasttype)
{

  //Decide if we are going to read from file or not.
  //If file is short, first bytes might be ALL bytes!
  if(lasttype!=-1) {
    if(this->size()<= (Fileinfo::filesizetype)m_nbytes) {
      //pointless to read - all bytes in the file are in the field
      //m_somebytes, or checksum is calculated!
      //      cout<<"Skipped reading from file because lasttype="<<lasttype
      //	  <<" and size="<<size()<<endl;
      return 0;
    }
  }

  //set memory to zero 
  //(this command is equivalent to memset(m_somebytes,0,sizeeof(m_somebytes))
  //but this is easier to understand.)
  memset(m_somebytes,0,m_nbytes*sizeof(m_somebytes[0]));
  
  std::fstream f1;
  f1.open(m_filename.c_str(),std::ios_base::in);
  if(!f1.is_open()){
    std::cerr<<"fillwithbytes.cc: Could not open file \""<<m_filename<<"\""<<std::endl;
    return -1;
  }

  
  int checksumtype=Checksum::NOTSET;
  //read some bytes
  switch(filltype) {
  case READ_FIRST_BYTES:
    //read at start of file
    f1.read(m_somebytes,m_nbytes);
    break;
  case READ_LAST_BYTES:
    //read at end of file
    f1.seekg(-m_nbytes,std::ios_base::end);
    f1.read(m_somebytes,m_nbytes);
    break;
  case CREATE_MD5_CHECKSUM://note: fall through is on purpose
    checksumtype=Checksum::MD5;
  case CREATE_SHA1_CHECKSUM:
     checksumtype=Checksum::SHA1;
    {//checksum calculation      
      Checksum chk;
      if(chk.init(checksumtype))
	std::cerr<<"error in checksum init"<<std::endl;
      char buffer[1024];
      int len=1;
      while(len){
	f1.read(buffer,1024);
	len=f1.gcount();
	if(len)
	   if(chk.update((unsigned int)len,(unsigned char*)buffer))
	      	  std::cerr<<"error in update"<<std::endl;
      }

      //store the result of the checksum calculation in somebytes
      int digestlength=chk.getDigestLength();
      if (digestlength<=0 || digestlength>=m_nbytes) 
	std::cerr<<"wrong answer from getDigestLength! FIXME"<<std::endl;
      if(chk.printToBuffer(m_somebytes))
	std::cerr<<"failed writing digest to buffer!!"<<std::endl;
    }
    break;
  default:
    std::cerr<<"does not know how to do that filltype:"<<filltype<<std::endl;
  }
  
  f1.close();
  return 0;
}


bool Fileinfo::readfileinfo(){
  struct stat info;
  m_info.is_file=false;
  m_info.is_directory=false;

  int res;
  do {
    res=stat(m_filename.c_str(), &info);
  } while(res<0 && errno==EINTR);
  
  if (res<0){
    m_info.st_size=0;
    m_info.st_ino=0;
    m_info.st_dev=0;
    std::cerr<<"readfileinfo.cc:Something went wrong when reading file info from \""<<
      m_filename<<"\" :"<<strerror(errno)<<std::endl;
    return false;	    
  }

  //only keep the relevant information
  m_info.st_size=(Fileinfo::filesizetype) info.st_size;
  m_info.st_ino= info.st_ino;
  m_info.st_dev= info.st_dev;

  m_info.is_file=S_ISREG(info.st_mode)?true:false;
  m_info.is_directory=S_ISDIR(info.st_mode)?true:false;
  return true; 
}



const std::string Fileinfo::getduptypestring(const Fileinfo &A) {

  switch (A.getduptype()) {
  case DUPTYPE_UNKNOWN:
    return "DUPTYPE_UNKNOWN";
  case DUPTYPE_FIRST_OCCURRENCE:
    return "DUPTYPE_FIRST_OCCURRENCE";
  case DUPTYPE_WITHIN_SAME_TREE:
    return "DUPTYPE_WITHIN_SAME_TREE";
  case DUPTYPE_OUTSIDE_TREE:
    return "DUPTYPE_OUTSIDE_TREE";
  default:
    std::cerr<<"error. does not know that one"<<std::endl;
  }
  return "error-error";
}

//constructor
Fileinfo::Fileinfostat::Fileinfostat() {
  st_size=99999;
  st_ino =99999;
  st_dev =99999;
  is_file    =false;
  is_directory=false;
}

int Fileinfo::deletefile()
{
  return unlink(name().c_str());
}


int simplifyPath(std::string &path){
  //replace a/./b to a/b
  std::string::size_type pos=std::string::npos;
  do {
    pos=path.find("/./");
    if(pos!=std::string::npos) {
      path.replace(pos,3,"/");
    }
  } while(pos!=std::string::npos);
  //we should get rid of /../ also, but that is more difficult.
  //future work...
  return 0;
}

//prepares target, so that location can point to target in 
//the best way possible
int makeReadyForLink(std::string &target, const std::string &location_)
{
  std::string location(location_);

  //simplify target and location
  simplifyPath(location);
  simplifyPath(target);

  //if target is not absolute, let us make it absolute
  if(target.length()>0 && target.at(0)=='/') {
    //absolute. do nothing.
    //    std::cout<<"absolute"<<std::endl;
  } else {
    //not absolute. make it absolute.
    //    std::cout<<"not absolute"<<std::endl;

    //yes, this is possible to do with dynamically allocated memory, 
    //but it is not portable then (and more work).
    const size_t buflength=256;
    char buf[buflength];
    if(buf!=getcwd(buf,buflength)) {
      std::cerr<<"failed to get current working directory"<<std::endl;
      return -1;
    }
    target=std::string(buf)+std::string("/")+target;
    //    std::cout<<"target is now "<<target<<std::endl;
  }
  return 0;
}

//makes a symlink that points to A
int Fileinfo::makesymlink(const Fileinfo &A) {
  int retval=0;
  //step 1: remove the file
  retval=unlink(name().c_str());
  if(retval) {
    cerr<<"failed to unlink file "<<name()<<endl;
    return retval;
  }

  //step 2: make a symlink.
  //the tricky thing is that the path must be correct, as seen from 
  //the directory where *this is.

  //simplifiy A and *this, so that asdf/../asdfdf are removed

  std::string target=A.name();
  makeReadyForLink(target,name());

  //  std::cout<<"will call symlink("<<target<<","<<name()<<")"<<std::endl;
  retval=symlink(target.c_str(),name().c_str());
  if(retval) {
    cerr<<"failed to make symlink "<<name()<<" to "<<A.name()<<endl;
    return retval;
  }

  return retval;
}

//makes a hard link that points to A
int Fileinfo::makehardlink(const Fileinfo &A) {
  int retval=0;
  //step 1: remove the file
  retval=unlink(name().c_str());
  if(retval) {
    cerr<<"failed to unlink file "<<name()<<endl;
    return retval;
  }
  
  //step 2: make a hardlink.
  retval=link(A.name().c_str(),name().c_str());
  if(retval) {
    cerr<<"failed to make hardlink "<<name()<<" to "<<A.name()<<endl;
    return retval;
  }

  return retval;
}

int Fileinfo::static_deletefile(Fileinfo &A, const Fileinfo &B) {
  //delete A.

  //  cout<<"wants to delete file "<<A.name()<<endl;
  //  return 0;

  return A.deletefile();
}

int Fileinfo::static_makesymlink(Fileinfo &A, const Fileinfo &B) { 
  //  cout<<"wants to make symlink from file "<<A.name()<<" to "<<B.name()<<endl;
  //  return 0;
  return A.makesymlink(B);
}

int Fileinfo::static_makehardlink(Fileinfo &A, const Fileinfo &B) { 
  //  cout<<"wants to make hardlink from file "<<A.name()<<" to "<<B.name()<<endl;
  //  return 0;
  return A.makehardlink(B);
}



bool Fileinfo::compareonbytes(const Fileinfo &a, const Fileinfo &b)
{
  int retval=memcmp(a.getbyteptr(),b.getbyteptr(),m_nbytes);
  return (retval<0);
  /*
  for(int i=0;i<m_nbytes;i++)
    if(a.getreadbyte(i)<b.getreadbyte(i))
      return true;
  return false;
  */
}

bool Fileinfo::equalbytes(const Fileinfo &a, const Fileinfo &b)
{
  int retval=memcmp(a.getbyteptr(),b.getbyteptr(),m_nbytes);
  return (retval==0);
}





bool Fileinfo::compareonsizeandfirstbytes(const Fileinfo &a, const Fileinfo &b)
{
  if (a.size()>b.size())
    return true;
  if (a.size()<b.size())
    return false;
  //must be equal. compare on bytes.
  return compareonbytes(a,b);
}


bool Fileinfo::equalsize(const Fileinfo &a, const Fileinfo &b)
{
  if (a.size()==b.size())
    return true;
  else
    return false;
}
