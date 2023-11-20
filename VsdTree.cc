#include "VsdTree.h"
#include "TTree.h"
#include "TObjString.h"

#include <fstream>

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
  TObjString vsd_tree_code;
  std::string fname = m_vsd_tree_class + ".class";
  printf("Opening .class file '%s'\n", fname.c_str());
  if (std::ifstream ifs { fname }) {
		vsd_tree_code.String().ReadFile(ifs);
    vsd_tree_code.Write("vsd_tree_code");
  } else {
    throw std::runtime_error("Derived VsdTree .class file not found");
  } 

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
  printf("VsdTree Base class, supported vector (size = %lu )\n", m_supported_vector.size());
}

void VsdTree::setAdressToSupportedBranches()
{
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

VsdBranchBase* VsdTree::get_collection(const std::string&cn) const {
  auto i = m_active_map.find(cn);
  return i != m_active_map.end() ? i->second : nullptr;
}
