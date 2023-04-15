
class fwVSDBase
{
public:
   virtual void dump() { printf("dump VSD Base class\n"); }
};

class fwVSDVertex : public fwVSDBase
{
public:
   ROOT::Math::Polar3DPoint location;
   void dump() { printf("VSDVertex x:%.2f, y:%.2f, z:%.2f \n", location.X(), location.Y(), location.Z()); }
};

class fwVSDJet : public fwVSDBase
{
public:
   // ROOT::Math::XYZVector p3;
   float eta{0.f};
   float phi{0.f};
   float pt{0.f};
   float em_fraction{0.f};
   float had_fraction{0.f};
   float radius;

   using fwVSDBase::dump;
   void dump() { printf("VSDJet pt:%.2f, eta:%.2f, phi:%.2f \n", pt, eta, phi); }
};

typedef std::vector<fwVSDBase *> fwVSDCollection;
typedef std::vector<fwVSDCollection *> fwVSDEvent;

class VSDProviderBase
{
public:
   // re-formatting
   virtual fwVSDEvent &getEvent(int eventIdx) = 0;
   virtual int getNEvents() = 0;

   // filtering
   virtual void setFilterExpr(const std::string &) = 0;
   bool getFilterEnabled() { return false; }
};

class NanoVSDProvider : public VSDProviderBase
{
private:
   TTree *m_tree{nullptr};
   fwVSDEvent m_VSDEvent;
   fwVSDCollection *m_jetCollection{nullptr};
   fwVSDCollection *m_PVCollection{nullptr};

   struct NanoJetAccess
   {
      TLeaf *nLeaf{nullptr};
      TLeaf *ptLeaf{nullptr};
      TLeaf *etaLeaf{nullptr};
      TLeaf *phiLeaf{nullptr};

      void initLeaves(TTree *tree)
      {
         nLeaf = NanoVSDProvider::FindLeafByName("nJet", tree);
         ptLeaf = NanoVSDProvider::FindLeafByName("Jet_pt", tree);
         etaLeaf = NanoVSDProvider::FindLeafByName("Jet_eta", tree);
         phiLeaf = NanoVSDProvider::FindLeafByName("Jet_phi", tree);
      }

      void produceCollection(fwVSDCollection *collection, int eventIdx)
      {
         auto entries = ((UInt_t *)(nLeaf->GetValuePointer()))[0];

         ptLeaf->GetBranch()->GetEntry(eventIdx);
         etaLeaf->GetBranch()->GetEntry(eventIdx);
         phiLeaf->GetBranch()->GetEntry(eventIdx);
         for (UInt_t i = 0; i < entries; ++i)
         {
            fwVSDJet *jet = new fwVSDJet();
            jet->pt = ((float *)(ptLeaf->GetValuePointer()))[i];
            jet->eta = ((float *)(etaLeaf->GetValuePointer()))[i];
            jet->phi = ((float *)(phiLeaf->GetValuePointer()))[i];
            collection->push_back(jet);
         }
      }
   };

   struct NanoVertexAccess
   {
      // TLeaf *nLeaf{nullptr};
      TLeaf *xLeaf{nullptr};
      TLeaf *yLeaf{nullptr};
      TLeaf *zLeaf{nullptr};

      void initLeaves(TTree *tree)
      {
         // nLeaf = NanoVSDProvider::FindLeafByName("PV_npvs", tree);
         xLeaf = NanoVSDProvider::FindLeafByName("PV_x", tree);
         yLeaf = NanoVSDProvider::FindLeafByName("PV_y", tree);
         zLeaf = NanoVSDProvider::FindLeafByName("PV_z", tree);
      }

      void produceCollection(fwVSDCollection *collection, int eventIdx)
      {
         auto entries = 1; // ((UInt_t *)(nLeaf->GetValuePointer()))[0];

         xLeaf->GetBranch()->GetEntry(eventIdx);
         yLeaf->GetBranch()->GetEntry(eventIdx);
         zLeaf->GetBranch()->GetEntry(eventIdx);
         for (UInt_t i = 0; i < entries; ++i)
         {
            fwVSDVertex *PV = new fwVSDVertex();
            PV->location.SetXYZ(
                ((float *)(xLeaf->GetValuePointer()))[i],
                ((float *)(yLeaf->GetValuePointer()))[i],
                ((float *)(zLeaf->GetValuePointer()))[i]);
            collection->push_back(PV);
         }
      }
   };

   NanoJetAccess m_jetAccess;
   NanoVertexAccess m_PVAccess;

public:
   NanoVSDProvider(const std::string &filePath)
   {
      auto file = TFile::Open(filePath.c_str());
      m_tree = (TTree *)file->Get("Events");

      // register jet collecitons
      m_jetAccess.initLeaves(m_tree);
      m_jetCollection = new fwVSDCollection();
      m_VSDEvent.push_back(m_jetCollection);
      // register vertex collecitons
      m_PVAccess.initLeaves(m_tree);
      m_PVCollection = new fwVSDCollection();
      m_VSDEvent.push_back(m_PVCollection);
   }

   using VSDProviderBase::getNEvents;
   int getNEvents()
   {
      return (int)m_tree->GetEntriesFast();
   }

   using VSDProviderBase::getEvent;
   fwVSDEvent &getEvent(int eventIdx)
   {
      for (auto &c : m_VSDEvent)
         c->clear();
      m_jetAccess.produceCollection(m_jetCollection, eventIdx);
      m_PVAccess.produceCollection(m_PVCollection, eventIdx);
      return m_VSDEvent;
   }

   using VSDProviderBase::setFilterExpr;
   void setFilterExpr(const std::string &) { printf("filtering not implemented"); }

   static TLeaf *FindLeafByName(std::string name, TTree *tree)
   {
      TObjArray *leaves = tree->GetListOfLeaves();
      int n_leaves = leaves ? leaves->GetEntriesFast() : 0;
      for (int l = 0; l < n_leaves; ++l)
      {
         TLeaf *leaf = (TLeaf *)leaves->UncheckedAt(l);
         std::string bn = leaf->GetName();
         if (bn == name)
            return leaf;
      }
      return nullptr;
   }
};

void VSDProvider()
{
   NanoVSDProvider provider("nano.root");

   // printf first few events from VSD structs
   for (int ev = 0; ev < 5; ++ev)
   {
      printf("\n\nprint Event [%d]...................\n", ev);
      fwVSDEvent &vsdEvent = provider.getEvent(ev);
      for (auto &collection : vsdEvent)
      {
         printf("vsd collection ====== \n");
         for (auto &vsd : *collection)
         {
            vsd->dump();
         }
      }
   }
}
