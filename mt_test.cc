#include "MyVsdTree.h"
#include "VsdBase.h"
#include "TFile.h"

#ifdef STANDALONE_WRITE_TEST

#include "TMath.h"
#include "TRandom.h"

// To load:
// root.exe -e 'gSystem->Load("libVsdDict.so")' vsd.root

int main() {
  TFile *f = TFile::Open("vsd.root", "RECREATE");

  MyVsdTree vsdt("VSD", "VSD TTree test");
  vsdt.append_collections({"cands", "jets"});
  vsdt.prepare_for_writing();

  for (int i = 0; i < 100; ++i) {
    int n_cands = 20 + gRandom->Integer(80);
    for (int j = 0; j < n_cands; ++j) {
      vsdt.cands().emplace_back(gRandom->Uniform(0.1, 20),
                                gRandom->Uniform(-2.5, 2.5),
                                gRandom->Uniform(-TMath::Pi(), TMath::Pi()),
                                (gRandom->Rndm() > 0.5 ? 1 : -1));
    }
    int n_jets = 6 + gRandom->Integer(24);
    for (int j = 0; j < n_jets; ++j) {
      vsdt.jets().emplace_back(gRandom->Uniform(0.1, 20),
                               gRandom->Uniform(-2.5, 2.5),
                               gRandom->Uniform(-TMath::Pi(), TMath::Pi()),
                              (gRandom->Rndm() > 0.5 ? 1 : -1),
                               gRandom->Uniform(0.1, 0.9),
                               gRandom->Uniform(0.05, 1));
    }

    vsdt.fill_all_braches(true);
  }

  vsdt.finalize_writing();

  f->Close();
  delete f;

  return 0;
}

#endif

#ifdef STANDALONE_READ_TEST

#include "TRint.h"
#include "TROOT.h"

int main(int argc, char *argv[]) {
  TFile *f = TFile::Open("vsd.root", "READ");

  TTree *tree = (TTree*) f->Get("VSD");

  MyVsdTree vsdt(tree);
  vsdt.append_collections({"cands", "jets"});

  printf("Opened VsdTree N_events=%lld, current event=%lld\n", vsdt.n_events(), vsdt.current_event());
  vsdt.goto_event(0);

  VsdCollection vsdc;
  std::string cname = "cands";
  auto bbi = vsdt.m_supported_map.find(cname);
  if (bbi !=  vsdt.m_supported_map.end())
  {
      bbi->second->fill_element_ptrs(vsdc.m_list);
  }
  else
  {
      printf("can't find branch %s\n", cname.c_str());
  }


  TRint rint("mt_read", &argc, argv);
  rint.Run(true);

  f->Close();

  return 0;
}

#endif
