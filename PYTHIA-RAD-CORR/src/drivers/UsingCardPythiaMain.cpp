//--------------------------------------------------------------------
//  UsingCardPythiaMain.cpp
//  Author: Kolja Kauder
//
//  Generate EIC events with BNL-customized Pythia6
//  using the TPythia interface in ROOT.
//  Read from existing STEER files
//  Note that the output is a bit scrambled because
//  c++ buffers and fortran (pythia) buffers don't play nice
// 
// Future: This program can be expanded to produce the same Lund
//         ascii output as the fortran one, or root trees directly
//         (e.g. for eic-smear), or something like HepMC
//
//  --------> p       e <--------- 
//--------------------------------------------------------------------
#include "TPythia6.h"
#include "TParticle.h"
#include "TClonesArray.h"
#include "TString.h"

#include <iostream>
#include <sstream>
#include <string>

// For uint32_t. This requires C++11.
#include <cstdint>
// For good seeds from random_device. Requires C++11.
#include <random> 

using namespace std;

// This is very important
// The extensions to pythia rely on global common blocks
// interfaced in these headers.
#include "pythia-radcorr/mc_set.h"
#include "pythia-radcorr/mcRadCor_inc.h"
#include "pythia-radcorr/other_commons.h"

// to directly interface with lhapdf, use things like
#ifndef WIN32
#define lhapdfsta pdfsta_
#else
#define lhapdfsta PDFSTA
#endif
extern "C" void lhapdfsta ( void );

// forward declaration
// Remove commas from read lines
void CleanLine (string& line);

// Main Program
// ------------
int main(int argc, char* argv[])
{

  // Set up pythia6 object
  // ---------------------
  TPythia6 *pythia = new TPythia6;  

  // default values as in pythia6-eic.f
  common_mcset.iModel=0;
  common_mcset.pbeam=100.;
  common_mcset.ebeam=4.0;

  int ltype=11;
  common_mcset.masse=pythia->Pymass(11);
  common_mcset.massp=pythia->Pymass(2212);

  long NEV = 100;
  long NPRT = 100;

  //   cout << "Usage: " << argv[0] << " < {Steering file}" << endl;
  //   return -1;

  // Sigh... need to parse things like "1,2", "1, 2" etc.
  // Fortran READ(*,*) happily uses commas and whitespace
  // as delimiter, C++ is more selective.
  // Using root to remove all commas since regex_replace doesn't 
  // work properly in gcc 4.8
  int lineno = 0;
  string line;
  
  // Read output file name
  string outfile; // Note that this is not used!
  std::getline(std::cin, line);  lineno++;  CleanLine ( line );
  stringstream( line ) >> outfile; 
  // Read lepton beam type;
  std::getline(std::cin, line);  lineno++;  CleanLine ( line );
  stringstream( line ) >> ltype;
  // Read parameters for PYINIT call (beam and target particle energy).
  std::getline(std::cin, line);  lineno++;  CleanLine ( line );
  stringstream( line ) >> common_mcset.pbeam >> common_mcset.ebeam;
  // Read number of events to generate, and to print.
  std::getline(std::cin, line);  lineno++;  CleanLine ( line );
  stringstream( line ) >> NEV >> NPRT;
  // Read min/max x of radgen lookup table
  std::getline(std::cin, line);  lineno++;  CleanLine ( line );
  stringstream( line ) >> common_mcset.mcSet_XMin >> common_mcset.mcSet_XMax;
  // Read min/max y of generation range
  std::getline(std::cin, line);  lineno++;  CleanLine ( line );
  stringstream( line ) >> common_mcset.mcSet_YMin >> common_mcset.mcSet_YMax;
  // Read min/max Q2 of generation range
  std::getline(std::cin, line);  lineno++;  CleanLine ( line );
  stringstream( line ) >> common_mcset.mcSet_Q2Min >> common_mcset.mcSet_Q2Max;
  // Read information for cross section used in radgen
  string hfstruct,hr;
  std::getline(std::cin, line);  lineno++;  CleanLine ( line );
  stringstream( line ) >> hfstruct >> hr;
  strncpy( common_mcset.genSet_FStruct, hfstruct.c_str(), 4 ); // allocated exactly 4 bytes, so don't copy null terminator
  strncpy( common_mcset.genSet_R, hr.c_str(), 4 );  
  // Read parameters of radcorr: do radcorr (1), generate look-up table (2)
  std::getline(std::cin, line);  lineno++;  CleanLine ( line );
  stringstream( line ) >> common_mcset.qedrad;
  // Read parameters for PYTHIA-Model = which generation is done     
  std::getline(std::cin, line);  lineno++;  CleanLine ( line );
  stringstream( line ) >> common_mcset.iModel;
  // Read target type mass and charge
  std::getline(std::cin, line);  lineno++;  CleanLine ( line );
  stringstream( line ) >> common_mcset.mcSet_TarA >> common_mcset.mcSet_TarZ;
  std::getline(std::cin, line);  lineno++;  CleanLine ( line );
  stringstream( line ) >> common_pynucl.INUMOD >> common_pynucl.CHANUM;
  // Read nuclear pdf correction order
  std::getline(std::cin, line);  lineno++;  CleanLine ( line );
  stringstream( line ) >> common_pynucl.ORDER;

  // Read information for cross section used in radgen
  while ( std::getline(std::cin, line) ){
    pythia->Pygive( line.c_str() );
  }


  cout << " *********************************************" << endl
       << " NOW all parameters are read by PYTHIA"  << endl
       << " ********************************************* " << endl;

  // Random seed. Note that this is NOT safe for batch jobs.
  // better idea would be to get a seed from /dev/urandom on UNIX type machines.
  // time_t initseed = time(0);
  // initseed=42;
  // Better c++11 solution:
  std::random_device rd;
  int initseed = rd();
  pythia->SetMRPY(1, initseed);
  cout << " SEED = " << initseed << endl;

  // proton is defined in positive z and as target
  pythia->SetP(2, 1, 0.); // p
  pythia->SetP(2, 2, 0.);
  pythia->SetP(2, 3, common_mcset.pbeam);

  // lepton is defined in negative z and as beam
  pythia->SetP(1, 1, 0.); // e
  pythia->SetP(1, 2, 0.);
  pythia->SetP(1, 3, -common_mcset.ebeam);
  
  if (common_mcset.mcSet_TarZ == 0) {
    common_mcset.massp=pythia->Pymass(2112);
  } else {
    common_mcset.massp=pythia->Pymass(2212);
  }
  common_mcset.masse=pythia->Pymass(ltype);

  // Not sure if all of this is necessary
  double pbeamE=sqrt(common_mcset.pbeam*common_mcset.pbeam+common_mcset.massp*common_mcset.massp);
  double pbeta=common_mcset.pbeam/pbeamE;
  double pgamma=pbeamE/common_mcset.massp;

  double ebeamE=sqrt(common_mcset.ebeam*common_mcset.ebeam+common_mcset.masse*common_mcset.masse);
  double ebeamEnucl=pgamma*ebeamE-pgamma*pbeta*(-common_mcset.ebeam);
  double epznucl=-pgamma*pbeta*(ebeamE)+pgamma*(-common_mcset.ebeam);
  common_mcset.mcSet_EneBeam=ebeamEnucl;
  cout << ebeamEnucl << "  " << ebeamE << "  " << epznucl << "  " << -common_mcset.ebeam << endl;

  // Important!! Easy to forget, impossible to detect
  common_mcevnt.ebeamEnucl = ebeamEnucl;
  
  double sqrts=sqrt(2.*common_mcset.pbeam*common_mcset.ebeam+2.*pbeamE*ebeamE
		    + common_mcset.massp*common_mcset.massp+common_mcset.masse*common_mcset.masse);
  cout << "********************************************* " << endl;
  cout << "proton beam momentum: " << common_mcset.pbeam << " GeV" << endl;
  cout << "lepton beam momentum: " << common_mcset.ebeam << " GeV" << endl;
  cout << "resulting sqrt(s): " << sqrts << " GeV" << endl;
  cout << "*********************************************" << endl;
  	  
  // Now the radgen initialization
  uint32_t UseLUT=0;
  uint32_t GenLUT=0;
  if ( common_mcset.iModel == 0){
    UseLUT=0;
    GenLUT=0;
    common_mcset.qedrad=0;
    pythia->SetMSTP(199, 0);
    common_mcradcor.mcRadCor_EBrems=0.0;
  } else if ( common_mcset.iModel == 1){
    switch ( common_mcset.qedrad ){
    case 0 :
      common_mcradcor.mcRadCor_EBrems=0.0;
      UseLUT=0;
      GenLUT=0;
      pythia->SetMSTP(199, 1);
      break;
    case 1 :
      common_mcradcor.mcRadCor_EBrems=0.0;
      UseLUT=1;
      GenLUT=0;
      pythia->SetMSTP(199, 1);
      radgen_init(UseLUT,GenLUT);
      cout << "I have initialized radgen" << endl;
      break;
    case 2 :
      cout << "radgen lookup table will be generated" << endl;
      common_mcradcor.mcRadCor_EBrems=0.0;
      UseLUT=1;
      GenLUT=1;
      pythia->SetMSTP(199, 1);
      radgen_init(UseLUT,GenLUT);
      cout << "lookup table is generated;" << endl;
      cout << "to run now pythia change parameter qedrad to 1" << endl;
      return 0;      
    default : 
      cerr << "iModel = " << common_mcset.iModel
	   << " and qedrad = " << common_mcset.qedrad 
	   << " not supported." << endl;
      return -1;
      break;
    }
  } else {
    cerr << "iModel = " << common_mcset.iModel << " not supported." << endl;
  }


  // Strings for initialization
  string target = "p+";
  string beam = "gamma/e-";
  string frame = "3MOM";
  // for fixed target, use frame = "FIXT";
  if ( common_mcset.pbeam <= 1e-4 ) {
    frame = "FIXT";
    common_mcset.pbeam=0;
  }
  // Other options exist, see manual
  double WIN = common_mcset.ebeam; // for 3MOM, this is a dummy, for FIXT, it needs to be the beam energy
    
  if (common_mcset.mcSet_TarZ>=1  ) target = "p+";
  if (common_mcset.mcSet_TarZ<=-1 ) target = "pbar-";
  if (common_mcset.mcSet_TarZ ==0 ) target = "n0";

  switch ( ltype ) {
  case  11 : beam = "gamma/e-";
    break;
  case -11 : beam = "gamma/e+";
    break;
  default : cerr << "Unsupported beam flavor " << ltype << endl;
    return -1;
    break;
  }
  
  pythia->Initialize(frame.c_str(), beam.c_str(),target.c_str(), WIN);

  // For debugging, cough up all information
  // cout << ltype << endl;
  // cout << common_mcset.pbeam << common_mcset.ebeam << endl;
  // cout << common_mcset.mcSet_XMin << common_mcset.mcSet_XMax << endl;
  // cout << common_mcset.mcSet_YMin << common_mcset.mcSet_YMax << endl;  
  // cout << common_mcset.mcSet_Q2Min << common_mcset.mcSet_Q2Max;
  // cout << common_mcset.qedrad << endl;
  // cout << common_mcset.iModel  << endl;
  // cout << common_mcset.mcSet_TarA << common_mcset.mcSet_TarZ << endl;
  // cout << common_pynucl.INUMOD << common_pynucl.CHANUM  << endl;
  // cout << common_pynucl.ORDER << endl;
  // printf("%.*s , %.*s \n", 4, common_mcset.genSet_FStruct, 4, common_mcset.genSet_R);  // cout chokes on the lack of 0 termination
  // pythia->Pystat(1);
  // pythia->Pystat(2);
  // pythia->Pystat(3);
  // pythia->Pystat(4);
  // pythia->Pystat(5);
  // pythia->Pystat(6);
  // pythia->Pystat(7);
  // pythia->Pystat(8);
  // return 0;

			    
  // From here on, we could book root files and histos, or we could
  // try and reproduce the same text output as the fortran file,
  // or run an analysis...
  double trueX;
  double trueW2;
  double trueNu;

      
  // ---------------------------------------------------------------------
  // ...Event generation loop
  // ---------------------------------------------------------------------
  // using fortran counting convention...
  long IEV = 1;
  TClonesArray* particles = new TClonesArray("TParticle");
  while ( IEV<= NEV ){
    pythia->GenerateEvent();

    // RadCorr could render this event invalid, requiring a do-over
    if ( pythia->GetMSTI(61) == 1 ) {
      cout << " go back to PYEVNT call" << endl;
      continue;
    }

    // Display the first NPRT events to stdout
    if (IEV <= NPRT) pythia->Pylist(2);

    // fortran driver has more book-keeping, starting with
    // ievent=IEV
    // genevent=NGEN(0,3)-lastgenevent

    // some event variables
    trueX =  pythia->GetVINT(307)/pythia->GetVINT(309)/(2.*common_mcset.pbeam*common_mcset.ebeam+2.*pbeamE*ebeamE);
    trueW2 = pow(common_mcset.massp,2) + pythia->GetVINT(307)*(1.0/trueX-1.0);
    trueNu = (trueW2 + pythia->GetVINT(307) - pow(common_mcset.massp,2))/(2.*common_mcset.massp);

    // Run over final state hadrons
    int numParticles = pythia->GetNumberOfParticles();
    int obtainedParticles = pythia->ImportParticles(particles, "All");
    if (numParticles != obtainedParticles) {
      cout << "Warning: could not obtain all particles generated." << endl;
    }  

    for (int i=0; i<obtainedParticles; ++i ){ 
      auto particle =  static_cast<TParticle*>(particles->ConstructedAt(9));
      if ( particle->GetStatusCode() != 1 ) continue; // Only final state
      if ( particle->GetPdgCode() == 11 ) continue; // skip electron
      double z = particle->Energy() / trueNu;
      if (z>1) {
	cout << "E = " << particle->Energy() << " nu  = "  << trueNu << " z = " << z <<endl;
	pythia->Pylist(1);
      }
    }
    
    // More quantities and output is done in the fortran version:
//       if (mcRadCor_EBrems.gt.0.) then
//          radgamEnucl=sqrt(dplabg(1)**2+dplabg(2)**2+dplabg(3)**2)
//          radgamE=pgamma*radgamEnucl-pgamma*pbeta*dplabg(3)
//          radgamp=-pgamma*pbeta*radgamEnucl+pgamma*dplabg(3)
// C         write(*,*) radgamEnucl, radgamE, dplabg(3), radgamp
//       else
//         radgamEnucl=0D0
//         radgamE=0D0
//         radgamp=0D0 
//       endif

//          tracknr=N
//          if (mcRadCor_EBrems.gt.0.) then
//             nrtrack=tracknr+1
//          else
//             nrtrack=tracknr
//          endif

//          if ((msti(1).ge.91).and.(msti(1).le.94)) msti(16)=0
           
//          write(29,32) 0, ievent, genevent, msti(1), msti(12), 
//      &        msti(16), pari(34), msti(15), pari(33), pari(53), 
//      &        VINT(309), VINT(307), trueX, trueW2, trueNu,
//      &        VINT(313), pari(14), pari(15), pari(16), 
//      &        pari(18),  pari(22), sngl(py6f2), sngl(py6f1), 
//      &        py6r, mcRadCor_Sigrad, mcRadCor_sigcor, radgamEnucl,
//      &        VINT(319), VINT(45), nrtrack 
//  32      format((I4,1x,$),(I10,1x,$),3(I4,1x,$),(I10,1x,$),f9.6,1x,$,
//      &         I12,1x,$,
//      &         2(f12.6,1x,$),7(f18.11,3x,$),12(f19.9,3x,$),I12,/)
//          write(29,*)'============================================'

//          DO I=1,tracknr
//          if (K(I,3).le.nrtrack) then
//          write(29,34) I,K(I,1),K(I,2),K(I,3),K(I,4),K(I,5),
//      &        P(I,1),P(I,2),P(I,3),P(I,4),P(I,5),
//      &        V(I,1),V(I,2),V(I,3)
//          endif
//          ENDDO
//          if (mcRadCor_EBrems.gt.0.) then
//             write(29,34) nrtrack, 55, 22, 1, 0, 0,
//      &      sngl(dplabg(1)),sngl(dplabg(2)),sngl(-radgamp),
//      &      sngl(radgamE), 0., 0., 0., 0.
//          endif
//  34      format(2(I6,1x,$),I10,1x,$,3(I8,1x,$),8(f15.6,1x,$),/)
//          write(29,*)'=============== Event finished ==============='
//          lastgenevent=NGEN(0,3)

    ++IEV;
  }
  
  
  // Finishing up
  // ------------
  pythia->Pystat(1);
  pythia->Pystat(4);
  cout << "The charm mass used is: "<< pythia->GetPMAS(4,1) << endl;

  // Print the Pythia cross section which is needed to get an absolut 
  // normalisation the number is in microbarns
  cout << "===================================================" << endl;
  cout << "Pythia total cross section normalisation:"
       << pythia->GetPARI(1)*1000.0 << " microbarn" << endl;
  cout << "Total Number of generated events " << pythia->GetMSTI(5) << endl;
  // no interface is offered to NGEN.
  // However, the number returned by the fortran code also does not correctly reflect
  // the number given in Pystat, so this call is questionable to begin with.
  // If needed, could access with extern "c" like other common block data
  // cout << "Total Number of trials " << pythia->GetNGEN(0,3) << endl;
  cout << "===================================================" << endl;

  if ( common_mcset.qedrad == 2) {
    cout << "lookup table is generated;" << endl;
    cout << "to run now pythia change parameter qedrad to 1" << endl;
  }

  // Check pdf status
  // ----------------
  lhapdfsta();

 return 0;
}


// -----------------------------------------------
void CleanLine (string& line) {
  // Curse you, gcc 4.8. This should be fine.
  // Except you mess up regexes. So I guess we'll use root
  // std::regex commare(",");
  // line = std::regex_replace( line, commare, " ");

  TString l = line;
  l.ReplaceAll(","," ");
  line = string(l.Data());
  return;
}
// -----------------------------------------------

