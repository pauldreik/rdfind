/*
   copyright 20016-2017 Paul Dreik (earlier Paul Sundvall)
   Distributed under GPL v 2.0 or later, at your option.
   See LICENSE for further details.
*/

#include "config.h"

// debug macros. pass  --enable-debug=yes to configure to enable it
#ifdef RDFIND_DEBUG
#include <iostream> // for std::cerr
#define RDDEBUG(args)                                                          \
  if (1) {                                                                     \
    std::cerr << __FILE__ << " " << __LINE__ << ":" << args;                   \
  }
#else
#define RDDEBUG(args)                                                          \
  if (0) {                                                                     \
  }
#endif
