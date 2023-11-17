#include "VsdBase.h"
#include "MyVsdNanoTree.h"
#include "ROOT/RDF/Utils.hxx"
#include "ROOT/RDataFrame.hxx"


typedef std::vector<VsdCollection> VsdCollections;

////////////////////////////////////////////////////////////////////////////
//////// HELPER CLASSES ////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#define RDF_EMPTY(_name_) \
public: \
    _name_(unsigned int) {} \
   std::string GetActionName() const { return "vsdhelper"; } \
   void Initialize() { std::cout << "initialize " << GetActionName() <<" \n"; } \
   void InitTask(TTreeReader *, int) {std::cout << "init task\n";} \
   using Result_t = VsdCollections; \
  std::shared_ptr<VsdCollections> GetResultPtr() const { return fFinalResult; } \
  std::shared_ptr<VsdCollections> fFinalResult = std::make_shared<VsdCollections>(); \
   void Finalize(){}


//----------------------------------------------------------------
class CandHelper : public ROOT::Detail::RDF::RActionImpl<CandHelper> {
public:
   RDF_EMPTY(CandHelper);

   /// Called at every entry.
   template <typename nSize_t, typename arrPt_t, typename arrEt_t, typename arrPhi_t>
   void Exec(unsigned int slot, nSize_t n, arrPt_t mPts, arrEt_t mEts, arrPhi_t mPhis)
   {
      fFinalResult->push_back(VsdCollection());
      for (unsigned int i = 0; i < n; i++)
      {
         VsdCandidate *m = new VsdCandidate();
         m->setPt(mPts[i]);
         m->setEta(mEts[i]);
         m->setPhi(mPhis[i]);
         fFinalResult->back().m_list.push_back(m);
      }
   }

};

//----------------------------------------------------------------
class MuHelper : public ROOT::Detail::RDF::RActionImpl<MuHelper> {
public:
   RDF_EMPTY(MuHelper);

   /// Called at every entry.
   template <typename nSize_t, typename arrPt_t, typename arrEt_t, typename arrPhi_t>
   void Exec(unsigned int slot, nSize_t n, arrPt_t mPts, arrEt_t mEts, arrPhi_t mPhis)
   {
      fFinalResult->push_back(VsdCollection());
      for (unsigned int i = 0; i < n; i++)
      {
         VsdMuon *m = new VsdMuon();
         m->setPt(mPts[i]);
         m->setEta(mEts[i]);
         m->setPhi(mPhis[i]);
         fFinalResult->back().m_list.push_back(m);
      }
   }

};
//----------------------------------------------------------------
class JetHelper : public ROOT::Detail::RDF::RActionImpl<JetHelper> {
public:
   RDF_EMPTY(JetHelper);
   template <typename nSize_t, typename arrPt_t, typename arrEt_t, typename arrPhi_t>
   void Exec(unsigned int slot, nSize_t n, arrPt_t mPts, arrEt_t mEts, arrPhi_t mPhis)
   {
       fFinalResult->push_back(VsdCollection());
      for (unsigned int i = 0; i < n; i++)
      {
         VsdJet *m = new VsdJet();
         m->setPt(mPts[i]);
         m->setEta(mEts[i]);
         m->setPhi(mPhis[i]);
         // jet had ratio ??
         fFinalResult->back().m_list.push_back(m);
      }
   }
};


//---------------------------------------------------------------
class VertexHelper : public ROOT::Detail::RDF::RActionImpl<VertexHelper> {
public:
   RDF_EMPTY(VertexHelper);
   /// Called at every entry.
   template <typename pvX, typename pvY, typename pvZ>
   void Exec(unsigned int slot, pvX ipvx, pvY ipvy, pvZ ipvz)
   {
      fFinalResult->push_back(VsdCollection());
      VsdVertex *m = new VsdVertex;
      m->setX(ipvx);
      m->setY(ipvy);
      m->setZ(ipvz);
      fFinalResult->back().m_list.push_back(m);
   }
};
//---------------------------------------------------------------
class EIHelper : public ROOT::Detail::RDF::RActionImpl<EIHelper> {
public:
   RDF_EMPTY(EIHelper);
   /// Called at every entry.
   template <typename run_t, typename lumi_t, typename event_t>
   void Exec(unsigned int slot, run_t run, lumi_t lumi, event_t event)
   {
      fFinalResult->push_back(VsdCollection());
      VsdEventInfo *m = new VsdEventInfo(run, lumi, event);
      fFinalResult->back().m_list.push_back(m);
   }
};

//---------------------------------------------------------------
class CaloMETHelper : public ROOT::Detail::RDF::RActionImpl<CaloMETHelper> {
public:
   RDF_EMPTY(CaloMETHelper);

   template <typename Phi_t, typename Pt_t, typename SumEt_t>
   void Exec(unsigned int slot, Phi_t iphi, Pt_t ipt, SumEt_t isumet)
   {
      fFinalResult->push_back(VsdCollection());
      VsdMET* m = new VsdMET();
      m->setPt(ipt);
      //m->setEta(ieta);
      m->setPhi(iphi);
      m->setSumEt(isumet);
      fFinalResult->back().m_list.push_back(m);
   }
};

//---------------------------------------------------------------
class ChsMETHelper : public ROOT::Detail::RDF::RActionImpl<ChsMETHelper> {
public:
   RDF_EMPTY(ChsMETHelper);

   template <typename Phi_t, typename Pt_t, typename SumEt_t>
   void Exec(unsigned int slot, Phi_t iphi, Pt_t ipt, SumEt_t isumet)
   {
      fFinalResult->push_back(VsdCollection());
      VsdMET* m = new VsdMET();
      m->setPt(ipt);
      //m->setEta(ieta);
      m->setPhi(iphi);
      m->setSumEt(isumet);
      fFinalResult->back().m_list.push_back(m);
   }
};

///////////////////////////////////////////////////////////////////////////////
////////////////// main //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void rdf()
{
   std::string path  = "/home/alja/root-dev/universal_format/nano.root";
   ROOT::RDataFrame d("Events", path);

   // go to event that has muons
   unsigned long firstEvent = 0;
   unsigned long lastEvent = 10;
   auto node = d.Range(firstEvent,lastEvent);


   auto eleHelper = node.Book(CandHelper(d.GetNSlots()), {"nElectron", "Electron_pt", "Electron_eta", "Electron_phi"});
   auto eleResult = *eleHelper;


   auto muonHelper = node.Book(MuHelper(d.GetNSlots()), {"nMuon", "Muon_pt", "Muon_eta", "Muon_phi"});
   auto muonResult = *muonHelper;


   auto jetHelper = node.Book(JetHelper(d.GetNSlots()), {"nJet", "Jet_pt", "Jet_eta", "Jet_phi"});
   auto jetResult = *jetHelper;

   auto vtxHelper = node.Book(VertexHelper(d.GetNSlots()), {"PV_x", "PV_y", "PV_z"});
   auto vtxResult = *vtxHelper;

   auto eiHelper = node.Book(EIHelper(d.GetNSlots()), {"run", "luminosityBlock", "event"});
   auto eiResult = *eiHelper;

   auto caloMETHelper = node.Book(CaloMETHelper(d.GetNSlots()), {"CaloMET_phi", "CaloMET_pt", "CaloMET_sumEt"});
   auto caloMETResult = *caloMETHelper;

   auto chsMETHelper = node.Book(ChsMETHelper(d.GetNSlots()), {"ChsMET_phi", "ChsMET_pt", "ChsMET_sumEt"});
   auto chsMETResult = *caloMETHelper;

   printf("VSD write \n");
   // Write to VSD
   TFile *f = TFile::Open("vsd-nano.root", "RECREATE");
   MyVsdNanoTree vsdt("VSD", "VSD TTree test");
   vsdt.append_collections({"cands", "muons", "jets", "primvs", "caloMETs", "chsMETs", "infos"});
   vsdt.prepare_for_writing();

   printf("prepare to write in Vsd tree\n");
   for (int e = firstEvent; e < lastEvent; ++e)
   {
      printf("=== Event [%d] ===\n", e);
      {
         auto vsdc = eleResult[e];
         for (auto vsd : vsdc.m_list) {
            VsdCandidate* m = dynamic_cast<VsdCandidate*>(vsd);
            vsdt.cands().emplace_back(m->pt(), m->eta(), m->phi());
            m->dump();
         }
      }
      {
         auto vsdc = muonResult[e];
         for (auto vsd : vsdc.m_list) {
            VsdMuon* m = dynamic_cast<VsdMuon*>(vsd);
            vsdt.muons().emplace_back(m->pt(), m->eta(), m->phi());
         }
      }
      {
         auto vsdc = jetResult[e];
         for (auto vsd : vsdc.m_list) {
            VsdJet* m = dynamic_cast<VsdJet*>(vsd);
            vsdt.jets().emplace_back(m->pt(), m->eta(), m->phi(), 0.5, 0.2); // ? had fraction
            m->dump();
         }
      }
      {
         auto vsdc = vtxResult[e];
         for (auto vsd : vsdc.m_list) {
            VsdVertex* m = dynamic_cast<VsdVertex*>(vsd);
            vsdt.primvs().emplace_back(m->x(), m->y(), m->z());
            m->dump();
         }
      }
      {
         auto vsdc = caloMETResult[e];
         for (auto vsd : vsdc.m_list) {
            VsdMET* m = dynamic_cast<VsdMET*>(vsd);
            vsdt.caloMETs().emplace_back(m->pt(), m->eta(), m->phi(), m->sumEt());
            m->dump();
         }
      }
      {
         auto vsdc = chsMETResult[e];
         for (auto vsd : vsdc.m_list) {
            VsdMET* m = dynamic_cast<VsdMET*>(vsd);
            vsdt.chsMETs().emplace_back(m->pt(), m->eta(), m->phi(), m->sumEt());
            m->dump();
         }
      }
      {
         for (auto vsd : eiResult[e].m_list) {
            VsdEventInfo* m = dynamic_cast<VsdEventInfo*>(vsd);
            vsdt.infos().emplace_back(m->run(), m->lumi(), m->event());
            std::cout  << "event of " << m->event() << "\n";
            m->dump();
         }
      }

     vsdt.fill_all_braches(true);
   }

   vsdt.finalize_writing();

   f->Close();
   delete f;
}


