#ifndef OTHER_COMMONS_H
#define OTHER_COMMONS_H

#include <string>

// declaration of MCSET common clocks
#ifndef WIN32
#define common_pynucl pynucl_
#else
#define common_pynucl PYNUCL
#endif

// Size: 4 doubles, one int
struct common_pynucl_t {

  double INUMOD;
  double CHANUM;  
  int ORDER;
};


extern "C" common_pynucl_t common_pynucl;


#endif // OTHER_COMMONS
