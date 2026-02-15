/*
   copyright 2006-2017 Paul Dreik (earlier Paul Sundvall)
   Distributed under GPL v 2.0 or later, at your option.
   See LICENSE for further details.
*/

#ifndef RDFIND_CHECKSUM_HH
#define RDFIND_CHECKSUM_HH

#include <cstddef>

#include <nettle/md5.h>
#include <nettle/sha1.h>
#include <nettle/sha2.h>
#include <nettle/version.h>

#include "config.h"

#ifdef HAVE_LIBXXHASH
#include <xxhash.h>
#endif

#include "ChecksumTypes.hh"

/**
 * class for checksum calculation
 */
class Checksum
{
public:
  explicit Checksum(checksumtypes type);
  Checksum(const Checksum& other);
  Checksum(Checksum&& other);
  ~Checksum();

  int update(std::size_t length, const unsigned char* buffer);
  int update(std::size_t length, const char* buffer);

  /// makes the object behave as if it was newly constructed
  void reset();

#if 0
  /// prints the checksum on stdout
  int print();
#endif
  // writes the checksum to buffer.
  // returns 0 if everything went ok.
  int printToBuffer(void* buffer, std::size_t N);

  // returns the number of bytes that the buffer needs to be
  // returns negative if something is wrong.
  [[gnu::pure]] int getDigestLength() const;

private:
  // to know what type of checksum we are doing
  const checksumtypes m_checksumtype = checksumtypes::NOTSET;
  // the checksum calculation internal state
  union ChecksumStruct
  {
    sha1_ctx sha1;
    sha256_ctx sha256;
    sha512_ctx sha512;
    md5_ctx md5;
#ifdef HAVE_LIBXXHASH
    XXH3_state_t* xxh128;
#endif
  } m_state;
};

#endif // RDFIND_CHECKSUM_HH
