#ifndef MyVsdTree_h

#if ! defined(FOR_VSD_CODE)
#include "VsdTree.h"
#endif
#include "VsdRegisterBranch.h"

VSD_BEGIN_CLASS(MyVsdTree)

VSD_REGISTER_BRANCH(VsdVertex, primvs)
VSD_REGISTER_BRANCH(VsdCandidate, cands)
VSD_REGISTER_BRANCH(VsdJet, jets)
VSD_REGISTER_BRANCH(VsdMET, cmets)
VSD_REGISTER_BRANCH(VsdMET, jebomets)
VSD_REGISTER_BRANCH(VsdEventInfo, infos)

VSD_END_CLASS

/*
#include "VsdTree.h"
TFile *_file0 = TFile::Open("vsd.root")
gROOT->ProcessLine(".L VsdTree.cc")
TObjString *cc = (TObjString*) gFile->Get("vsd_tree_code")
cc->Print("")
gROOT->ProcessLine(cc->GetString())
.ls
TTree *t = (TTree*) gFile->Get("VSD")
MyVsdTree mt = MyVsdTree(t)

// In plain service we work through base-class interface
VsdTree *vsd_tree = new MyVsdTree(t)

*/

#endif
