#include "VSDBase.h"
#include "nlohmann/json.hpp"

class NanoCollection: public VSDCollection
{ public:
  NanoCollection(const std::string& n,const std::string& p) : VSDCollection(n,p) {}
   VSDReader* m_data{nullptr};
};

class NanoProvider : public VSDProvider
{
   public:
   TTree *m_tree{nullptr};

   VSDReader* m_data{nullptr};

   NanoProvider(TTree *t, nlohmann::json* cfg) :  m_tree(t)
   {
      m_config = cfg;
      m_data = new VSDReader(t);
   }

   virtual void GotoEvent(int eventIdx)
   {
      m_tree->GetEntry(eventIdx);
      VSDProvider::GotoEvent(eventIdx);
   }

   virtual Long64_t GetNumEvents() { return m_tree->GetEntriesFast(); }

   void registerCollection(const std::string &desc, const std::string &vsdClassType, Color_t color = kBlue, std::string filter = "")
   {
      try
      {
         using namespace nlohmann;
         TString fillFunc = "  virtual void fill() {\n VSDReader &r = *m_data;\n";
         json &j = m_config->at(vsdClassType);
         // std::cout << j.dump(4) << "\n";
         std::string numKey;
         try
         {
            numKey = j["N"];
         }
         catch (std::exception &e)
         {
            std::cout << "missing collection size info " << e.what() << "\n";
         }

         // single element collection
         if (numKey == "undef")
         {
            fillFunc += TString::Format("auto vsdObj = new VSD%s();\n", vsdClassType.c_str());
            for (json::iterator it = j.begin(); it != j.end(); ++it)
            {
               if (it.key() == "N")
                  continue;

               std::string k = it.key(), v = it.value();
               fillFunc += TString::Format("vsdObj->m_%s = r.%s%s;\n", k.c_str(), desc.c_str(), v.c_str());
            }
            fillFunc += "m_list.push_back(vsdObj);\n";
         }
         else // create from array
         {
            fillFunc += TString::Format("for (int i = 0; i < r.%s%s; ++i) {\n", numKey.c_str(), desc.c_str());
            fillFunc += TString::Format("auto vsdObj = new VSD%s();\n", vsdClassType.c_str());

            for (json::iterator it = j.begin(); it != j.end(); ++it)
            {
               if (it.key() == "N")
                  continue;

               std::string k = it.key(), v = it.value();
               fillFunc += TString::Format("vsdObj->m_%s = r.%s%s[i]; \n",k.c_str(), desc.c_str(), v.c_str());
            }
            fillFunc += "m_list.push_back(vsdObj);\n";
            fillFunc += "}\n // end loop through vsd array";
         }
         fillFunc += "\n}\n"; // end fill body function
         // printf("VSD collection fill body  %s \n ", fillFunc.Data());

         // make sources for class
         TString cname = TString::Format("%s%sCollection", desc.c_str(), vsdClassType.c_str());
         TString exp = TString::Format("class %s : public NanoCollection { \n", cname.Data());
         exp += "public:\n";
         exp += TString::Format("%s(const std::string &n, const std::string &p) : NanoCollection(n, p) {}\n", cname.Data());
         exp += fillFunc;
         exp += "};\n\n";

         exp += TString::Format("g_provider->addCollection(new %s(\"%s\", \"%s\"));\n", cname.Data(), desc.c_str(), vsdClassType.c_str());
         // std::cout << "Expression to evaluate\n\n" << exp << "\n";
                   
         gROOT->ProcessLine(exp.Data());

         // config collection
         NanoCollection *coll = dynamic_cast<NanoCollection *>(RefColl(desc));
         coll->m_color = color;
         coll->m_filter = filter;
         coll->m_name = desc;
         coll->m_data = m_data;
      }
      catch (std::exception &e)
      {
         std::cerr << e.what() << "\n";
      }
   }
};

void cms_nano_aod_bootstrap(TFile *file, nlohmann::json *mconfig, nlohmann::json *cList)
{
   auto tree = (TTree *)file->Get("Events");
   NanoProvider *provider = new NanoProvider(tree, mconfig);
   provider->m_title = file->GetName();
   g_provider = provider;
   for (nlohmann::json::iterator it = cList->begin(); it != cList->end(); ++it)
   {
      // optional color
      nlohmann::json col = *it;
      Color_t color = kBlue;
      try
      {
         color = col.at("color");
      }
      catch (std::exception &e)
      {
      }
      // optional filter
      std::string filter = "";
      try
      {
         filter = col.at("filter");
      }
      catch (std::exception &e)
      {
      }
      provider->registerCollection(col["prefix"], col["type"], color, filter);
   }
}
