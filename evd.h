#include "VSDBase.h"
#include "VSDProxies.h"

#include "ROOT/REveDataCollection.hxx"
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"
#include "ROOT/REveManager.hxx"
#include "ROOT/REveScalableStraightLineSet.hxx"
//#include "ROOT/REveViewContext.hxx"
#include <ROOT/REveGeoShape.hxx>
// #include <ROOT/REveJetCone.hxx>
#include <ROOT/REvePointSet.hxx>
#include <ROOT/REveProjectionBases.hxx>
#include <ROOT/REveProjectionManager.hxx>
#include <ROOT/REveScene.hxx>
#include <ROOT/REveTableProxyBuilder.hxx>
#include <ROOT/REveTableInfo.hxx>
#include <ROOT/REveTrack.hxx>
#include <ROOT/REveTrackPropagator.hxx>
#include <ROOT/REveViewer.hxx>
#include <ROOT/REveViewContext.hxx>
#include <ROOT/REveDataCollection.hxx>
#include <ROOT/REveSelection.hxx>
#include <ROOT/REveManager.hxx>

// #include "TTree.h"
#include "TGeoTube.h"
// #include "TList.h"
#include "TParticle.h"
//#include "TRandom.h"
#include "TApplication.h"
#include "TMathBase.h"
#include "TMath.h"
#include "TClass.h"
#include "TGeoBBox.h"
#include "TEnv.h"
using namespace ROOT::Experimental;

// globals
ROOT::Experimental::REveManager* eveMng;
ROOT::Experimental::REveProjectionManager* mngRhoZ;
ROOT::Experimental::REveProjectionManager* mngRPhi;
ROOT::Experimental::REveViewContext* viewContext;
ROOT::Experimental::REveTrackPropagator* muonPropagator_g;

//==============================================================================
//== Selection =================================================================
//==============================================================================

class FWSelectionDeviator : public REveSelection::Deviator {
public:
   FWSelectionDeviator() {}

   using REveSelection::Deviator::DeviateSelection;
   bool DeviateSelection(REveSelection *selection, REveElement *el, bool multi, bool secondary,
                         const std::set<int> &secondary_idcs)
   {
      if (el) {
         auto *colItems = dynamic_cast<REveDataItemList *>(el);
         if (colItems) {
            // std::cout << "Deviate RefSelected=" << colItems->RefSelectedSet().size() << " passed set " << secondary_idcs.size() << "\n";
            ExecuteNewElementPicked(selection, colItems, multi, true, colItems->RefSelectedSet());
            return true;
         }
      }
      return false;
   }
};

class InvMassDialog : public REveElement
{
   public:
   void Calculate()
   {
      fTitle = "<pre>";
      printf("FWWebInvMassDialog::calculate() .... \n");
      double sum_len = 0;
      double sum_len_xy = 0;
      int n = 0;
      REveVector first, second, sum;
      addLine("");
      addLine("--------------------------------------------------+--------------");
      addLine("       px          py          pz          pT     | Collection");
      addLine("--------------------------------------------------+--------------");

      TClass *rc_class = TClass::GetClass(typeid(VSDCandidate));
      auto s = ROOT::Experimental::gEve->GetScenes()->FindChild("Collections");
      for (auto &c : s->RefChildren())
      {
         REveDataCollection *coll = (REveDataCollection *)(c);
         auto items = coll->GetItemList();
         for (auto &au : items->RefAunts())
         {
            if (au == ROOT::Experimental::gEve->GetSelection())
            {
                std::cout << c->GetName() << " " << items->GetImpliedSelected() << " --- " << items->RefSelectedSet().size() << "\n";
                for (auto &ss : items->RefSelectedSet())
                {
                    TString line;
                    TClass *model_class = coll->GetItemClass();
                    // std::cout << "item with type " << model_class->GetName() << "\n";
                    void *model_data = const_cast<void *>(coll->GetDataPtr(ss));
                    REveVector v;

                    VSDCandidate *rc = reinterpret_cast<VSDCandidate *>(model_class->DynamicCast(rc_class, model_data));

                    float theta = EtaToTheta(rc->eta());
                    float pz = rc->pt() / TMath::Tan(theta);
                    float px = rc->pt() * TMath::Cos(rc->phi());
                    float py = rc->pt() * TMath::Sin(rc->phi());

                    if (rc != nullptr)
                    {
                        v.Set(px, py, pz);
                        rc->dump();

                        sum += v;
                        sum_len += TMath::Sqrt(v.Mag2());
                        sum_len_xy += TMath::Sqrt(v.Perp2());

                        line = TString::Format("  %+10.3f  %+10.3f  %+10.3f  %10.3f", v.fX, v.fY, v.fZ, TMath::Sqrt(v.Perp2()));
                    }
                    else
                    {
                        line = TString::Format("  -------- not a VSDCandidate --------");
                    }
                    line += TString::Format("  | %s[%d]", coll->GetCName(), ss);

                    addLine(line);

                    if (n == 0)
                        first = v;
                    else if (n == 1)
                        second = v;
                }

                break;
            }
         }
      }

      addLine("--------------------------------------------------+--------------");
      addLine(TString::Format(
          "  %+10.3f  %+10.3f  %+10.3f  %10.3f  | Sum", sum.fX, sum.fY, sum.fZ, TMath::Sqrt(sum.Perp2())));
      addLine("");
      addLine(TString::Format("m  = %10.3f", TMath::Sqrt(TMath::Max(0.0, sum_len * sum_len - sum.Mag2()))));
      addLine(TString::Format("mT = %10.3f", TMath::Sqrt(TMath::Max(0.0, sum_len_xy * sum_len_xy - sum.Perp2()))));
      addLine(TString::Format("HT = %10.3f", sum_len_xy));

      if (n == 2)
      {
        // addLine(TString::Format("deltaPhi  = %+6.4f", deltaPhi(first.Phi(), second.Phi()))); //AMT deltaPhi exisiting only in data formats
         addLine(TString::Format("deltaEta  = %+6.4f", first.Eta() - second.Eta()));
        // addLine(TString::Format("deltaR    = % 6.4f", deltaR(first.Eta(), first.Phi(), second.Eta(), second.Phi())));
      }

      fTitle += "</pre>";
      StampObjProps();
   }

   void
   addLine(const TString &line)
   {
      fTitle += "\n";
      fTitle += line.Data();
   }
   int WriteCoreJson(nlohmann::json &j, int rnr_offset) override
   {
     int ret = REveElement::WriteCoreJson(j, rnr_offset);
   
      std::cout << j.dump(4);
      j["UT_PostStream"] = "UT_refresh_invmass_dialog";
      return ret;
   }
};
//==============================================================================
//== Collection Manager =============================================================
//==============================================================================

class CollectionManager
{
private:
    VSDProvider *m_event{nullptr};
    REveElement *m_collections{nullptr};

    std::vector<REveDataProxyBuilderBase *> m_builders;

    bool m_isEventLoading{false}; // don't process model changes when applying filter on new event
public:
    CollectionManager(VSDProvider *event) : m_event(event)
    {
        m_collections = eveMng->GetScenes()->FindChild("Collections");
    }

    void LoadCurrentEventInCollection(REveDataCollection *rdc)
    {
        m_isEventLoading = true;
        rdc->ClearItems();
        VSDCollection *vsdc = m_event->RefColl(rdc->GetName());
        std::string cname = rdc->GetName();
        printf("-------- LoadCurrentEventInCollection %s size %lu\n", rdc->GetCName(), vsdc->m_list.size());
        std::string t = "dummy";
        for (auto vsd : vsdc->m_list)
        {
            // printf ("add item tp REveColl\n"); vsd->dump();
            rdc->AddItem(vsd, t, t);
        }
        rdc->ApplyFilter();
        rdc->GetItemList()->StampObjProps();

        m_isEventLoading = false;
    }

    void RenewEvent()
    {
        for (auto &el : m_collections->RefChildren())
        {
            auto c = dynamic_cast<REveDataCollection *>(el);
            LoadCurrentEventInCollection(c);
        }

        for (auto proxy : m_builders)
        {
            proxy->Build();
        }
    }

    REveDataProxyBuilderBase *getProxyBuilderFromVSD(VSDCollection *vsdc)
    {
        if (vsdc->m_purpose == "Candidate")
            return new CandidateProxyBuilder();
        else if (vsdc->m_purpose == "Jet")
            return new JetProxyBuilder();
        else if (vsdc->m_purpose == "MET")
            return new METProxyBuilder();
        else if (vsdc->m_purpose == "Muon")
            return new MuonProxyBuilder();
        else if (vsdc->m_purpose == "Vertex")
            return new VertexProxyBuilder();

        std::cout << typeid(vsdc).name() << '\n';

        // amt alternative way
        // std::string pbn = vsdc->m_purpose + "ProxyBuilder";
        // TClass* pbc = TClass::GetClass(pbn.c_str());

        printf("can't find proxy for purpose %s \n", vsdc->m_purpose.c_str());
        return nullptr;
    }

    void
    addCollection(VSDCollection *vsdc)
    {
        REveDataCollection *collection = new REveDataCollection(vsdc->m_name);
        m_collections->AddElement(collection);
        std::string class_name = "VSD" + vsdc->m_name; // !!! This works beacuse it is a root macro

        std::cout << "calss name " << class_name << "\n";

        TClass* tc  = TClass::GetClass(class_name.c_str());
        if (!tc) {
            class_name = "VSD" + vsdc->m_purpose; // !!! This works beacuse it is a root macro
            tc  = TClass::GetClass(class_name.c_str());
            if (!tc)
            throw( std::runtime_error("addCollection " +  vsdc->m_name ) );
        }


        collection->SetItemClass(TClass::GetClass(class_name.c_str()));
        collection->SetMainColor(vsdc->m_color);
        collection->SetFilterExpr(vsdc->m_filter.c_str());

        REveDataProxyBuilderBase *glBuilder = getProxyBuilderFromVSD(vsdc);

        // load data
        LoadCurrentEventInCollection(collection);
        glBuilder->SetCollection(collection);
        glBuilder->SetHaveAWindow(true);

        for (auto &scene : eveMng->GetScenes()->RefChildren())
        {

            // REveElement *product = glBuilder->CreateProduct(scene->GetTitle(), viewContext);

            if (strncmp(scene->GetCName(), "Table", 5) == 0)
                continue;
            if (!strncmp(scene->GetCTitle(), "RhoZProjected", 8))
            {
                REveElement *product = glBuilder->CreateProduct("RhoZViewType", viewContext);
                mngRhoZ->ImportElements(product, scene);
                continue;
            }
            if (!strncmp(scene->GetCTitle(), "RPhiProjected", 8))
            {
                REveElement *product = glBuilder->CreateProduct("RPhiViewType", viewContext);
                mngRPhi->ImportElements(product, scene);
                continue;
            }
            else if ((!strncmp(scene->GetCName(), "Event scene", 8)))
            {
                REveElement *product = glBuilder->CreateProduct(scene->GetTitle(), viewContext);
                scene->AddElement(product);
            }
        }
        m_builders.push_back(glBuilder);
        glBuilder->Build();

        // Tables
        auto tableBuilder = new REveTableProxyBuilder();
        tableBuilder->SetHaveAWindow(true);
        tableBuilder->SetCollection(collection);
        REveElement *tablep = tableBuilder->CreateProduct("table-type", viewContext);
        auto tableMng = viewContext->GetTableViewInfo();
        if (collection == m_collections->FirstChild())
        {
            tableMng->SetDisplayedCollection(collection->GetElementId());
        }

        for (auto &scene : eveMng->GetScenes()->RefChildren())
        {
            if (strncmp(scene->GetCTitle(), "Table", 5) == 0)
            {
                scene->AddElement(tablep);
                // tableBuilder->Build(rdc, tablep, viewContext );
                break;
            }
        }
        tableMng->AddDelegate([=]()
                              { tableBuilder->ConfigChanged(); });
        m_builders.push_back(tableBuilder);

        // set tooltip expression for items

        auto tableEntries = tableMng->RefTableEntries(collection->GetItemClass()->GetName());
        int N = TMath::Min(int(tableEntries.size()), 3);
        for (int t = 0; t < N; t++)
        {
            auto te = tableEntries[t];
            collection->GetItemList()->AddTooltipExpression(te.fName, te.fExpression);
        }

        collection->GetItemList()->SetItemsChangeDelegate([&](REveDataItemList *collection, const REveDataCollection::Ids_t &ids)
                                                          { this->ModelChanged(collection, ids); });
        collection->GetItemList()->SetFillImpliedSelectedDelegate([&](REveDataItemList *collection, REveElement::Set_t &impSelSet, const std::set<int> &sec_idcs)
                                                                  { this->FillImpliedSelected(collection, impSelSet, sec_idcs); });
    }

    void ModelChanged(REveDataItemList *itemList, const REveDataCollection::Ids_t &ids)
    {
        if (m_isEventLoading)
            return;

        for (auto proxy : m_builders)
        {
            if (proxy->Collection()->GetItemList() == itemList)
            {
                printf("Model changes check proxy %s: \n", proxy->Collection()->GetCName());
                proxy->ModelChanges(ids);
            }
        }
    }

    void FillImpliedSelected(REveDataItemList *itemList, REveElement::Set_t &impSelSet, const std::set<int> &sec_idcs)
    {

        for (auto proxy : m_builders)
        {
            if (proxy->Collection()->GetItemList() == itemList)
            {
                proxy->FillImpliedSelected(impSelSet, sec_idcs);
            }
        }
    }
};

//==============================================================================
//== Event Manager =============================================================
//==============================================================================

class EventManager : public REveElement
{
private:
   CollectionManager    *m_collectionMng{nullptr};
   VSDProvider          *m_event{nullptr};
   std::chrono::duration<double> m_deltaTime{1};
   std::thread *m_timerThread{nullptr};
   std::mutex m_mutex;
   std::condition_variable m_CV;
   bool m_autoplay{false};
   

public:
   EventManager(CollectionManager* m, VSDProvider* e):REveElement("EventManager") {m_collectionMng  = m; m_event = e; m_deltaTime = std::chrono::milliseconds(500);}
   virtual ~EventManager() {}

   virtual void GotoEvent(int id)  
   {
      m_event->GotoEvent(id);
      UpdateTitle();
      m_collectionMng->RenewEvent();
   }


  void UpdateTitle()
   {
      printf("======= update title %lld/%lld event ifnfo run=[%d], lumi=[%d], event = [%lld]\n", m_event->m_eventIdx, m_event->GetNumEvents(),
             m_event->m_eventInfo.m_lumi, m_event->m_eventInfo.m_run, m_event->m_eventInfo.m_event);
      SetTitle(Form("%lld/%lld/%d/%d/%lld",m_event->m_eventIdx, m_event->GetNumEvents(), m_event->m_eventInfo.m_lumi , m_event->m_eventInfo.m_run,  m_event->m_eventInfo.m_event));
      StampObjProps();
   }
   virtual void NextEvent()
   {
      int id = m_event->m_eventIdx +1;
      if (id ==  m_event->GetNumEvents()) {
         printf("NextEvent: reached last %lld\n", m_event->GetNumEvents());
         id = 0;
      }
      GotoEvent(id);
   }

   virtual void PreviousEvent()
   {
      int id;
      if (m_event->m_eventIdx == 0)
      {
         id = m_event->GetNumEvents() - 1;
      }
      else
      {
         id = m_event->m_eventIdx - 1;
      }

      printf("going to previous %d \n", id);
      GotoEvent(id);
   }

   void autoplay_scheduler()
   {
      while (true)
      {
         bool autoplay;
         {
                std::unique_lock<std::mutex> lock{m_mutex};
                if (!m_autoplay)
                {
                // printf("exit thread pre wait\n");
                return;
                }
                if (m_CV.wait_for(lock, m_deltaTime) != std::cv_status::timeout)
                {
                printf("autoplay not timed out \n");
                if (!m_autoplay)
                {
                    printf("exit thread post wait\n");
                    return;
                }
                else
                {
                    continue;
                }
                }
                autoplay = m_autoplay;
         }
         if (autoplay)
         {
                REveManager::ChangeGuard ch;
                NextEvent();
         }
         else
         {
                return;
         }
      }
   }

   void autoplay(bool x)
   {
      std::cout << "Set autoplay " << x << std::endl;
      static std::mutex autoplay_mutex;
      std::unique_lock<std::mutex> aplock{autoplay_mutex};
      {
         std::unique_lock<std::mutex> lock{m_mutex};

         StampObjProps();
         m_autoplay = x;
         if (m_autoplay)
         {
                if (m_timerThread)
                {
                m_timerThread->join();
                delete m_timerThread;
                m_timerThread = nullptr;
                }
                NextEvent();
                m_timerThread = new std::thread{[this]
                                                { autoplay_scheduler(); }};
         }
         else
         {
                m_CV.notify_all();
         }
      }
   }

   void playdelay(float x)
   {
      printf("playdelay %f\n", x);
      std::unique_lock<std::mutex> lock{m_mutex};
      m_deltaTime = std::chrono::milliseconds(int(x));
      StampObjProps();
      m_CV.notify_all();
   }
};

//==============================================================================
//== init scenes and views  =============================================================
//==============================================================================
void createScenesAndViews()
{
   //view context
   float r = 139.5;
   float z = 290;
   auto prop = new REveTrackPropagator();
   prop->SetMagFieldObj(new REveMagFieldDuo(350, -3.5, 2.0));
   prop->SetMaxR(r);
   prop->SetMaxZ(z);
   prop->SetMaxOrbs(6);
   prop->IncRefCount();



   viewContext = new REveViewContext();
   viewContext->SetBarrel(r, z);
   viewContext->SetTrackPropagator(prop);

   // table specs
   auto tableInfo = new REveTableViewInfo();
   tableInfo->table("VSDVertex").
      column("x",  1, "i.x()").
      column("y",  1, "i.y()").
      column("z",  1, "i.z()");

   tableInfo->table("VSDCandidate").
      column("pt",  1, "i.pt()").
      column("eta", 3, "i.eta()").
      column("phi", 3, "i.phi()").
      column("charge", 3, "i.charge()");

   tableInfo->table("VSDElectron").
      column("pt",  1, "i.pt()").
      column("eta", 3, "i.eta()").
      column("phi", 3, "i.phi()").
      column("HoE", 3, "i.hadronicOverEm()");

   tableInfo->table("VSDMET").
      column("pt",  1, "i.pt()").
      column("sumEt", 3, "i.sumEt()").
      column("phi", 3, "i.phi()");

   tableInfo->table("VSDJet").
      column("pt",  1, "i.pt()").
      column("eta", 3, "i.eta()").
      column("phi", 3, "i.phi()").
      column("hadFraction", 3, "i.hadFraction()");

   viewContext->SetTableViewInfo(tableInfo);

   // Geom  ry
   auto b1 = new REveGeoShape("Barrel 1");
   b1->SetShape(new TGeoTube(r -2 , r+2, z));
   b1->SetMainColor(kCyan);
   eveMng->GetGlobalScene()->AddElement(b1);

   // Projected RPhi
   if (1)
   {
      auto rPhiEventScene = eveMng->SpawnNewScene("RPhi Scene", "RPhiProjected");
      mngRPhi = new REveProjectionManager(REveProjection::kPT_RPhi);
      mngRPhi->SetImportEmpty(true);
      auto rPhiView = eveMng->SpawnNewViewer("RPhi View");
      rPhiView->SetCameraType(REveViewer::kCameraOrthoXOY);
      rPhiView->AddScene(rPhiEventScene);

      auto pgeoScene = eveMng->SpawnNewScene("Projection Geometry");
      mngRPhi->ImportElements(b1,pgeoScene );
      rPhiView->AddScene(pgeoScene);
   }
   // Projected RhoZ
   if (1)
   {
      auto rhoZEventScene = eveMng->SpawnNewScene("RhoZ Scene", "RhoZProjected");
      mngRhoZ = new REveProjectionManager(REveProjection::kPT_RhoZ);
      mngRhoZ->SetImportEmpty(true);
      auto rhoZView = eveMng->SpawnNewViewer("RhoZ View");
      rhoZView->SetCameraType(REveViewer::kCameraOrthoXOY);
      rhoZView->AddScene(rhoZEventScene);

      auto pgeoScene = eveMng->SpawnNewScene("Projection Geometry");
      mngRhoZ->ImportElements(b1,pgeoScene );
      rhoZView->AddScene(pgeoScene);
   }
      // collections
   eveMng->SpawnNewScene("Collections", "Collections");

   // Table
   if (1) {
      auto tableScene = eveMng->SpawnNewScene ("Tables", "Tables");
      auto tableView  = eveMng->SpawnNewViewer("Table", "Table View");
      tableView->AddScene(tableScene);
      tableScene->AddElement(viewContext->GetTableViewInfo());
   }

   // ((REveViewer*)(eveMng->GetViewers()->FirstChild()))->SetMandatory(false);
}
////////////////////////////////////////////////////
////////////////////////////////////////////////////
void evd()
{
   VSDProvider* prov = g_provider;
   eveMng = REveManager::Create();

   createScenesAndViews();
   auto collectionMng = new CollectionManager(prov);

   auto eventMng = new EventManager(collectionMng, prov);
   eventMng->UpdateTitle();
   // eventMng->SetName(prov->GetFile()->GetName());

  TString name = prov->m_title;
  int l = name.Last('/');
  if (l != kNPOS)
      name.Remove(0, l + 1);

  eventMng->SetName(name.Data());
  auto massDialog = new InvMassDialog();
  eventMng->AddElement(massDialog);
  eveMng->GetWorld()->AddElement(eventMng);

  auto deviator = std::make_shared<FWSelectionDeviator>();
  eveMng->GetSelection()->SetDeviator(deviator);
  eveMng->GetHighlight()->SetDeviator(deviator);
  for (auto &vsdc : prov->m_collections)
  {
      printf("vsd collection ====== %s\n", vsdc->m_name.c_str());
      if (vsdc->m_purpose == "EventInfo")
         continue;
      collectionMng->addCollection(vsdc);
   }
   eventMng->GotoEvent(0);

   std::string locPath = "ui5";
   eveMng->AddLocation("mydir/", locPath);
   eveMng->SetDefaultHtmlPage("file:mydir/eventDisplay.html");
   ((REveViewer*)(ROOT::Experimental::gEve->GetViewers()->FirstChild()))->SetMandatory(false);

   gEnv->SetValue("WebEve.DisableShow", 1);
   // gEnv->SetValue("WebGui.HttpMaxAge", 0);
   eveMng->Show();
}
