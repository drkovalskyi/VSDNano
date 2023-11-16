#include "VsdBase.h"
#include "VsdTree.h"

VsdProvider *g_provider = nullptr;


#define FILL_TEST(_name_) \
    for (auto &i : m_vsdTree-> ## _name_ ## ) {  i.dump();}




class TestProvider : public VsdProvider
{
public:
    VsdTree* m_vsdTree{nullptr};

    TestProvider(std::string fn)
    {
        TFile* f = TFile::Open(fn.c_str(), "READ");
        TTree* tree = (TTree*) f->Get("VSD");
        m_vsdTree = new VsdTree(tree);
        m_vsdTree->goto_event(0);

        for (auto &s : m_vsdTree->m_supported_vector)
        {
           // printf("branch name %s trype %s colType %s \n", s->m_name.c_str(), s->m_type.c_str(), s->m_collection_type.c_str());
            std::string name =  s->m_name;
            std::string purpose = s->m_type.substr(3);
            VsdCollection* c = new VsdCollection(name, purpose);
            c->m_type = s->m_type;
            addCollection(c);
            
        }

    }

    virtual Long64_t GetNumEvents() { return m_vsdTree->n_events(); }

    virtual void GotoEvent(int eventIdx)
    {
        printf("Go to event %d \n", eventIdx);
        m_eventIdx = eventIdx;
        m_vsdTree->goto_event(eventIdx);

        // fill VSD collections
        for (auto h : m_collections)
            h->m_list.clear();

        {
           VsdCollection* vsdc = RefColl("primvs");
           for (auto &i : m_vsdTree->primvs())vsdc->m_list.push_back(&i);
        }
        {
           VsdCollection* vsdc = RefColl("jets");
           for (auto &i : m_vsdTree->jets()) vsdc->m_list.push_back(&i);
        }
        {
            VsdCollection* vsdc = RefColl("cands");
            for (auto &i : m_vsdTree->cands()) vsdc->m_list.push_back(&i);
        }
        {
            VsdCollection* vsdc = RefColl("muons");
            for (auto &i : m_vsdTree->muons()) vsdc->m_list.push_back(&i);
        }
        {
            VsdCollection* vsdc = RefColl("caloMETs");
            for (auto &i : m_vsdTree->caloMETs()) vsdc->m_list.push_back(&i);
        }
        {
            VsdCollection* vsdc = RefColl("chsMETs");
            for (auto &i : m_vsdTree->chsMETs()) vsdc->m_list.push_back(&i);
        }
        {
            VsdCollection* vsdc = RefColl("infos");
            for (auto &i : m_vsdTree->infos()) vsdc->m_list.push_back(&i);
        }
        set_event_info();
   }

};


void testevd()
{
   g_provider = new TestProvider("vsd-nano.root");
   gROOT->LoadMacro("evd.h");
   gROOT->ProcessLine("evd()");
}
