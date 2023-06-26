#ifndef VSDBase_h
#define VSDBase_h

class VSDReader;

/////////////////////////////////////////////////
class VSDBase
{
public:
   virtual void dump() { printf("dump VSD Base class\n"); }
};

/////////////////////////////////////////////////
class VSDVertex : public VSDBase
{
private:
   //ROOT::Math::Polar3DPoint location;
   float m_x;
   float m_y; 
   float m_z;

public:
   VSDVertex(float ix, float iy, float iz) { m_x= ix; m_y = iy; m_z = iz;}
   void dump() { printf("VSDVertex x:%.2f, y:%.2f, z:%.2f \n", m_x, m_y,m_z); }

   float x() const { return m_x; }
   float y() const { return m_y; }
   float z() const { return m_z; }
};

/////////////////////////////////////////////////
class VSDCandidate : public VSDBase
{
protected:
   // ROOT::Math::Polar3DPoint momentum;
   float m_eta{0.f}; float m_phi{0.f}; float m_pt{0.f};
   int m_charge{0};

public:
   VSDCandidate(float ipt, float ieta, float iphi, int charge = 0) :m_pt(ipt), m_eta(ieta), m_phi(iphi), m_charge(charge){}
   float phi() const { return m_phi; }
   float eta() const { return m_eta; }
   float pt() const { return m_pt; }
   float charge() const { return m_charge; }

   void dump() { printf("VSDCanidate %f\n", m_pt); }
};

/////////////////////////////////////////////////
class VSDJet : public VSDCandidate
{
private:
   float m_hadFraction{0.f};
   float m_coneR{0.2f}; // cone radius in eta phi space

public:
   float hadFraction() const { return m_hadFraction; }
   float coneR() const { return m_coneR; }

   VSDJet(float pt, float eta, float phi, float had_fraction, float coneR = 0.2) : VSDCandidate(pt, eta, phi), m_hadFraction(had_fraction), m_coneR(coneR) {}

   using VSDBase::dump;
   void dump() { printf("VSDJet pt:%.2f, eta:%.2f, phi:%.2f / had_frac: %.2f\n", m_pt, m_eta, m_phi, m_hadFraction); }
};


/////////////////////////////////////////////////
class VSDMuon : public VSDCandidate
{
   bool m_global{false};

public:
   float global() const { return m_global; }

   VSDMuon(float pt, float eta, float phi, int charge, bool global) : VSDCandidate(pt, eta, phi, charge), m_global(global) {}
};

////////////////////////////////////////////////
class VSDMET : public VSDCandidate
{
private:
  float m_sumEt{0.f};
public:
   float sumEt() { return m_sumEt; }
   using VSDBase::dump;
   VSDMET(float pt, float eta, float phi, float sumEt) :  VSDCandidate(pt, eta, phi), m_sumEt(sumEt) {}
   void dump() { printf("VSDMET: phi: 2f, sumEt:%.2f / pt: %.2f\n", m_phi, m_sumEt); }
};

/////////////////////////////////////////////////
// Event structs
/////////////////////////////////////////////////

//typedef std::vector<VSDBase *> VSDCollection;

class VSDCollection
{
public:
   VSDCollection(const std::string& n, const std::string& p, Color_t c=kBlue, std::string f="") : 
                 m_name(n), m_purpose(p), m_color(c), m_filter(f) {}
   std::string m_name;
   std::string m_purpose;
   std::string m_filter;
   Color_t     m_color{kBlue};
   std::vector<VSDBase *> m_list;

   virtual void fill(VSDReader &) {}
};


/////////////////////////////////////////////////
class VSDProvider;
VSDProvider *g_provider = nullptr;
class VSDProvider
{
public:
   struct EventInfo
   { 
      uint run{99};
      uint lumi{99};
      Long64_t event{99};
   };

   VSDProvider(TTree* t) : m_tree(t){}

   TTree *m_tree{nullptr};
   EventInfo m_eventInfo;
   VSDReader* m_data{nullptr};
   Long64_t m_eventIdx{0};
   std::vector<VSDCollection *> m_collections;


   virtual Long64_t GetNumEvents() { return (int)m_tree->GetEntriesFast(); }

   void addCollection(VSDCollection *h)
   {
      m_collections.push_back(h);
   }

   virtual void GotoEvent(int eventIdx)
   {
      m_eventIdx = eventIdx;
      m_tree->GetEntry(eventIdx);

      // fill VSD collections
      for (auto h : m_collections)
      {
         h->m_list.clear();
      //  h->fill(*m_data);

         h->fill(*(this->m_data));

         // debug
         for (auto e : h->m_list)
            e->dump();
      }
      set_event_info(m_eventInfo);
   }

  // virtual void fill_collections() = 0;
   virtual void set_event_info(EventInfo& ) = 0;

   VSDCollection *RefColl(const std::string &name)
   {
      for (auto collection : m_collections)
      {
         if (name == collection->m_name)
            return collection;
      }
      return nullptr;
   };
};


////////////////////////////////////////////////////////////
void makeVSDClassAndObj(const std::string& ci, const std::string &desc, Color_t c, std::string f)
{
   std::string exp = std::regex_replace(ci, std::regex("ZZZ"), desc);
   std::cout << exp << "\n";
   gROOT->ProcessLine(exp.c_str());
   VSDCollection *coll = g_provider->RefColl(desc);
   coll->m_color = c;
   coll->m_filter = f;
   coll->m_name = desc;
}


////////////////////////////////////////////////////////////
void MakeCollFromFillStr(const std::string& fillBody, const std::string &desc, const std::string &purpose, Color_t c, std::string f)
{
   std::string cname = desc; cname += purpose; cname +="Collection";
   std::stringstream ss;
   ss << "class "<< cname << " : public VSDCollection \n"
      << "{\n"
      << "public:\n"
      <<    cname << "(const std::string &n, const std::string &p) : VSDCollection(n, p) {}\n"
      << "  virtual void fill(VSDReader &r) {\n" << fillBody << "\n}\n"
      << "};\n"
      << "\n"
      << "g_provider->addCollection(new " << cname << "(\"ZZZ\",\"" << purpose << "\"));\n";
   makeVSDClassAndObj(ss.str(), desc, c, f);
}


#endif // #ifdef VSDBase
