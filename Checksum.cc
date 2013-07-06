//Checksum calculation.
//author Paul Sundvall 20060201
/*
cvs info:
$Revision: 56 $
$Id: Checksum.cc 56 2006-03-15 19:06:35Z pauls $
See LICENSE for details.
*/
#include "Checksum.hh"
#include <stdio.h>

#include <string.h> //for memcpy

//all this rubbish to include sha.h from nettle in the right way
#ifdef __cplusplus 
 extern "C" {
  #include <nettle/sha.h>
  #include <nettle/md5.h>
 }
#else
 #include <nettle/sha.h>
 #include <nettle/md5.h>
#endif 



//this is a small function to print the checksum to stdout
void
Checksum::display_hex(unsigned length, void *data_)
{ 
  uint8_t* data=(uint8_t*)data_;
  unsigned i;
  for (i = 0; i<length; i++)
    printf("%02x", data[i]);
  printf("\n");
}


//destructor
Checksum::~Checksum() {
  release();
}

//deletes allocated memory
int Checksum::release() {
  switch(m_checksumtype) {
  case NOTSET:
    return 0;
    break;
  case SHA1:
    delete (struct sha1_ctx*)m_state;
    delete [] (uint8_t*)m_digest;
    break;
  case MD5:
    delete (struct md5_ctx*)m_state;
    delete [] (uint8_t*)m_digest;
    break;
  default:
    //this is never reached
    return -1;
  }
  m_state=0;
  m_digest=0;
  return 0;
}

//init
int Checksum::init(int checksumtype) {
  
  //first release allocated memory if it exists
  if(release())
    return -1;

  m_checksumtype=checksumtype;

  //one may not init to something stupid
  if(m_checksumtype==NOTSET)
    return -2;

  switch(m_checksumtype) {
  case SHA1:
    {
      m_state=new struct sha1_ctx; 
      m_digest=new uint8_t[SHA1_DIGEST_SIZE]; 
      sha1_init((sha1_ctx*)m_state);
    }
    break;
  case MD5:  
    {
      m_state=new struct md5_ctx; 
      m_digest=new uint8_t[MD5_DIGEST_SIZE]; 
      md5_init((md5_ctx*)m_state);
    }
    break;
  default:
    //not allowed to have something that is not recognized.
    return -1;
  }
  return 0;
}


//update
int Checksum::update(unsigned int length, unsigned char *buffer) {
  switch(m_checksumtype) {
  case SHA1:    
    sha1_update((sha1_ctx*)m_state, length, buffer);
    break; 
  case MD5:    
    md5_update((md5_ctx*)m_state, length, buffer);
    break;
  default:
    return -1;
  }
  return 0;
}





int Checksum::print()
{ 
  switch(m_checksumtype) {
  case SHA1:    	
    sha1_digest( (sha1_ctx*)m_state, SHA1_DIGEST_SIZE, (uint8_t*)m_digest);
    display_hex(SHA1_DIGEST_SIZE, m_digest);
    break;
  case MD5:
    md5_digest( (md5_ctx*)m_state, MD5_DIGEST_SIZE, (uint8_t*)m_digest);
    display_hex(MD5_DIGEST_SIZE, m_digest);
    break;
  default:
    return -1;
 }
  return 0;
}


  //returns the number of bytes that the buffer needs to be
int Checksum::getDigestLength() const
{
  switch(m_checksumtype) {
  case SHA1: return SHA1_DIGEST_SIZE;
  case MD5: return MD5_DIGEST_SIZE;
  default: return -1;
  }
  return -1;
}



  //writes the checksum to buffer
int Checksum::printToBuffer(void *buffer) {
    switch(m_checksumtype) {
  case SHA1:    	
    sha1_digest( (sha1_ctx*)m_state, SHA1_DIGEST_SIZE, (uint8_t*)m_digest);
    memcpy(buffer,m_digest,SHA1_DIGEST_SIZE);
    break;
  case MD5:
    md5_digest( (md5_ctx*)m_state, MD5_DIGEST_SIZE, (uint8_t*)m_digest);
    memcpy(buffer,m_digest,MD5_DIGEST_SIZE);
    break;
  default:
    return -1;
 }
  return 0;
}
