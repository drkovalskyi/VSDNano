#include "TFile.h"
#include "TTree.h"

void makeClass()
{
    auto file = TFile::Open("nano.root");
    auto  tree = (TTree *)file->Get("Events");
    tree->MakeClass("VSDReader");
}
