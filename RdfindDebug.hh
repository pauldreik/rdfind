#include "config.h"

//debug macros. pass  --enable-debug=yes to configure to enable it
#ifdef RDFIND_DEBUG
#include <iostream> // for std::cerr
# define RDDEBUG(args) if(1) {			\
    std::cerr<<__FILE__<<" "<<__LINE__<<":"<<args;	\
  }
#else
# define RDDEBUG(args) if(0) {			\
  }
#endif
