#include "VSDBase.h"

class NanoVSDProvider : public VSDProvider
{
public:
   NanoVSDProvider(TTree *tree) : VSDProvider(tree)
   {
      m_data = new VSDReader(m_tree);
   }

   ///////////////////////////////////////////////////////////////////
   // User code goes here
   void set_event_info(VSDProvider::EventInfo& eventInfo) override
   {
      printf("nanoprovier %lld events total %lld !!!!! \n", m_eventIdx, GetNumEvents());
      eventInfo.run = m_data->run;
      eventInfo.lumi = m_data->luminosityBlock;
      eventInfo.event = m_data->event;
   }
};

////////////////////////////////////////////////////////////
void registerJetCollection(const std::string &desc, const std::string &purpose, Color_t c = kBlue, std::string f = "")
{
   std::string s;
   s += "for (int i = 0; i < r.nZZZ; ++i) \n";
   s += "    m_list.push_back(new VSDJet(r.ZZZ_pt[i], r.ZZZ_eta[i], r.ZZZ_phi[i], r.ZZZ_chHEF[i])); \n";
   MakeCollFromFillStr(s, desc, purpose, c, f);
}
////////////////////////////////////////////////////////////
void registerMuonCollection(const std::string &desc, const std::string &purpose, Color_t c = kBlue, std::string f = "")
{
   std::string s;
   s += "for (int i = 0; i < r.nZZZ; ++i) \n";
   s += "    m_list.push_back(new VSDMuon(r.ZZZ_pt[i], r.ZZZ_eta[i], r.ZZZ_phi[i],  r.ZZZ_charge[i], r.ZZZ_isGlobal[i])); \n";
   MakeCollFromFillStr(s, desc, purpose, c, f);
}

////////////////////////////////////////////////////////////
void registerCandidateCollection(const std::string &desc, const std::string &purpose, Color_t c = kBlue, std::string f = "")
{
   std::string s;
   s += "for (int i = 0; i < r.nZZZ; ++i)\n";
   s += "    m_list.push_back(new VSDCandidate(r.ZZZ_pt[i], r.ZZZ_eta[i], r.ZZZ_phi[i],r.ZZZ_charge[i] )); \n";
   MakeCollFromFillStr(s, desc, purpose, c, f);
}

////////////////////////////////////////////////////////////
void registerMETCollection(const std::string &desc, const std::string &purpose, Color_t c = kBlue, std::string f = "")
{
   std::string s = "m_list.push_back(new VSDMET(r.ZZZ_pt, 0, r.ZZZ_phi, r.ZZZ_sumEt));";
   MakeCollFromFillStr(s, desc, purpose, c, f);
}
////////////////////////////////////////////////////////////
void registerVertexCollection(const std::string &desc, const std::string &purpose, Color_t c = kBlue, std::string f = "")
{
   std::string s = "m_list.push_back(new VSDVertex(r.ZZZ_x, r.ZZZ_y, r.ZZZ_z));";
   MakeCollFromFillStr(s, desc, purpose, c, f);
}

////////////////////////////////////////////////////////////

void cms_nano_aod_bootstrap(TTree *tree)
{
   g_provider = new NanoVSDProvider(tree);

   registerJetCollection("Jet", "Jet", kYellow);
   registerVertexCollection("PV", "Vertex", kGreen);
   registerMETCollection("CaloMET", "MET", kRed);
   registerMETCollection("ChsMET", "MET", kViolet);
   registerCandidateCollection("Electron", "Candidate", kCyan);
   registerMuonCollection("Muon", "Muon", kRed);
}
