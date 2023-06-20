#include "TTree.h"
#include "TLeaf.h"
#include "Math/Point3D.h"
#include "TFile.h"

// Data access code
#define VSDReader_cxx
#include "VSDReader.h"
void VSDReader::Loop(){};

#include "VSDBase.h"

class NanoVSDProvider : public VSDProvider
{
private:
   TTree *m_tree{nullptr};
   VSDReader *m_data{nullptr};

public:
   NanoVSDProvider(const std::string &filePath)
   {
      auto file = TFile::Open(filePath.c_str());
      m_tree = (TTree *)file->Get("Events");

      // setup data access
      m_data = new VSDReader(m_tree);
   }

   using VSDProvider::GetNumEvents;
   Long64_t GetNumEvents()
   {
      return (int)m_tree->GetEntriesFast();
   }

   void addCollection(VSDCollection *h)
   {
      m_collections.push_back(h);
   }

   using VSDProvider::GotoEvent;
   void GotoEvent(int eventIdx) /// AMT should this be set event ??
   {
      m_eventIdx = eventIdx;
      m_tree->GetEntry(eventIdx);

      // event info
      m_eventInfoRun = m_data->run;
      m_eventInfoLumi = m_data->luminosityBlock;
      m_eventInfoEvent = m_data->event;
      printf("nanoprovier %lld events total %lld !!!!! \n", m_eventIdx, GetNumEvents());

      fill_collections();
   }
   ///////////////////////////////////////////////////////////////////
   // User code goes here
   void
   fill_collections()
   {
      for (auto h : m_collections)
      {
         printf("helper list ..............................................\n");
         h->m_list.clear();
         h->fill(*m_data);

         for (auto e : h->m_list)
            e->dump();
      }
   }

   using VSDProvider::setFilterExpr;
   void setFilterExpr(const std::string &) { printf("filtering not implemented"); }

};
