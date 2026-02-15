#pragma once

#include "config.h"

#include <cstddef>
#include <string>

#include "ChecksumTypes.hh"
#include "Fileinfo.hh"

class Parser;

struct Options
{
  // operation mode and default values
  bool makesymlinks = false;   // turn duplicates into symbolic links
  bool makehardlinks = false;  // turn duplicates into hard links
  bool makeresultsfile = true; // write a results file
  Fileinfo::filesizetype minimumfilesize =
    1; // minimum file size to be noticed (0 - include empty files)
  Fileinfo::filesizetype maximumfilesize =
    0; // if nonzero, files this size or larger are ignored
  bool deleteduplicates = false;      // delete duplicate files
  bool followsymlinks = false;        // follow symlinks
  bool dryrun = false;                // only dryrun, don't destroy anything
  bool remove_identical_inode = true; // remove files with identical inodes
  bool usemd5 = false;       // use md5 checksum to check for similarity
  bool usesha1 = false;      // use sha1 checksum to check for similarity
  bool usesha256 = false;    // use sha256 checksum to check for similarity
  bool usesha512 = false;    // use sha512 checksum to check for similarity
  bool usexxh128 = false;    // use xxh128 checksum to check for similarity
  bool nochecksum = false;   // skip using checksumming (unsafe!)
  bool deterministic = true; // be independent of filesystem order
  bool showprogress = false; // show progress while reading file contents
  std::size_t buffersize = 1 << 20; // chunksize to use when reading files
  long nsecsleep = 0; // number of nanoseconds to sleep between each file read.
  std::string resultsfile = "results.txt"; // results file name.
  std::uint64_t first_bytes_size =
    64; // how much to read during the "read first bytes" step
  std::uint64_t last_bytes_size =
    64; // how much to read during the "read last bytes" step
  /// checksum used for first and last bytes
  checksumtypes checksum_for_firstlast_bytes =
#ifdef HAVE_LIBXXHASH
    checksumtypes::XXH128;
#else
    // the fastest one after xxh128 is sha1
    checksumtypes::SHA1;
#endif
};

void
usage();

Options
parseOptions(Parser& parser);
