/*
   copyright 2006-2017 Paul Dreik (earlier Paul Sundvall)
   Distributed under GPL v 2.0 or later, at your option.
   See LICENSE for further details.
*/

#include "config.h"

// std
#include <array>
#include <cassert>
#include <cstdio>
#include <stdexcept>

// project
#include "Checksum.hh"

Checksum::Checksum(checksumtypes type)
  : m_checksumtype(type)
{
  switch (m_checksumtype) {
    case checksumtypes::SHA1: {
      sha1_init(&m_state.sha1);
    } break;
    case checksumtypes::SHA256: {
      sha256_init(&m_state.sha256);
    } break;
    case checksumtypes::SHA512: {
      sha512_init(&m_state.sha512);
    } break;
    case checksumtypes::MD5: {
      md5_init(&m_state.md5);
    } break;
#ifdef HAVE_LIBXXHASH
    case checksumtypes::XXH128: {
      m_state.xxh128 = XXH3_createState();
      assert(m_state.xxh128 != NULL && "Out of memory!");
      [[maybe_unused]] const auto ret = XXH3_128bits_reset(m_state.xxh128);
      assert(ret == XXH_OK);
    } break;
#endif
    default:
      // not allowed to have something that is not recognized.
      throw std::runtime_error("wrong checksum type - programming error");
  }
}

int
Checksum::update(std::size_t length, const unsigned char* buffer)
{
  switch (m_checksumtype) {
    case checksumtypes::SHA1:
      sha1_update(&m_state.sha1, length, buffer);
      break;
    case checksumtypes::SHA256:
      sha256_update(&m_state.sha256, length, buffer);
      break;
    case checksumtypes::SHA512:
      sha512_update(&m_state.sha512, length, buffer);
      break;
    case checksumtypes::MD5:
      md5_update(&m_state.md5, length, buffer);
      break;
#ifdef HAVE_LIBXXHASH
    case checksumtypes::XXH128: {
      const auto res = XXH3_128bits_update(m_state.xxh128, buffer, length);
      return res == XXH_OK ? 0 : -1;
    }
#endif
    default:
      return -1;
  }
  return 0;
}

int
Checksum::update(std::size_t length, const char* buffer)
{
  return update(
    length,
    static_cast<const unsigned char*>(static_cast<const void*>(buffer)));
}

#if 0
// prints checksum to stdout
static void
display_hex(std::size_t length, const void* data_)
{
  const char* data = static_cast<const char*>(data_);
  for (std::size_t i = 0; i < length; i++) {
    std::printf("%02x", data[i]);
  }
  std::printf("\n");
}
int
Checksum::print()
{
  // the result is stored in this variable
  switch (m_checksumtype) {
    case SHA1: {
      std::array<unsigned char, SHA1_DIGEST_SIZE> digest;
      sha1_digest(&m_state.sha1, SHA1_DIGEST_SIZE, digest.data());
      display_hex(SHA1_DIGEST_SIZE, digest.data());
    } break;
    case SHA256: {
      std::array<unsigned char, SHA256_DIGEST_SIZE> digest;
      sha256_digest(&m_state.sha256, digest.size(), digest.data());
      display_hex(digest.size(), digest.data());
    } break;
    case MD5: {
      std::array<unsigned char, MD5_DIGEST_SIZE> digest;
      md5_digest(&m_state.md5, MD5_DIGEST_SIZE, digest.data());
      display_hex(MD5_DIGEST_SIZE, digest.data());
    } break;
    default:
      return -1;
  }
  return 0;
}
#endif

int
Checksum::getDigestLength() const
{
  switch (m_checksumtype) {
    case checksumtypes::SHA1:
      return SHA1_DIGEST_SIZE;
    case checksumtypes::SHA256:
      return SHA256_DIGEST_SIZE;
    case checksumtypes::SHA512:
      return SHA512_DIGEST_SIZE;
    case checksumtypes::MD5:
      return MD5_DIGEST_SIZE;
#ifdef HAVE_LIBXXHASH
    case checksumtypes::XXH128:
      return sizeof(XXH128_hash_t);
#endif
    default:
      return -1;
  }
  return -1;
}

int
Checksum::printToBuffer(void* buffer, std::size_t N)
{

  assert(buffer);

  switch (m_checksumtype) {
    case checksumtypes::SHA1:
      if (N >= SHA1_DIGEST_SIZE) {
        sha1_digest(
          &m_state.sha1, SHA1_DIGEST_SIZE, static_cast<unsigned char*>(buffer));
      } else {
        // bad size.
        return -1;
      }
      break;
    case checksumtypes::SHA256:
      if (N >= SHA256_DIGEST_SIZE) {
        sha256_digest(&m_state.sha256,
                      SHA256_DIGEST_SIZE,
                      static_cast<unsigned char*>(buffer));
      } else {
        // bad size.
        return -1;
      }
      break;
    case checksumtypes::SHA512:
      if (N >= SHA512_DIGEST_SIZE) {
        sha512_digest(&m_state.sha512,
                      SHA512_DIGEST_SIZE,
                      static_cast<unsigned char*>(buffer));
      } else {
        // bad size.
        return -1;
      }
      break;
    case checksumtypes::MD5:
      if (N >= MD5_DIGEST_SIZE) {
        md5_digest(
          &m_state.md5, MD5_DIGEST_SIZE, static_cast<unsigned char*>(buffer));
      } else {
        // bad size.
        return -1;
      }
      break;
#ifdef HAVE_LIBXXHASH
    case checksumtypes::XXH128:
      if (N >= sizeof(XXH128_hash_t)) {
        XXH128_hash_t result = XXH3_128bits_digest(m_state.xxh128);
        XXH128_canonicalFromHash(static_cast<XXH128_canonical_t*>(buffer),
                                 result);
        XXH3_freeState(m_state.xxh128);
      } else {
        // bad size.
        return -1;
      }
      break;
#endif
    default:
      return -1;
  }
  return 0;
}
