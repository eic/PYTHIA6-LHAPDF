#include <eicsmear/functions.h>

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


void genTree(const std::string x, const std::string y) // TString
{
  gSystem->Load("libeicsmear");
  gSystem->Load("libeicsmeardetectors");
  BuildTree(x,y);
}

