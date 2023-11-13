#include "VsdTree.h"
#include "TTree.h"
#include "TFile.h"

//==============================================================

VsdBranchBase::VsdBranchBase(VsdTree *vsd_tree, std::string_view name, std::string_view type, std::string_view ctype) :
  m_name(name), m_type(type), m_collection_type(ctype)
{
  printf("VsdBranchBase registered: %s, %s, %s\n", m_name.c_str(), m_type.c_str(), m_collection_type.c_str());
  vsd_tree->register_supported_collection(m_name, this);  
}

//==============================================================

// === Writing interface / workflow ===

VsdTree::VsdTree(const char *name, const char * title) {
  m_tree = new TTree(name, title);
}

void VsdTree::append_collections(std::vector<std::string> &&col_names) {
  for (auto &cn : col_names) {
    if (has_collection(cn)) throw std::runtime_error("collection name already registered - " + cn);

    auto si = m_supported_map.find(cn);
    if (si == m_supported_map.end()) throw std::runtime_error("collection name not supported - " + cn);

    m_active_vector.push_back(si->second);
    m_active_map.insert(std::make_pair(cn, si->second));
  }
}

void VsdTree::prepare_for_writing() {
  // register branches in order they were specified
  for (auto cbptr : m_active_vector) {
    cbptr->register_branch(m_tree);
  }
  m_current_event = 0;
}

void VsdTree::fill_all_braches(bool clear_collections) {
  m_tree->Fill();
  if (clear_collections) {
    for (auto cbptr : m_active_vector) {
      cbptr->clear_collection();
    }
  }
  ++m_current_event;
}

void VsdTree::finalize_writing() {
  m_tree->Write();
}

// === Reading interace ===

VsdTree::VsdTree(TTree *tree) : m_tree(tree) {
  for (auto cbptr : m_supported_vector) {
    const char *bname = cbptr->m_name.c_str();
    TBranch *br = (TBranch*) m_tree->GetListOfBranches()->FindObject(bname);
    if (br != nullptr) {
      printf("  Branch %s found ... setting up.\n", bname);
      cbptr->set_branch_address(br);
    } else {
      printf("  Branch %s not found.\n", bname);
    }
  }
  goto_event(0);
}

long long VsdTree::n_events() const {
  return m_tree->GetEntries();
}

long long VsdTree::current_event() const {
  return m_current_event;
}

void VsdTree::goto_event(long long ev) {
  m_tree->GetEntry(ev);
  m_current_event = ev;
}

// === Common functions ===

void VsdTree::register_supported_collection(const std::string& cn, VsdBranchBase *cbptr) {
  m_supported_vector.push_back(cbptr);
  m_supported_map.insert(std::make_pair(cn, cbptr));
}

bool VsdTree::supports_collection(const std::string &cn) const {
  auto i = m_supported_map.find(cn);
  return i != m_supported_map.end();
}

bool VsdTree::has_collection(const std::string&cn) const {
  auto i = m_active_map.find(cn);
  return i != m_active_map.end();
}

//==============================================================

#ifdef STANDALONE_WRITE_TEST

#include "TMath.h"
#include "TRandom.h"

// To load:
// root.exe -e 'gSystem->Load("libVsdDict.so")' vsd.root

int main() {
  TFile *f = TFile::Open("vsd.root", "RECREATE");

  VsdTree vsdt("VSD", "VSD TTree test");
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

  VsdTree vsdt(tree);

  printf("Opened VsdTree N_events=%lld, current event=%lld\n", vsdt.n_events(), vsdt.current_event());


  TRint rint("mt_read", &argc, argv);
  rint.Run(true);

  f->Close();

  return 0;
}

#endif
