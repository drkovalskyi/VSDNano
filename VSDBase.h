#ifndef VSDBase_h
#define VSDBase_h

#include "nlohmann/json.hpp"
#include "TStyle.h"
#include "Rtypes.h"
#include "TTree.h"


class VSDProvider;
VSDProvider *g_provider = nullptr;

/////////////////////////////////////////////////
class VSDBase
{
public:
virtual ~VSDBase(){}
   virtual void dump() { printf("dump VSD Base class\n"); }
};

/////////////////////////////////////////////////
class VSDVertex : public VSDBase
{
public:
   //ROOT::Math::Polar3DPoint location;
   float m_x;
   float m_y; 
   float m_z;

public:
   // VSDVertex(){}
   // VSDVertex(float ix, float iy, float iz) { m_x= ix; m_y = iy; m_z = iz;}
   virtual ~VSDVertex(){}
   void dump() { printf("VSDVertex x:%.2f, y:%.2f, z:%.2f \n", m_x, m_y,m_z); }

   float x() const { return m_x; }
   float y() const { return m_y; }
   float z() const { return m_z; }
};

/////////////////////////////////////////////////
class VSDCandidate : public VSDBase
{
// protected:
public:
   // ROOT::Math::Polar3DPoint momentum;
   float m_eta{0.f}; float m_phi{0.f}; float m_pt{0.f};
   float m_charge{0};

public:
   virtual ~VSDCandidate(){}
   // VSDCandidate(float ipt, float ieta, float iphi, int charge = 0) :m_pt(ipt), m_eta(ieta), m_phi(iphi), m_charge(charge){}
   float phi() const { return m_phi; }
   float eta() const { return m_eta; }
   float pt() const { return m_pt; }
   float charge() const { return m_charge; }

   void dump() { printf("VSDCanidate pt = %f, charge = %f \n", m_pt, m_charge); }
};

/////////////////////////////////////////////////
class VSDJet : public VSDCandidate
{
// private:
public:
   float m_hadFraction{0.f};
   float m_coneR{0.2f}; // cone radius in eta phi space

public:
   float hadFraction() const { return m_hadFraction; }
   float coneR() const { return m_coneR; }

   virtual ~VSDJet(){}
   //VSDJet(float pt, float eta, float phi, float had_fraction, float coneR = 0.2) : VSDCandidate(pt, eta, phi), m_hadFraction(had_fraction), m_coneR(coneR) {}

   using VSDBase::dump;
   void dump() { printf("VSDJet pt:%.2f, eta:%.2f, phi:%.2f / had_frac: %.2f\n", m_pt, m_eta, m_phi, m_hadFraction); }
};


/////////////////////////////////////////////////
class VSDMuon : public VSDCandidate
{
public:
   bool m_global{false};

   float global() const { return m_global; }

   virtual ~VSDMuon(){}
   // VSDMuon(float pt, float eta, float phi, int charge, bool global) : VSDCandidate(pt, eta, phi, charge), m_global(global) {}
};

////////////////////////////////////////////////
class VSDMET : public VSDCandidate
{
public:
  float m_sumEt{0.f};
public:
   // VSDMET(float pt, float eta, float phi, float sumEt) :  VSDCandidate(pt, eta, phi), m_sumEt(sumEt) {}
   virtual ~VSDMET(){}

   float sumEt() { return m_sumEt; }
   void dump() { printf("VSDMET: phi: 2f, sumEt:%.2f / pt: %.2f\n", m_phi, m_sumEt); }
};

////////////////////////////////////////////////
struct VSDEventInfo : public VSDBase
{
   uint m_run{99};
   uint m_lumi{99};
   Long64_t m_event{99};
};
/////////////////////////////////////////////////
// Event structs
/////////////////////////////////////////////////

//typedef std::vector<VSDBase *> VSDCollection;

class VSDCollection
{
public:
   VSDCollection(const std::string &n, const std::string &p, Color_t c = kBlue, std::string f = "") : m_name(n), m_purpose(p), m_color(c), m_filter(f) {}

   VSDCollection() {}
   virtual ~VSDCollection() {}
   std::string m_name;
   std::string m_purpose;
   Color_t m_color{kBlue};
   std::string m_filter;
   std::vector<VSDBase *> m_list;

   virtual void fill() {}
};

class VSDProvider
{
public:

    virtual ~VSDProvider(){}

    VSDEventInfo m_eventInfo;
    //  VSDReader* m_data{nullptr};
    Long64_t m_eventIdx{0};
    std::vector<VSDCollection *> m_collections;
    nlohmann::json *m_config{nullptr};
    std::string m_title{"VSDPRovider"};

    virtual Long64_t GetNumEvents() { return 0; }

    void addCollection(VSDCollection *h)
    {
        m_collections.push_back(h);
    }

    virtual void GotoEvent(int eventIdx)
    {
        printf("goto   start \n");
        // m_tree->GetEntry(eventIdx);
        m_eventIdx = eventIdx;

        // fill VSD collections
        for (auto h : m_collections)
        {
            h->m_list.clear();

            // h->fill(*(this->m_data));
            h->fill();

            // debug
            if (0)
            {
                for (auto e : h->m_list)
                    e->dump();
            }
        }
        set_event_info();
    }

    virtual void set_event_info()
    {
        printf("vsd provier %lld events total %lld !!!!! \n", m_eventIdx, GetNumEvents());

        for (auto &vsdc : m_collections)
        {
            if (vsdc->m_purpose == "EventInfo")
            {
                VSDEventInfo *ei = (VSDEventInfo *)vsdc->m_list[0];
                m_eventInfo = *ei;
                // printf("...... setting event info %lld \n", m_eventInfo.m_event);
                return;
            }
        }
    }

    VSDCollection *RefColl(const std::string &name)
    {
        for (auto collection : m_collections)
        {
            if (name == collection->m_name)
                return collection;
        }
        return nullptr;
    }
};



#endif // #ifdef VSDBase
