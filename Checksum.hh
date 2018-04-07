/*
   copyright 20016-2017 Paul Dreik (earlier Paul Sundvall)
   Distributed under GPL v 2.0 or later, at your option.
   See LICENSE for further details.
*/

#include <cstddef>

/**
 * class for checksum calculation
 */
class Checksum
{
public:
  Checksum () : m_checksumtype (NOTSET), m_state (0) {}

  ~Checksum ();

  // these are the checksums that can be calculated
  enum checksumtypes
  {
    NOTSET = 0,
    MD5,
    SHA1,
  };

  // init the object
  int init (int checksumtype);

  /// FIXME size_t
  int update (unsigned int length, unsigned char *buffer);

  /// prints the checksum on stdout
  int print ();

  // writes the checksum to buffer.
  // returns 0 if everything went ok.
  int printToBuffer (void *buffer, std::size_t N);

  // returns the number of bytes that the buffer needs to be
  // returns negative if something is wrong.
  int getDigestLength () const;

private:
  // deletes allocated memory
  int release ();
  // prints the checksum to stdout
  static void display_hex (unsigned length, const void *data_);

  // to know what type of checksum we are doing
  int m_checksumtype;
  // the checksum calculation internal state
  void *m_state;

};
