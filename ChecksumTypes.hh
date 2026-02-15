#pragma once

/// these are the checksums that can be calculated. see class Checksum
enum class checksumtypes
{
  NOTSET = 0,
  MD5,
  SHA1,
  SHA256,
  SHA512,
  XXH128
};
