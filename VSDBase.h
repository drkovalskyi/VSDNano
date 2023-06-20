#ifndef VSDBase_h
#define VSDBase_h

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
   int m_charge{1};

public:
   VSDCandidate(float ieta, float iphi, float ipt) : m_eta(ieta), m_phi(iphi), m_pt(ipt){}
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

public:
   float hadFraction() const { return m_hadFraction; }

   VSDJet(float eta, float phi, float pt, float had_fraction) : VSDCandidate(eta, phi, pt), m_hadFraction(had_fraction) {}

   using VSDBase::dump;
   void dump() { printf("VSDJet pt:%.2f, eta:%.2f, phi:%.2f / had_frac: %.2f\n", m_pt, m_eta, m_phi, m_hadFraction); }
};


/////////////////////////////////////////////////
class VSDElectron : public VSDCandidate
{
   float m_hadronicOverEm{0.f};

public:
   float hadronicOverEm() const { return m_hadronicOverEm; }

   VSDElectron(float eta, float phi, float pt, float had_fraction) : VSDCandidate(eta, phi, pt), m_hadronicOverEm(had_fraction) {}
};

////////////////////////////////////////////////
class VSDMET : public VSDCandidate
{
private:
  float m_sumEt{0.f};
public:
   float sumEt() { return m_sumEt; }
   using VSDBase::dump;
   VSDMET(float phi, float pt, float sumEt) :  VSDCandidate(0.f, phi, pt), m_sumEt(sumEt) {}
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

class VSDProvider
{
   public:

// event info
   uint m_eventInfoRun{99};
   uint m_eventInfoLumi{99};
   Long64_t m_eventInfoEvent{99};
   Long64_t m_eventIdx{0};


   // re-formatting
   virtual void GotoEvent(int eventIdx) = 0;
   virtual Long64_t GetNumEvents() = 0;

   // filtering
   virtual void setFilterExpr(const std::string &) = 0;
   bool getFilterEnabled() { return false; }
   std::vector<VSDCollection *> m_collections;

   VSDCollection* RefColl(const std::string &name)
   {
       for (auto collection : m_collections)
       {
           if (name == collection->m_name)
               return collection;
       }
       return nullptr;
   };
};

#endif // #ifdef VSDBase
