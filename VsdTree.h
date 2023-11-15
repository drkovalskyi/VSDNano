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

  virtual void register_branch(TTree *tree) = 0;
  virtual void clear_collection() = 0;
  virtual void set_branch_address(TBranch *branch) = 0;

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

  void register_branch(TTree *tree) override { m_branch = tree->Branch(m_name.c_str(), &m_collection); }
  void clear_collection() override { m_collection->clear(); }
  void set_branch_address(TBranch *branch) override { m_branch = branch; branch->SetAddress(&m_collection); }

  std::vector<VSDCLS> *m_collection = nullptr;
};

#define REGISTER_BRANCH(_type_, _name_) \
protected: \
  VsdBranch<_type_> m_ ## _name_ { this, #_name_, #_type_, "std::vector<" #_type_ ">" }; \
public: \
  bool has_ ## _name_() const { return m_ ## _name_.m_collection != nullptr; } \
  std::vector<_type_>& _name_() const { return *m_ ## _name_.m_collection; } \
  _type_& _name_(int i) const { return _name_()[i]; } \
  int _name_ ## _size() const { return (int) _name_().size(); }

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

protected:
  std::vector<VsdBranchBase*> m_supported_vector;
  std::map<std::string, VsdBranchBase*> m_supported_map;

  std::vector<VsdBranchBase*> m_active_vector;
  std::map<std::string, VsdBranchBase*> m_active_map;

  TTree *m_tree = nullptr;
  long long m_current_event = -1;

  // Register supported collections, this has to be AFTER the above vector/map declarations.
  REGISTER_BRANCH(VsdVertex, primvs);
  REGISTER_BRANCH(VsdCandidate, cands);
  REGISTER_BRANCH(VsdJet, jets);
  REGISTER_BRANCH(VsdMET, cmets);
  REGISTER_BRANCH(VsdEventInfo, infos);
};

#undef REGISTER_BRANCH

#endif
