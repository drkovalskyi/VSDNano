#include "VSDBase.h"
//#define VSDReader_cxx
//#include "VSDReader.h"

class NanoVSDProvider : public VSDProvider
{
private:
   VSDReader *m_data{nullptr};

public:
   NanoVSDProvider(TTree* tree)
   {
      m_tree = tree;
      m_data = new VSDReader(m_tree);
   }

   ///////////////////////////////////////////////////////////////////
   // User code goes here
   void fill_collections() override
   {
      // event info presumes as a collection
      m_eventInfoRun = m_data->run;
      m_eventInfoLumi = m_data->luminosityBlock;
      m_eventInfoEvent = m_data->event;
      printf("nanoprovier %lld events total %lld !!!!! \n", m_eventIdx, GetNumEvents());

      // reformat
      for (auto h : m_collections)
      {
         h->m_list.clear();
         h->fill(*m_data);

         // debug
         for (auto e : h->m_list)
            e->dump();

      }
   }
};

NanoVSDProvider *g_provider = nullptr;
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

class NanoVSDJetCollection : public VSDCollection
{
public:
   NanoVSDJetCollection(const std::string &n, const std::string &p, Color_t c=kBlue, std::string f="") : VSDCollection(n, p, c,f) {}
   virtual void fill(VSDReader &r)
   {
      for (int i = 0; i < r.nJet; ++i) {
         m_list.push_back(new VSDJet(r.Jet_pt[i], r.Jet_eta[i], r.Jet_phi[i], r.Jet_chHEF[i]));
      }
   }
};

class NanoVSDVertexCollection : public VSDCollection
{
public:
   NanoVSDVertexCollection(const std::string &n, const std::string &p, Color_t c = kBlue, std::string f = "") : VSDCollection(n, p, c, f) {}
   virtual void fill(VSDReader &r)
   {
      m_list.push_back(new VSDVertex(r.PV_x, r.PV_y, r.PV_z));
   }
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
void registerMETCollection(const std::string &desc, const std::string &purpose, Color_t c = kBlue, std::string f = "")
{
   std::stringstream ss;
   ss << "class ZZZ_VSDCollection : public VSDCollection \n"
      << "{\n"
      << "public:\n"
      << "  ZZZ_VSDCollection(const std::string &n, const std::string &p) : VSDCollection(n, p) {}\n"
      << "  virtual void fill(VSDReader &r) {\n"
      << "     m_list.push_back(new VSDMET(r.ZZZ_pt, 0, r.ZZZ_phi, r.ZZZ_sumEt)); \n"
      << "  }\n"
      << "};\n"
      << "\n"
      << "g_provider->addCollection(new ZZZ_VSDCollection(\"ZZZ\",\"" << purpose << "\"));\n";
   makeVSDClassAndObj(ss.str(), desc, c, f);
}

////////////////////////////////////////////////////////////
void registerCandidateCollection(const std::string &desc, const std::string &purpose, Color_t c = kBlue, std::string f = "")
{
   std::stringstream ss;
   ss << "class ZZZ_VSDCandidateCollection : public VSDCollection \n"
      << "{\n"
      << "public:\n"
      << "  ZZZ_VSDCandidateCollection(const std::string &n, const std::string &p) : VSDCollection(n, p) {}\n"
      << "  virtual void fill(VSDReader &r) {\n"
      << "     for (int i = 0; i < r.nZZZ; ++i)\n"
      << "     m_list.push_back(new VSDCandidate(r.ZZZ_pt[i], r.ZZZ_eta[i], r.ZZZ_phi[i],r.ZZZ_charge[i] )); \n"
      << "  }\n"
      << "};\n"
      << "\n"
      << "g_provider->addCollection(new ZZZ_VSDCandidateCollection(\"ZZZ\",\"" << purpose << "\"));\n";
   makeVSDClassAndObj(ss.str(), desc, c, f);
}

////////////////////////////////////////////////////////////
void registerMuonCollection(const std::string &desc, const std::string &purpose, Color_t c = kBlue, std::string f = "")
{
   std::stringstream ss;
   ss << "class ZZZ_MuonCollection : public VSDCollection \n"
      << "{\n"
      << "public:\n"
      << "  ZZZ_MuonCollection(const std::string &n, const std::string &p) : VSDCollection(n, p) {}\n"
      << "  virtual void fill(VSDReader &r) {\n"
      << "     for (int i = 0; i < r.nZZZ; ++i) {\n"
      << "     m_list.push_back(new VSDMuon(r.ZZZ_pt[i], r.ZZZ_eta[i], r.ZZZ_phi[i],  r.ZZZ_charge[i], r.ZZZ_isGlobal[i])); std::cout << r.ZZZ_charge[i] << std::endl;} \n"
      << "  }\n"
      << "};\n"
      << "\n"
      << "g_provider->addCollection(new ZZZ_MuonCollection(\"ZZZ\",\"" << purpose << "\"));\n";
   makeVSDClassAndObj(ss.str(), desc, c, f);
}
////////////////////////////////////////////////////////////

void cms_nano_aod_bootstrap(TTree* tree)
{
   g_provider = new NanoVSDProvider(tree);
   {
      g_provider->addCollection(new NanoVSDJetCollection("Jet", "Jet", kYellow));
      g_provider->addCollection(new NanoVSDVertexCollection("Vertex", "Vertex", kGreen));

      registerMETCollection("CaloMET", "MET", kRed);
      registerMETCollection("ChsMET", "MET", kViolet);
      registerCandidateCollection("Electron", "Candidate", kCyan);
      registerMuonCollection("Muon", "Muon", kRed);
   }

}
