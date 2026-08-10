#ifndef MCRADCOR_WRAP_H
#define MCRADCOR_WRAP_H

// Fortran forces logical to be four bytes.
// Using uint32_t should make things portable,
// but untested on windows and 32 bit

// For uint32_t. This requires C++11 
#include <cstdint>

#ifndef WIN32
#define common_mcradcor mcradcor_
#define radgen_init radgen_init_
#else
#define common_mcradcor MCRADCOR
#define radgen_init RADGEN_INIT
#endif

struct common_mcradcor_t {
  int mcRadCor;
  int mcRadCor_9999;
  int mcRadCor_ID;
  char   mcRadCor_cType[4];
  double mcRadCor_XTrue;
  double mcRadCor_YTrue;
  double mcRadCor_NuTrue;
  double mcRadCor_Q2True;
  double mcRadCor_W2True;
  double mcRadCor_ThetaBrems;
  double mcRadCor_PhiBrems;
  double mcRadCor_SigRad;
  double mcRadCor_SigCor;
  double mcRadCor_SigCorErr;
  double mcRadCor_TailIne;
  double mcRadCor_TailEla;
  double mcRadCor_TailCoh;
  double mcRadCor_Vacuum;
  double mcRadCor_Vertex;
  double mcRadCor_Small;
  double mcRadCor_RedFac;
  double mcRadCor_EBrems;
};

extern "C" common_mcradcor_t common_mcradcor;

// Note that arguments have to be handed over by reference!
extern "C" void radgen_init( uint32_t& UseLUT, uint32_t& GenLUT );

#endif // MCRADCOR_WRAP_H
