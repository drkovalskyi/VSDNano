#include "VsdBase.h"
#include "VsdTree.h"
#include "TClass.h"
#include "TVirtualCollectionProxy.h"
#include "TBranchElement.h"
#include "TPRegexp.h"
#include "nlohmann/json.hpp"


VsdProvider *g_provider = nullptr;
VsdTree* g_vsdTree = nullptr; // temporary, but piggish AMT!!!

struct ColBranchInfo
{
    TBranchElement *m_branch;
    TClass *m_cclass, *m_eclass;
    TVirtualCollectionProxy *m_proxy;
    long long m_base_offset;
    char *m_collection;
    std::string m_name;

    VsdBase *vsd_base_at(int i)
    {
        return (VsdBase *)((void *)((long long)m_proxy->At(i) + m_base_offset));
    }

    ColBranchInfo() = default;
};

class VsdProvider
{
public:
    TTree *m_vsdTree{nullptr};
    VsdEventInfo m_eventInfo;

    Long64_t m_eventIdx{0};
    std::vector<VsdCollection *> m_collections;
    std::string m_title{"VsdProvider"};

    // New!
    std::map<std::string, ColBranchInfo> cmap;
    TClass *vsdbase_class = TClass::GetClass<VsdBase>();

    virtual ~VsdProvider() {}

    // construcutor
    VsdProvider(std::string fn)
    {
        TFile *f = TFile::Open(fn.c_str(), "READ");
        TTree *tree = (TTree *)f->Get("VSD");
        printf("\n--- Starting branch processing ...\n");

        TPMERegexp re("^(?:std::)?vector<(Vsd[\\w\\d]+)>$");
        int i = 0;
        TIter bi(tree->GetListOfBranches());
        while (TBranch *br = (TBranch *)bi())
        {
            TBranchElement *bre = dynamic_cast<TBranchElement *>(br);
            printf("  %2d. name='%s' class='%s' ptr=%p mother_ptr=%p branch_class='%s' branch_element_ptr=%p\n",
                   ++i, br->GetName(), br->GetClassName(), br, br->GetMother(), br->ClassName(), bre);

            if (bre && re.Match(br->GetClassName()))
            {
                printf("      matched type '%s' ... whole = '%s'\n",
                       re[1].Data(), re[0].Data());

                bre->GetEntry(0);
                // printf(" get entry(0) -> address=%p object=%p -- VsdBase offset=%d\n",
                //       bre->GetAddress(), bre->GetObject());

                TClass *cc = TClass::GetClass(br->GetClassName());
                TVirtualCollectionProxy *cp = cc->GetCollectionProxy()->Generate();
                cp->PushProxy(bre->GetObject());

                TClass *ec = TClass::GetClass(re[1].Data());
                void *ooo = ec->New();
                long long off = ec->GetBaseClassOffset(vsdbase_class, ooo);
                ec->Destructor(ooo);

                cmap.insert({br->GetName(), {bre, cc, ec, cp, off, bre->GetObject(), br->GetName()}});

                printf("  post get entry 0 %s %u\n",
                       br->GetName(), cp->Size());

                std::string colType = re[1].Data();
                VsdCollection* vc = new VsdCollection(br->GetName(), colType.substr(3));
                addCollection(vc);
                try
                {
                    nlohmann::json data = nlohmann::json::parse(br->GetTitle());
                    // std::cout << data.dump(3) << "\n";
                    for (auto &el : data.items())
                    {
                        // std::cout << "key: " << el.key() << ", value:" << el.value() << '\n';
                        if (el.key() == "filter") {
                            vc->m_filter = el.value();
                        }
                        if (el.key() == "color") {
                            vc->m_color = el.value();
                        }
                        if (el.key() == "purpose") {
                            vc->m_purpose = el.value();
                        }
                    }
                }
                catch (nlohmann::json::parse_error &ex)
                {
                    std::cerr << "parse error  "<< br->GetTitle() << " at byte " << ex.byte << std::endl;
                }
            }
        }

        m_vsdTree = tree; // Old
    }

    void addCollection(VsdCollection *h)
    {
        m_collections.push_back(h);

        if (h->m_purpose == "Jet")
            h->m_color = kYellow;
        if (h->m_purpose == "MET")
            h->m_color = kRed;
        if (h->m_purpose == "Muon")
            h->m_color = kRed;
        if (h->m_purpose == "Vertex")
            h->m_color = kBlack;
        if (h->m_purpose == "Candidate")
            h->m_color = kGreen;
    }

    virtual void set_event_info()
    {
        printf("vsd provider %lld events total %lld !!!!! \n", m_eventIdx, GetNumEvents());
        m_eventInfo = VsdEventInfo(9999, 9999, m_eventIdx);

        for (auto &vsdc : m_collections)
        {
            if (vsdc->m_purpose == "EventInfo")
            {
                if (vsdc->m_list.empty())
                {
                    printf("empty event info !\n");
                    return;
                }
                VsdEventInfo *ei = dynamic_cast<VsdEventInfo *>(vsdc->m_list[0]);
                m_eventInfo = *ei;
                // printf("...... setting event info %lld \n", m_eventInfo.event());
                return;
            }
        }

        // just set event id if other information is not available
        m_eventInfo = VsdEventInfo(9999, 9999, m_eventIdx);
    }

    VsdCollection *RefColl(const std::string &name)
    {
        for (auto collection : m_collections)
        {
            if (name == collection->m_name)
                return collection;
        }
        return nullptr;
    }

    void GotoEvent(int eventIdx)
    {
        m_eventIdx = eventIdx;

        m_vsdTree->GetEntry(eventIdx);

        for (auto &&[name, cbi] : cmap)
        {
            printf("  Trying to read %s\n", name.c_str());
            printf("    pre get branch entry size = %u\n", cbi.m_proxy->Size());
            cbi.m_branch->GetEntry(eventIdx);
            printf("    size = %u\n", cbi.m_proxy->Size());

            VsdCollection *vc = RefColl(name);
            vc->m_list.clear();
            int ss = cbi.m_proxy->Size();
            for (int i = 0; i < ss; ++i)
            {
                // cbi.vsd_base_at(i)->dump();
                vc->m_list.push_back(cbi.vsd_base_at(i));
            }
        }
        set_event_info();
    }

    virtual Long64_t GetNumEvents() { return m_vsdTree->GetEntriesFast(); }
};
