/*
   copyright 20016-2017 Paul Dreik (earlier Paul Sundvall)
   Distributed under GPL v 2.0 or later, at your option.
   See LICENSE for further details.
*/

#ifndef RDFIND_CHECKSUM_HH
#define RDFIND_CHECKSUM_HH

#include <cstddef>

#include <nettle/md5.h>
#include <nettle/sha.h>

/**
 * class for checksum calculation
 */
class Checksum
{
public:
  // these are the checksums that can be calculated
  enum checksumtypes
  {
    NOTSET = 0,
    MD5,
    SHA1,
  };

  // init the object
  int init(int checksumtype);

  int update(std::size_t length, const unsigned char* buffer);
  int update(std::size_t length, const char* buffer);
  int update(long length, const char* buffer);

  /// prints the checksum on stdout
  int print();

  // writes the checksum to buffer.
  // returns 0 if everything went ok.
  int printToBuffer(void* buffer, std::size_t N);

  // returns the number of bytes that the buffer needs to be
  // returns negative if something is wrong.
  int getDigestLength() const;

private:
  // to know what type of checksum we are doing
  int m_checksumtype = NOTSET;
  // the checksum calculation internal state
  union ChecksumStruct
  {
    sha1_ctx sha1;
    md5_ctx md5;
  } m_state;
};

#endif // RDFIND_CHECKSUM_HH
