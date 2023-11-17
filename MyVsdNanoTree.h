#ifndef MyVsdNanoTree_h

#if ! defined(FOR_VSD_CODE)
#include "VsdTree.h"
#endif
#include "VsdRegisterBranch.h"

VSD_BEGIN_CLASS(MyVsdNanoTree)

  VSD_REGISTER_BRANCH(VsdVertex, primvs);
  VSD_REGISTER_BRANCH(VsdCandidate, cands);
  VSD_REGISTER_BRANCH(VsdJet, jets);
  VSD_REGISTER_BRANCH(VsdMET, caloMETs);
  VSD_REGISTER_BRANCH(VsdMET, chsMETs);
  VSD_REGISTER_BRANCH(VsdMuon, muons);
  VSD_REGISTER_BRANCH(VsdEventInfo, infos);

VSD_END_CLASS
#endif
