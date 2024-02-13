#ifndef VsdBase_h
#define VsdBase_h

#include "nlohmann/json.hpp"
#include "TStyle.h"
#include "Rtypes.h"
#include "TTree.h"


class VsdProvider;
// VsdProvider *g_provider = nullptr;

/////////////////////////////////////////////////
class VsdBase
{
public:
   virtual ~VsdBase(){}
   virtual void dump() { printf("dump VSD Base class\n"); }
};

/////////////////////////////////////////////////
class VsdVertex : public VsdBase
{
public:
   //ROOT::Math::Polar3DPoint location;
   float m_x;
   float m_y; 
   float m_z;

public:
   VsdVertex() = default;
   VsdVertex& operator=(const VsdVertex&) = default;
   VsdVertex(float ix, float iy, float iz) { m_x= ix; m_y = iy; m_z = iz;}
   virtual ~VsdVertex(){}
   void dump() { printf("VsdVertex x:%.2f, y:%.2f, z:%.2f \n", m_x, m_y,m_z); }

   float x() const { return m_x; }
   float y() const { return m_y; }
   float z() const { return m_z; }

   void setX(float x) {m_x = x;}
   void setY(float x) {m_y = x;}
   void setZ(float x) {m_z = x;}
};

/////////////////////////////////////////////////
class VsdCandidate : public VsdBase
{
// protected:
public:
   // ROOT::Math::Polar3DPoint momentum;
   float m_eta{0.f}; float m_phi{0.f}; float m_pt{0.f};
   int m_charge{0};

public:
   VsdCandidate() = default;
   VsdCandidate& operator=(const VsdCandidate&) = default;
   VsdCandidate(float ipt, float ieta, float iphi, int charge = 0) :
      m_pt(ipt), m_eta(ieta), m_phi(iphi), m_charge(charge) {}
   virtual ~VsdCandidate(){}

   float phi() const { return m_phi; }
   float eta() const { return m_eta; }
   float pt() const { return m_pt; }
   float charge() const { return m_charge; }

   void setPt(float x) { m_pt = x;}
   void setEta(float x) { m_eta = x;}
   void setPhi(float x) { m_phi = x;}
   void setCharge(float x) { m_charge = x;}

   void dump() { printf("VsdCanidate pt = %f, charge = %d \n", m_pt, m_charge); }
};

/////////////////////////////////////////////////
class VsdJet : public VsdCandidate
{
// private:
public:
   float m_hadFraction{0.f};
   float m_coneR{0.2f}; // cone radius in eta phi space

public:
   VsdJet() = default;
   VsdJet& operator=(const VsdJet&) = default;
   VsdJet(float pt, float eta, float phi, int charge, float had_fraction, float coneR = 0.2) :
      VsdCandidate(pt, eta, phi, charge), m_hadFraction(had_fraction), m_coneR(coneR) {}
   virtual ~VsdJet(){}

   float hadFraction() const { return m_hadFraction; }
   float coneR() const { return m_coneR; }

   using VsdBase::dump;
   void dump() { printf("VsdJet pt:%.2f, eta:%.2f, phi:%.2f / had_frac: %.2f\n", m_pt, m_eta, m_phi, m_hadFraction); }
};

/////////////////////////////////////////////////
class VsdMuon : public VsdCandidate
{
   bool m_global{false};
public:
   float global() const { return m_global; }
   void setGlobal(bool x) { m_global = x; }

   VsdMuon() = default;
   VsdMuon& operator=(const VsdMuon&) = default;
   virtual ~VsdMuon(){}
   VsdMuon(float pt, float eta, float phi, int charge=0, bool global=false) : VsdCandidate(pt, eta, phi, charge), m_global(global) {}
};

////////////////////////////////////////////////
class VsdMET : public VsdCandidate
{
public:
  float m_sumEt{0.f};
public:
   VsdMET() = default;
   VsdMET& operator=(const VsdMET&) = default;
   VsdMET(float pt, float eta, float phi, float sumEt) :  VsdCandidate(pt, eta, phi), m_sumEt(sumEt) {}
   virtual ~VsdMET(){}

   float sumEt() { return m_sumEt; }
   void setSumEt(float x) { m_sumEt = x;}

   using VsdBase::dump;
   void dump() { printf("VsdMET: phi: 2f, sumEt:%.2f / pt: %.2f\n", m_phi, m_sumEt); }
};

////////////////////////////////////////////////
class VsdEventInfo : public VsdBase
{
   uint m_run{99};
   uint m_lumi{99};
   Long64_t m_event{99};

public:
   VsdEventInfo() = default;
   VsdEventInfo& operator=(const VsdEventInfo&) = default;

  VsdEventInfo(uint r, uint l, long e) {m_run = r; m_lumi=l; m_event=e; }
  uint run() {return m_run;}
  uint lumi() {return m_lumi;}
  Long64_t event() {return m_event;}

   using VsdBase::dump;
  void dump() { printf("run %d lumi %d event %lld \n", m_run, m_lumi, m_event);}
};

/////////////////////////////////////////////////
// Event structs
/////////////////////////////////////////////////

//typedef std::vector<VsdBase *> VsdCollection;

class VsdCollection
{
public:
   VsdCollection(const std::string &n, const std::string &p, Color_t c = kBlue, std::string f = "") : m_name(n), m_purpose(p), m_color(c), m_filter(f) {}

   VsdCollection() {}
   virtual ~VsdCollection() {}
   std::string m_name;
   std::string m_purpose;
   std::string m_type;
   Color_t m_color{kBlue};
   std::string m_filter;
   std::vector<VsdBase *> m_list;

   virtual void fill() {}
};

#endif // #ifdef VsdBase
