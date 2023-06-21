#include "TFile.h"
#include "TTree.h"

void makeClass()
{
    std::cout << "Make class\n";
    auto file = TFile::Open("nano-CMSSW_11_0_0-RelValZTT-mcRun.root");
    auto  tree = (TTree *)file->Get("Events");
    tree->MakeClass("VSDReader");
    exit(0);
}
