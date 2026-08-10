#ifndef MC_SET_H
#define MC_SET_H

#include <string>

// declaration of MCSET common blocks
#ifndef WIN32
# define common_mcset common_mc_set_
# define common_mcevnt mcevnt_
#else
# define common_mcset COMMON_MC_SET
# define common_mcevnt MCEVNT
#endif

/* // Size: 11 doubles, 4 ints, 3 strings of 4 chars each */
/* // I wish I could do this better, but I'm assuming */
/* // sizeof(int) = 2, sizeof(double) = 4, sizeof (char)=1 */
/* char common_mcset[4*2 + 11*4 + 3*4*1]; */

struct common_mcset_t {

  float mcSet_EneBeam;
  int mcSet_TarA;
  int mcSet_TarZ;
  float mcSet_Q2Min;
  float mcSet_Q2Max;
  float mcSet_YMin;
  float mcSet_YMax;

  int qedrad;
  int iModel;

  char genSet_FStruct[4];
  char genSet_R[4];
  char mcSet_PTarget[4];

  float mcSet_XMin;
  float mcSet_XMax;
  float massp;
  float masse;
  float ebeam;
  float pbeam;
};

/* // Size: 10 doubles */
/* double common_mcent[10]; */

struct common_mcevnt_t{  
  double weight;
  double genq2, gennu, genx, geny, genw2;
  double genthe, genphi, geneprim, ebeamEnucl;
};


extern "C" common_mcset_t common_mcset;
extern "C" common_mcevnt_t common_mcevnt;


/*   void *mcset_common_address(const char* name) { */
/*    if      (!strcmp(name,"PYJETS")) return pyjets; */
/*    else if (!strcmp(name,"PYDAT1")) return pydat1; */
/*    else if (!strcmp(name,"PYDAT2")) return pydat2; */
/*    else if (!strcmp(name,"PYDAT3")) return pydat3; */
/*    else if (!strcmp(name,"PYDAT4")) return pydat4; */
/*    else if (!strcmp(name,"PYDATR")) return pydatr; */
/*    else if (!strcmp(name,"PYSUBS")) return pysubs; */
/*    else if (!strcmp(name,"PYPARS")) return pypars; */
/*    else if (!strcmp(name,"PYINT1")) return pyint1; */
/*    else if (!strcmp(name,"PYINT2")) return pyint2; */
/*    else if (!strcmp(name,"PYINT3")) return pyint3; */
/*    else if (!strcmp(name,"PYINT4")) return pyint4; */
/*    else if (!strcmp(name,"PYINT5")) return pyint5; */
/*    else if (!strcmp(name,"PYINT6")) return pyint6; */
/*    else if (!strcmp(name,"PYINT7")) return pyint7; */
/*    else if (!strcmp(name,"PYINT8")) return pyint8; */
/*    else if (!strcmp(name,"PYINT9")) return pyint9; */
/*    else if (!strcmp(name,"PYUPPR")) return pyuppr; */
/*    else if (!strcmp(name,"PYMSSM")) return pymssm; */
/*    else if (!strcmp(name,"PYSSMT")) return pyssmt; */
/*    else if (!strcmp(name,"PYINTS")) return pyints; */
/*    else if (!strcmp(name,"PYBINS")) return pybins; */
/*    return 0; */
/* } */
   



#endif // MC_SET_H
