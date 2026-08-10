#include <eicsmear/functions.h>

using namespace erhic;

/*
void genTree(const TString x, const TString y)
{
  cout << "In genTree" << endl;
  gSystem->Load("libeicsmear");
  gSystem->Load("libeicsmeardetectors");

  cout << gSystem->DynamicPathName("libeicsmear") << endl;

  BuildTree(x,y);
}
*/


void genHepMC(const std::string x, const std::string y) // TString
{
  gSystem->Load("libeicsmear");
  gSystem->Load("libeicsmeardetectors");
  //BuildTree(x,y);
  TreeToHepMC(x,y,-1,0);
}

