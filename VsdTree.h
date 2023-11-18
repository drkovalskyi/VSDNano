#ifndef VsdTree_h
#define VsdTree_h

#include "VsdBase.h"
#include <functional>
#include "string"
#include "set"
#include "vector"

class TTree;
class VsdTree;

// -------------------------------------------------------------

class VsdBranchBase {
public:
  VsdBranchBase(VsdTree *vsd_tree, std::string_view name, std::string_view type, std::string_view ctype);
  virtual ~VsdBranchBase() {}

  virtual void register_branch(TTree *tree) = 0;
  virtual void clear_collection() = 0;
  virtual void set_branch_address(TBranch *branch) = 0;

  virtual void fill_element_ptrs(std::vector<VsdBase*> &vec) = 0;

  std::string m_name;
  std::string m_type;
  std::string m_collection_type;
  TBranch *m_branch;
};

// -------------------------------------------------------------

// Could have separate sub-classes for maps or other types, now all are vectors.

template<class VSDCLS>
class VsdBranch : public VsdBranchBase {
public:
  VsdBranch(VsdTree *vsd_tree, std::string_view name, std::string_view type, std::string_view ctype) :
    VsdBranchBase(vsd_tree, name, type, ctype)
  {}
  virtual ~VsdBranch() {}

  void register_branch(TTree *tree) override { m_branch = tree->Branch(m_name.c_str(), &m_collection); }
  void clear_collection() override { m_collection.clear(); }
  void set_branch_address(TBranch *branch) override { m_branch = branch; branch->SetAddress(&m_collection); }

  void fill_element_ptrs(std::vector<VsdBase*> &vec) override {
    vec.reserve(vec.size() + m_collection.size());
    for (auto &e : m_collection) vec.push_back(&e);
  }

  std::vector<VSDCLS> m_collection;
};

// -------------------------------------------------------------

class VsdTree {
  friend class VsdBranchBase;
  void register_supported_collection(const std::string& cn, VsdBranchBase *cbptr);

public:
  // Writing constructor & workflow
  VsdTree(const char *name, const char * title);

  void append_collections(std::vector<std::string> &&col_names);

  void prepare_for_writing();
  void fill_all_braches(bool clear_collections);
  void finalize_writing();

  // Reading constructor & workflow
  VsdTree(TTree *tree);

  long long n_events() const;
  long long current_event() const;
  void goto_event(long long ev);

  // Common functions
  bool supports_collection(const std::string &cn) const;
  bool has_collection(const std::string &cn) const;
  VsdBranchBase* get_collection(const std::string &cn) const;

// protected:
  std::vector<VsdBranchBase*> m_supported_vector;

  std::map<std::string, VsdBranchBase*> m_supported_map;

  std::vector<VsdBranchBase*> m_active_vector;
  std::map<std::string, VsdBranchBase*> m_active_map;

  std::string m_vsd_tree_class;

  TTree *m_tree = nullptr;
  long long m_current_event = -1;
};

#endif
