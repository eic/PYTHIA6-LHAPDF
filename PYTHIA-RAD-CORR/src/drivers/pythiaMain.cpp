//--------------------------------------------------------------------
//  pythiaMain.cpp
//  Author: Kolja Kauder, Thomas Ullrich
//
//  Generate EIC events with BNL-customized Pythia6
//  using the TPythia interface in ROOT.
//
//  --------> p       e <--------- 
//--------------------------------------------------------------------
#include "TPythia6.h"
#include "TParticle.h"
#include "TClonesArray.h"
#include "TTree.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"

#include <iostream>

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

// Forward declarations
// --------------------
bool initPythia(TPythia6* pythia, double mom_e, double mom_p, double minQ2, double maxQ2);


// Example event info written to tuple
// -----------------------------------
struct EventHeader{
  double s;
  double Q2;
  double x;
  double y;
  double W2;
  int isub;
};
EventHeader myEventHeader;

// Main Program
// ------------
int main(int argc, char* argv[])
{
    
  // Deal with arguments
  // -------------------
  if (argc != 7) {
    cout << "Usage: " << argv[0] << "   nevents ebeam  pbeam minQ2 maxQ2 rootfile" << endl;
    return 2;
  }

  unsigned int nevents = atol(argv[1]);
  double eBeam = atof(argv[2]);
  double pBeam = atof(argv[3]);
  double minQ2 = atof(argv[4]);
  double maxQ2 = atof(argv[5]);
  char* rootfile = argv[6];
  
  // Print settings
  // --------------
  cout << "========== " << argv[0] << " ==========" << endl;
  cout << "\tEvents to generate: " << nevents << endl;
  cout << "\te beam: " << eBeam << " GeV" << endl;
  cout << "\tp beam: " << pBeam << " GeV" << endl;
  cout << "\tmin Q2: " << minQ2 << " GeV2" << endl;
  cout << "\tmaxQ2: " << maxQ2 << " GeV2" << endl;
  cout << "\troot file: " << rootfile << endl;

  // Particle cuts in lab frame
  double MaxEtaLab = 10;  
  const double minPt = 0.2;

  // Open output file before booking objects
  // ---------------------------------------
  TFile *hfile = new TFile(rootfile,"RECREATE");

  // Setup ROOT tree
  // ---------------
  TTree* tree = new TTree("tree","pythia6");
  tree->Branch("event", &myEventHeader.s,
  	      "s/D:Q2/D:x/D:y/D:W2/D:isub/I");
  tree->Branch("event", &myEventHeader);
  
  
  // A histogram
  // -----------
  double histo_maxEta = 4.5;
  TH2F *electron2D=0;
  electron2D = new TH2F("electron2D", "Scattered Electron", 50, -histo_maxEta, histo_maxEta, 50, -TMath::Pi(), TMath::Pi());
    
  // Set up pythia6 object
  // ---------------------
  TPythia6 *pythia = new TPythia6;

  // Global common block data that needs to be set without
  // using the TPythia6 object
  
  // lepton beam type;
  int ltype = 11;

  // parameters for PYINIT call (beam and target particle energy).
  common_mcset.pbeam = pBeam;
  common_mcset.ebeam = eBeam;

  // min/max x of radgen lookup table
  common_mcset.mcSet_XMin = 1e-07;
  common_mcset.mcSet_XMax = 0.99;
  
  // min/max y of generation range
  common_mcset.mcSet_YMin = 0.05;
  common_mcset.mcSet_YMax = 0.95;

  // min/max Q2 of generation range 
  common_mcset.mcSet_Q2Min  = 1e-09;
  common_mcset.mcSet_Q2Max = 20000;

  // information for cross section used in radgen
  strncpy( common_mcset.genSet_FStruct, "F2PY", 4 ); // allocated exactly 4 bytes, so don't copy null terminator
  strncpy( common_mcset.genSet_R, "1998", 4 );  

  // parameters of radcorr: do radcorr (1), generate look-up table (2)
  common_mcset.qedrad = 0;

  // parameters for PYTHIA-Model = which generation is done     
  common_mcset.iModel = 0;

  // target type mass and charge
  common_mcset.mcSet_TarA = 1;
  common_mcset.mcSet_TarZ = 0;

  common_pynucl.INUMOD = 1;
  common_pynucl.CHANUM = 1;

  // nuclear pdf correction order
  common_pynucl.ORDER = 201;
  
  // beam masses
  common_mcset.massp=pythia->Pymass(2212);  // p, GeV
  common_mcset.masse=pythia->Pymass(ltype); // e, GeV

  // derived quantities
  double pbeamE=sqrt(common_mcset.pbeam*common_mcset.pbeam+common_mcset.massp*common_mcset.massp);
  double pbeta=common_mcset.pbeam/pbeamE;
  double pgamma=pbeamE/common_mcset.massp;

  double ebeamE=sqrt(common_mcset.ebeam*common_mcset.ebeam+common_mcset.masse*common_mcset.masse);
  double ebeamEnucl=pgamma*ebeamE-pgamma*pbeta*(-common_mcset.ebeam);
  double epznucl=-pgamma*pbeta*(ebeamE)+pgamma*(-common_mcset.ebeam);
  common_mcset.mcSet_EneBeam=ebeamEnucl;
  common_mcevnt.ebeamEnucl = ebeamEnucl; // 
 
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

  // CKINS, MSTPs, etc.
  initPythia(pythia, eBeam, pBeam, minQ2, maxQ2);

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

  // output container
  TClonesArray* particles = 0;  // holds all particles generated in an event 
    

  for (unsigned int ievent=0; ievent<nevents; ievent++) {    
    pythia->GenerateEvent();

    // RadCorr could render this event invalid, requiring a do-over
    if ( pythia->GetMSTI(61) == 1 ) {
      cout << " go back to PYEVNT call" << endl;
      continue;
    }

    if (ievent<30) pythia->Pylist(1); // print first X events
    // pythia->Pylist(1);

    int numParticles = pythia->GetNumberOfParticles();
    delete particles;
    particles = new TClonesArray("TParticle", numParticles);
    int obtainedParticles = pythia->ImportParticles(particles, "All");
    if (numParticles != obtainedParticles) {
      cout << "Warning: could not obtain all particles generated." << endl;
    }  

    // Start filling header
    // --------------------    
    auto me = common_mcset.masse;
    auto mp = common_mcset.massp;
    double trueS = pythia->GetVINT(302);
    double trueQ2 = pythia->GetVINT(307);
    double trueY = pythia->GetVINT(309);
    double trueX = trueQ2/(trueY*(trueS - me*me - mp*mp));
    double trueW2 = mp*mp + trueQ2*(1-trueX)/trueX;
    int isub = pythia->GetMINT(1);
    myEventHeader.Q2 = trueQ2;
    myEventHeader.x = trueX;
    myEventHeader.y = trueY;
    myEventHeader.s = trueS;
    myEventHeader.W2 = trueW2;
    myEventHeader.isub = isub;    
    
    //  Fill ROOT tree
    // ---------------    
    tree->Fill();
  
    if (!((ievent+1)%1000)) {
      cout << ievent+1 << " events processed, cs = " << pythia->GetPARI(1) << " mb" << endl;
    }
  
    // previousCrossSection = pythia->GetPARI(1);
  }
  
  //
  //   Finishing up
  //
  
  pythia->Pystat(1);
  pythia->Pystat(4);
  cout << "The charm mass used is: "<< pythia->GetPMAS(4,1) << endl;
  cout << "\nAll events processed" << endl;
  cout << "Writing File" << endl;
  hfile->Write();

 return 0;
}

bool initPythia(TPythia6* pythia, double mom_e, double mom_p, double minQ2, double maxQ2)
{
  //  Here we set all Pythia parameters and at the very
  //  end initialize it (aka PYINIT). I added as much explanation
  //  as reasonable here. For more info see the EIC Wiki page
  //  and of course the PYTHIA6 manual.
  //
  if (!pythia) return false;
  pythia->SetMSEL(2);   // standard

  // pythia->SetMSEL(0);    // select specific processed with SetMSUB(proc,1)
  // // pythia->SetMSUB(99,1);
  // pythia->SetMSUB(135,1); // T
  // pythia->SetMSUB(136,1); // L
  // pythia->SetMSUB(131,1); // T
  // pythia->SetMSUB(132,1); // L

    
  //
  //  General Switches and Parameters: MSTPs
  //
  pythia->SetMSTP(14, 30);  // default
  pythia->SetMSTP(15, 0);   // default
  pythia->SetMSTP(16, 1);   // default
  pythia->SetMSTP(17, 4);   // default
  pythia->SetMSTP(18, 3);   // default
  pythia->SetMSTP(19, 1);   // (default = 4) choice of partonic cross section in the DIS process 99.
  pythia->SetMSTP(20, 0);   // (default = 3)
  pythia->SetMSTP(32, 8);   // default
  pythia->SetMSTP(38, 4);   // (default = 5)

  // for 5: https://lhapdf.hepforge.org/lhapdf5/manual#tth_sEcA
  // for 6: https://lhapdf.hepforge.org/pdfsets

  // Download scripts are rather broken. Do it by hand, e.g. with
  // export LHAPDF5_DATA=/Users/kkauder/software/lhapdf5/share/lhapdf/PDFsets/
  // wget -O- https://lhapdf.hepforge.org/downloads?f=pdfsets/5.9.1/cteq6AB.LHgrid > $LHAPDF5_DATA/cteq6AB.LHgrid
  // which makes 10250 work

  // 7 (internal) = CTEQ 5L (leading order).
  // LHAPDF6: 13200=CTEQ14lo, 11000=CT10nlo
  // LHAPDF5: 10000=cteq6
  // Both: 10550=cteq66 (they do give different results though)

  // You may have to adjust MSEL. pythia->SetMSUB(99,1) seems to work
  
  // PDFLIB: build the number out of https://www.hep.phy.cam.ac.uk/theory/webber/MCatNLO/pdflib_doc.ps.gz
  // But currently, this isn't supported (linking is messed up)
  
  // pythia->SetMSTP(51, 10550 );
  // pythia->SetMSTP(52, 2);   // 2=use pdflib (lhapdf), 1=internal pythia
  pythia->SetMSTP(51, 7);
  pythia->SetMSTP(52, 1);   // 2=use pdflib (lhapdf), 1=internal pythia
  
  pythia->SetMSTP(53, 3);   // default
  pythia->SetMSTP(54, 1);   // default
  pythia->SetMSTP(55, 5);   // default
  pythia->SetMSTP(56, 1);   // default
  pythia->SetMSTP(57, 1);   // default
  pythia->SetMSTP(58, 5);   // (default = min(5, 2×MSTP(1))) maximum number of quark flavours used in pdf
  pythia->SetMSTP(59, 1);   // default
  pythia->SetMSTP(60, 7);   // default
  pythia->SetMSTP(61, 2);   // default
  pythia->SetMSTP(71, 1);   // default
  pythia->SetMSTP(81, 0);   // (default=1) master switch for multiple interactions 0=off, 1=on
  pythia->SetMSTP(82, 1);   // (default=4) structure of multiple interactions
  pythia->SetMSTP(91, 1);   // default 1; 0= no primordial kT
  pythia->SetMSTP(92, 3);   // default
  pythia->SetMSTP(93, 1);   // default 1; 0= no primordial kT
  pythia->SetMSTP(101, 3);  // default
  pythia->SetMSTP(102, 1);  // default
  pythia->SetMSTP(111, 1);  // master switch for fragmentation and decay (0=off)
  pythia->SetMSTP(121, 0);  // default
    
  //
  //  PARPs
  //
  pythia->SetPARP(13, 1);        // default
  pythia->SetPARP(18, 0.40);     // default
  pythia->SetPARP(81, 1.9);      // default
  pythia->SetPARP(89, 1800);     // default
  pythia->SetPARP(90, 0.16);     // default
  // pythia->SetPARP(91, 2);      // (default=2) width of Gaussian primordial kT distribution inside hadron
  pythia->SetPARP(91, 0.4);      // 0.4 from comparison input card
  pythia->SetPARP(93, 5.);       // default = 5   // TMP
  // pythia->SetPARP(99, 1);      // (default=1) width parameter of primordial kT distribution inside photon
  pythia->SetPARP(99, 0.4);      // 0.4 from comparison input card
  pythia->SetPARP(100, 5);       // default
  pythia->SetPARP(102, 0.28);    // default
  pythia->SetPARP(103, 1.0);     // default
  pythia->SetPARP(104, 0.8);     // default
  pythia->SetPARP(111, 2.);      // default
  pythia->SetPARP(161, 3.00);    // (default=2.2)  coupling f_V^2/4pi of photon to rho
  pythia->SetPARP(162, 24.6);    // (default=23.6)  coupling of photon to omega
  pythia->SetPARP(163, 18.8);    // (default=18.4)  coupling of photon to phi
  pythia->SetPARP(164, 11.5);    // (default=11.5)  coupling of photon to J/psi
  pythia->SetPARP(165, 0.47679); // (default=0.5)
  pythia->SetPARP(166, 0.67597); // no record in Pythia documentation
    
  //
  //  Now come all the switches for Jetset
  //
  pythia->SetPARJ(1, 0.100); // default
  pythia->SetPARJ(2, 0.300); // default
  // pythia->SetPARJ(11, 0.5);  // default 0.5
  pythia->SetPARJ(11, 0.25);  // 0.25 in card
  // pythia->SetPARJ(12, 0.6);  // default
  pythia->SetPARJ(12, 0.25);  // 0.25 in card
  pythia->SetPARJ(21, 0.40); // default = 0.36
  pythia->SetPARJ(32, 1.0);  // default
  pythia->SetPARJ(33, 0.80); // default
  pythia->SetPARJ(41, 0.30); // default
  pythia->SetPARJ(42, 0.58); // default
  pythia->SetPARJ(45, 0.5);  // default
  pythia->SetMSTJ(1, 1);     // default - Lund fragmentation
  pythia->SetMSTJ(12, 1);    // default = 2
  pythia->SetMSTJ(45, 5);    // default
  pythia->SetMSTU(16, 2);    // default=1 but is 2 on EIC std settings
  pythia->SetMSTU(112, 5);   // default
  pythia->SetMSTU(113, 5);   // minimum number of flavours that may be assumed in alpha_s (default=3)
  pythia->SetMSTU(114, 5);   // maximum number of flavours (default=5)
    
  //
  //  All the CKINs for Pythia
  //
  pythia->SetCKIN(1, 1.);    // (default=2), min mass=sqrt(shat)
  pythia->SetCKIN(2, -1.);   // default, max mass, -1 means no upper limit
  pythia->SetCKIN(3, 0.);    // default, range of allowed pthat values for hard 2->2
  pythia->SetCKIN(4, -1.);   // default
  pythia->SetCKIN(5, 1.00);  // default
  pythia->SetCKIN(6, 1.00);  // default
  pythia->SetCKIN(7, -10.);  // default
  pythia->SetCKIN(8, 10.);   // default
  pythia->SetCKIN(9, -40.);  // default
  pythia->SetCKIN(10, 40.);  // default
  pythia->SetCKIN(11, -40.); // default
  pythia->SetCKIN(12, 40.);  // default
  pythia->SetCKIN(13, -40.); // default
  pythia->SetCKIN(14, 40.);  // default
  pythia->SetCKIN(15, -40.); // default
  pythia->SetCKIN(16, 40.);  // default
  pythia->SetCKIN(17, -1.);  // default
  pythia->SetCKIN(18, 1.);   // default
  pythia->SetCKIN(19, -1.);  // default
  pythia->SetCKIN(20, 1.);   // default
  pythia->SetCKIN(21, 0.);   // default, range of allowed x1 values for the parton on side 1
  // that enters the hard interaction.
  pythia->SetCKIN(22, 1.);   // default,
  pythia->SetCKIN(23, 0.);   // default, range of allowed x2 values for the parton on side 2
  // that enters the hard interaction.
  pythia->SetCKIN(24, 1.);   // default
  pythia->SetCKIN(25, -1.);  // default, range of allowed xF = x1-x2
  pythia->SetCKIN(26, 1.);   // default
    
  pythia->SetCKIN(27, -1.);  // default
  pythia->SetCKIN(28, 1.);   // default
  pythia->SetCKIN(31, 2.);   // default
  pythia->SetCKIN(32, -1.);  // default
  pythia->SetCKIN(35, 0.);   // default
  pythia->SetCKIN(36, -1);   // default
  pythia->SetCKIN(37, 0.);   // default
  pythia->SetCKIN(38, -1.);  // default
  pythia->SetCKIN(39, 4.);   // default
  pythia->SetCKIN(40, -1.);  // default
  // pythia->SetCKIN(65, minQ2);  // Min for Q^2 (default=0)
  // pythia->SetCKIN(66, maxQ2);  // Max for Q^2 (default=-1 ie no limit)
  // pythia->SetCKIN(65, 0);  // Min for Q^2 (default=0)
  // pythia->SetCKIN(66, -1);  // Max for Q^2 (default=-1 ie no limit)
  pythia->SetCKIN(65, 1e-9);  // Min for Q^2 (default=0)
  pythia->SetCKIN(66, -1);  // Max for Q^2 (default=-1 ie no limit)
  pythia->SetCKIN(67, 0.);   // default  ??? related to above
  pythia->SetCKIN(68, -1.);  // default
  pythia->SetCKIN(77, 2);         // Min W (default=2)
  pythia->SetCKIN(78, -1.);       // Max W (default=-1)
    
  //
  //   Set beam momenta for call to Initialize()
  //
  pythia->SetP(1, 1, 0.); // e
  pythia->SetP(1, 2, 0.);
  pythia->SetP(1, 3, -mom_e);
  pythia->SetP(2, 1, 0.); // p
  pythia->SetP(2, 2, 0.);
  pythia->SetP(2, 3, mom_p);
    
  //
  //  Seed for random generator
  //
  // int seed = 42;
  std::random_device rd;
  int seed = rd();
  pythia->SetMRPY(1, seed);
    
  //
  //  Finally initialize Pythia
  //
  // for fixed target, use frame = "FIXT";
  string frame = "3MOM";
  if ( common_mcset.pbeam <= 1e-4 ) {
    frame = "FIXT";
    common_mcset.pbeam=0;
  }
  double WIN = common_mcset.ebeam; // for 3MOM, this is a dummy, for FIXT, it needs to be the beam energy

  pythia->Initialize(frame.c_str(), "gamma/e-","p+", WIN);
  
  return true;
}

