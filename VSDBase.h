#ifndef VSDBase_h
#define VSDBase_h

#include "nlohmann/json.hpp"
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
public:
   //ROOT::Math::Polar3DPoint location;
   float m_x;
   float m_y; 
   float m_z;

public:
   // VSDVertex(){}
   // VSDVertex(float ix, float iy, float iz) { m_x= ix; m_y = iy; m_z = iz;}
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
   int m_charge{0};

public:
   // VSDCandidate(float ipt, float ieta, float iphi, int charge = 0) :m_pt(ipt), m_eta(ieta), m_phi(iphi), m_charge(charge){}
   float phi() const { return m_phi; }
   float eta() const { return m_eta; }
   float pt() const { return m_pt; }
   float charge() const { return m_charge; }

   void dump() { printf("VSDCanidate %f\n", m_pt); }
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

   // VSDMuon(float pt, float eta, float phi, int charge, bool global) : VSDCandidate(pt, eta, phi, charge), m_global(global) {}
};

////////////////////////////////////////////////
class VSDMET : public VSDCandidate
{
public:
  float m_sumEt{0.f};
public:
   // VSDMET(float pt, float eta, float phi, float sumEt) :  VSDCandidate(pt, eta, phi), m_sumEt(sumEt) {}
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
   VSDProvider(TTree *t) : m_tree(t)
   {
      m_data = new VSDReader(t);
   }
   TTree *m_tree{nullptr};
   VSDEventInfo m_eventInfo;
   VSDReader* m_data{nullptr};
   Long64_t m_eventIdx{0};
   std::vector<VSDCollection *> m_collections;
   nlohmann::json m_config;

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

         h->fill(*(this->m_data));

         // debug
         for (auto e : h->m_list)
            e->dump();
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
   };

   nlohmann::json &RefVSDMemberConfig() { return m_config; }

   ////////////////////////////////////////////////////////////
   void registerCollection(const std::string &desc, const std::string &vsdClassType, Color_t color = kBlue, std::string filter = "")
   {
      try
      {
         using namespace nlohmann;
         TString cmd;
         json &j = m_config[vsdClassType];

         std::cout << j.dump(4) << "\n";
         std::string numKey;
         try {
               numKey = j["N"];
         }
         catch (std::exception &e) {
            std::cout << "missing collection size info " << e.what() << "\n"; 
         }

         // single element collection
         if (numKey == "undef")
         {
            cmd += TString::Format("auto vsdObj = new VSD%s();\n", vsdClassType.c_str());
            for (json::iterator it = j.begin(); it != j.end(); ++it)
            {
               if (it.key() == "N")
                  continue;

               std::string k = it.key(), v = it.value();
               cmd += TString::Format("vsdObj->m_%s = r.ZZZ%s;\n", k.c_str(), v.c_str());
            }
            cmd += "m_list.push_back(vsdObj);\n";
         }
         else // create from array
         {
            cmd += TString::Format("for (int i = 0; i < r.%sZZZ; ++i) {\n", numKey.c_str());
            cmd += TString::Format("auto vsdObj = new VSD%s();\n", vsdClassType.c_str());

            for (json::iterator it = j.begin(); it != j.end(); ++it)
            {
               if (it.key() == "N")
                  continue;

               std::string k = it.key(), v = it.value();
               cmd += TString::Format("vsdObj->m_%s = r.ZZZ%s[i]; \n", k.c_str(), v.c_str());
            }
            cmd += "m_list.push_back(vsdObj);\n";
            cmd += "}\n // end loop through vsd array";
         }
         // printf("VSD collection fill body  %s \n ", cmd.Data());

         // make sources for class
         std::string cname = desc;
         cname += vsdClassType;
         cname += "Collection";
         std::stringstream ss;
         ss << "class " << cname << " : public VSDCollection \n"
            << "{\n"
            << "public:\n"
            << cname << "(const std::string &n, const std::string &p) : VSDCollection(n, p) {}\n"
            << "  virtual void fill(VSDReader &r) {\n"
            << cmd.Data() << "\n}\n"
            << "};\n"
            << "\n"
            << "g_provider->addCollection(new " << cname << "(\"ZZZ\",\"" << vsdClassType << "\"));\n";

         std::cout << ss.str() << "\n\n.....\n";
         std::string exp = std::regex_replace(ss.str(), std::regex("ZZZ"), desc);
         std::cout << "Expression to evaluate" << exp << "\n";
         gROOT->ProcessLine(exp.c_str());

         // config collection
         VSDCollection *coll = RefColl(desc);
         coll->m_color = color;
         coll->m_filter = filter;
         coll->m_name = desc;
      }
      catch (std::exception &e)
      {
         std::cerr << e.what() << "\n";
      }
   };
/*
   ////////////////////////////////////////////////////////////
   void MakeCollFromFillStr(TString &fillBody, const std::string &desc, const std::string &vsdClassType, Color_t color, std::string filter)
   {
      std::string cname = desc;
      cname += vsdClassType;
      cname += "Collection";
      std::stringstream ss;
      ss << "class " << cname << " : public VSDCollection \n"
         << "{\n"
         << "public:\n"
         << cname << "(const std::string &n, const std::string &p) : VSDCollection(n, p) {}\n"
         << "  virtual void fill(VSDReader &r) {\n"
         << fillBody.Data() << "\n}\n"
         << "};\n"
         << "\n"
         << "g_provider->addCollection(new " << cname << "(\"ZZZ\",\"" << vsdClassType << "\"));\n";


      // std::cout << ss.str();   
      makeVSDClassAndObj(ss.str(), desc, color, filter);
   }

   ////////////////////////////////////////////////////////////
   void makeVSDClassAndObj(const std::string &ci, const std::string &desc, Color_t c, std::string f)
   {
      std::string exp = std::regex_replace(ci, std::regex("ZZZ"), desc);
      // std::cout << exp << "\n";
      gROOT->ProcessLine(exp.c_str());
      VSDCollection *coll = g_provider->RefColl(desc);
      coll->m_color = c;
      coll->m_filter = f;
      coll->m_name = desc;
   }*/
}; // end class VSD provider

#endif // #ifdef VSDBase
