/*
   copyright 20016-2017 Paul Dreik (earlier Paul Sundvall)
   Distributed under GPL v 2.0 or later, at your option.
   See LICENSE for further details.
*/

/**
 * class for checksum calculation
 */
class Checksum
{
public:
  Checksum () : m_checksumtype (NOTSET), m_state (0), m_digest (0) {}

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
  int printToBuffer (void *buffer);

  // returns the number of bytes that the buffer needs to be
  // returns negative if something is wrong.
  int getDigestLength () const;

private:
  // deletes allocated memory
  int release ();
  // prints the checksum to stdout
  void display_hex (unsigned length, void *data_);

  // to know what type of checksum we are doing
  int m_checksumtype;
  // the checksum calculation internal state
  void *m_state;
  // the result is stored in this variable
  void *m_digest;
};
