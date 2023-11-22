#include "VsdBase.h"
#include "VsdTree.h"

VsdProvider *g_provider = nullptr;
VsdTree* g_vsdTree = nullptr; // temporary, but piggish

class TestProvider : public VsdProvider
{
public:
    VsdTree* m_vsdTree{nullptr};

    // construcutor
    TestProvider(std::string fn)
    {
        TFile* f = TFile::Open(fn.c_str(), "READ");
        TTree* tt = (TTree*) f->Get("VSD");

        std::string cname = "VsdTree";

        TObjString *cc = (TObjString*) gFile->Get("vsd_tree_code");
        std::string scc = cc->String().Data();
        std::regex re("class (.*): public VsdTree");
        std::smatch m;
        std::regex_search(scc, m, re);
        if (m.size() > 1) {
            cname = m[1].str();
        }
        else
        {  
            throw std::runtime_error("Can't guess the tree class");
        }

        TString cmd = TString::Format("\n g_vsdTree = new %s((TTree*) gFile->Get(\"VSD\"));\n", cname.c_str());
        cc->String().Append(cmd);
        gROOT->ProcessLine(cc->GetString());
        m_vsdTree = g_vsdTree;
   
        m_vsdTree->goto_event(0);
        for (auto &s : m_vsdTree->m_supported_vector)
        {
            // printf("branch name %s trype %s colType %s \n", s->m_name.c_str(), s->m_type.c_str(), s->m_collection_type.c_str());
            std::string name =  s->m_name;
            std::string purpose = s->m_type.substr(3);
            VsdCollection* c = new VsdCollection(name, purpose);
            c->m_type =  s->m_type;
            addCollection(c);
        }
    }

    virtual void GotoEvent(int eventIdx)
    {
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
    try {
        g_provider = new TestProvider("vsd-nano.root");
        gROOT->LoadMacro("evd.h");
        gROOT->ProcessLine("evd()");
    }
    catch (const std::exception& e)
    {
        std::cout << "failed to run event display: " << e.what();
    }
}
