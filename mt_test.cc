#include "VsdBase.h"
#include "TFile.h"

#ifdef STANDALONE_WRITE_TEST

#include "MyVsdTree.h"

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

#include "MyVsdTree.h"

#include "TRint.h"
#include "TROOT.h"

int main(int argc, char *argv[]) {
  TFile *f = TFile::Open("vsd.root", "READ");

  TTree *tree = (TTree*) f->Get("VSD");

  MyVsdTree vsdt(tree);

  printf("Derived class, supported vector (size = %lu )\n", vsdt.m_supported_vector.size());
  // Do not call in reading! This is writing only.
  // vsdt.append_collections({"cands", "jets"});

  printf("Opened VsdTree N_events=%lld, current event=%lld\n", vsdt.n_events(), vsdt.current_event());
  vsdt.goto_event(0);

  if (vsdt.cands().empty())
  {
    printf("VSDTree has cands() empty !\n");
  }

  VsdCollection vsdc;
  std::string cname = "cands";
  auto bbi = vsdt.m_active_map.find(cname);
  if (bbi !=  vsdt.m_active_map.end())
  {
      bbi->second->fill_element_ptrs(vsdc.m_list);
      printf("Filled element pointers for 'cands', size=%d\n", (int) vsdc.m_list.size());
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

#ifdef USER_VSD_READ_TEST

#include "VsdTree.h"

#include "TPRegexp.h"
#include "TRint.h"
#include "TROOT.h"
#include "TStopwatch.h"

int main(int argc, char *argv[]) {
  TFile *f = TFile::Open("UserVsd.root", "READ");

  TTree *tree = (TTree*) f->Get("VSD");

  printf("\n--- Starting branch processing ...\n");

  TString code =
"#include \"VsdTree.h\"\n\n"
"#include \"VsdRegisterBranch.h\"\n\n"
"VSD_BEGIN_CLASS(UserVsdTree)\n\n";

  TPMERegexp re("^(?:std::)?vector<(Vsd[\\w\\d]+)>$");
  int i = 0;
  TIter bi(tree->GetListOfBranches());
  while (TBranch *br = (TBranch*) bi()) {
    printf("  %2d. name='%s' class='%s' ptr=%p mother_ptr=%p\n",
           ++i, br->GetName(), br->GetClassName(), br, br->GetMother());
    if (re.Match(br->GetClassName())) {
      printf("      matched type '%s' ... whole = '%s'\n",
             re[1].Data(), re[0].Data());

      code += TString::Format("VSD_REGISTER_BRANCH(%s, %s)\n",
                              re[1].Data(), br->GetName());
    }
  }

  VsdTree *vsd_tree = nullptr;

  code += TString::Format(
"\nVSD_END_CLASS\n\n"
"void tmp_user_vsd() {\n"
"  *((VsdTree**) %p) = new UserVsdTree((TTree*) %p);\n"
"}\n",
  &vsd_tree, tree);

  printf("\n--- Finished branch processing, here's the code I'll process now:\n\n%s\n",
         code.Data());

  FILE *fp = fopen("tmp_user_vsd.C", "w");
  // XXXX check status
  code.Puts(fp);
  fclose(fp);

  TStopwatch sw;
  sw.Start();
  gROOT->ProcessLine(".x tmp_user_vsd.C+");
  sw.Stop();
  // XXXX check status, can also just check vsd_tree == nullptr

  printf("\n--- Finished macro processing, local variable vsd_tree = %p\n", vsd_tree);
  sw.Print();

  // called automatially from constructor
  // vsd_tree->setAdressToSupportedBranches();

  printf("\n--- Active collections:\n");
  for (auto &&[name, brptr] : vsd_tree->m_active_map) {
    printf("  '%s' -> '%s'\n",
      name.c_str(), brptr->m_collection_type.c_str());
  }

  return 0;
}

#endif

#define USER_COLPROXY_READ_TEST
#ifdef USER_COLPROXY_READ_TEST

#include "VsdTree.h"

#include "TClass.h"
#include "TVirtualCollectionProxy.h"
#include "TBranchElement.h"

#include "TPRegexp.h"
// #include "TRint.h"
// #include "TROOT.h"
// #include "TStopwatch.h"

int main(int argc, char *argv[]) {
  TFile *f = TFile::Open("UserVsd.root", "READ");

  TTree *tree = (TTree*) f->Get("VSD");

  printf("\n--- Starting branch processing ...\n");

  struct ColBranchInfo {
    TBranchElement *m_branch;
    TClass *m_cclass, *m_eclass;
    TVirtualCollectionProxy *m_proxy;
    char *m_collection;
    std::string m_name;

    ColBranchInfo() = default;
  };

  std::map<std::string, ColBranchInfo> cmap;

  TPMERegexp re("^(?:std::)?vector<(Vsd[\\w\\d]+)>$");
  int i = 0;
  TIter bi(tree->GetListOfBranches());
  while (TBranch *br = (TBranch*) bi())
  {
    TBranchElement *bre = dynamic_cast<TBranchElement*>(br);
    printf("  %2d. name='%s' class='%s' ptr=%p mother_ptr=%p branch_class='%s' branch_element_ptr=%p\n",
           ++i, br->GetName(), br->GetClassName(), br, br->GetMother(), br->ClassName(), bre);

    if (bre && re.Match(br->GetClassName()))
    {
      printf("      matched type '%s' ... whole = '%s'\n",
             re[1].Data(), re[0].Data());

      bre->GetEntry(0);
      printf(" get entry(0) -> address=%p object=%p\n",
          bre->GetAddress(), bre->GetObject());

      TClass *cc = TClass::GetClass(br->GetClassName());
      TVirtualCollectionProxy *cp = cc->GetCollectionProxy()->Generate();
      cp->PushProxy(bre->GetObject());

      TClass *ec = TClass::GetClass(re[1].Data());
      cmap.insert({br->GetName(), {bre, cc, ec, cp, bre->GetObject(), br->GetName()}});

      printf("  post get entry 0 %s %u\n", br->GetName(), cp->Size());
    }
  }

  // Crashed before letting ROOT setup the branches.
  // tree->GetEntry(0);

  for (int ev = 0; ev < 10; ++ev) {
    printf("Event %d\n", ev);
    tree->GetEntry(ev);
    for (auto &&[name, cbi] : cmap) {
      printf("  Trying to read %s\n", name.c_str());
      //cbi.m_proxy->PushProxy(cbi.m_collection);
      printf("    pre get branch entry size = %u\n", cbi.m_proxy->Size());
      cbi.m_branch->GetEntry(ev);
      printf("    size = %u\n", cbi.m_proxy->Size());
      // cbi.m_proxy->PopProxy();
    }
  }

  return 0;
}

#endif
