#include "VsdBase.h"
#include "MyVsdNanoTree.h"
#include "VsdTree.h"

VsdProvider *g_provider = nullptr;


class TestProvider : public VsdProvider
{
public:
    MyVsdNanoTree* m_vsdTree{nullptr};

    // construcutor
    TestProvider(std::string fn)
    {
        TFile* f = TFile::Open(fn.c_str(), "READ");
        TTree* tree = (TTree*) f->Get("VSD");
        m_vsdTree = new MyVsdNanoTree(tree);
        m_vsdTree->append_collections({"cands", "muons", "jets", "primvs", "caloMETs", "chsMETs", "infos"});
        m_vsdTree->goto_event(0);

        for (auto &s : m_vsdTree->m_supported_vector)
        {
            printf("branch name %s trype %s colType %s \n", s->m_name.c_str(), s->m_type.c_str(), s->m_collection_type.c_str());
            std::string name =  s->m_name;
            std::string purpose = s->m_type.substr(3);
            VsdCollection* c = new VsdCollection(name, purpose);
            c->m_type = s->m_type;


    TBranch *br = (TBranch*) tree->GetListOfBranches()->FindObject(name.c_str());
      s->set_branch_address(br);


            addCollection(c);
        }
    }

    virtual void GotoEvent(int eventIdx)
    {
        printf("Go to event %d \n", eventIdx);
        m_eventIdx = eventIdx;
        m_vsdTree->goto_event(eventIdx);

        // fill VSD collections
        for (auto h : m_collections)
        {
            h->m_list.clear();
            std::string cname = h->m_name;
            auto bbi = m_vsdTree->m_active_map.find(cname);
            if (bbi !=  m_vsdTree->m_active_map.end())
            {
                bbi->second->fill_element_ptrs(h->m_list);
            }
            else
            {
                printf("can't find branch %s\n", cname.c_str());
            }
        }

        set_event_info();
   }

    virtual Long64_t GetNumEvents() { return m_vsdTree->n_events(); }
};


void testevd()
{
   g_provider = new TestProvider("vsd-nano.root");
   gROOT->LoadMacro("evd.h");
   gROOT->ProcessLine("evd()");
}
