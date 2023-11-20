// Assumes _class_name_.class file

#define VSD_BEGIN_CLASS(_class_name_) \
class _class_name_: public VsdTree { \
public: \
  _class_name_(const char *name, const char * title) : \
    VsdTree(name, title) { m_vsd_tree_class = #_class_name_; } \
  _class_name_(TTree *tree) : \
    VsdTree(tree) { m_vsd_tree_class = #_class_name_; setAdressToSupportedBranches();}

#define VSD_REGISTER_BRANCH(_type_, _name_) \
protected: \
  VsdBranch<_type_> m_ ## _name_ { this, #_name_, #_type_, "std::vector<" #_type_ ">" }; \
public: \
  bool has_ ## _name_() const { return !m_ ## _name_.m_collection.empty(); } \
  const std::vector<_type_>& _name_() const { return m_ ## _name_.m_collection; } \
  std::vector<_type_>& _name_() { return m_ ## _name_.m_collection; } \
  const _type_& _name_(int i) const { return _name_()[i]; } \
  _type_& _name_(int i) { return _name_()[i]; } \
  int _name_ ## _size() const { return (int) _name_().size(); }

#define VSD_END_CLASS \
};
