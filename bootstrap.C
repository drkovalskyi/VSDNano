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
      printf ("foto provier 1\n");
      m_tree->GetEntry(eventIdx);
      printf ("foto provier 2\n");
      VSDProvider::GotoEvent(eventIdx);
   }

   virtual Long64_t GetNumEvents() { return m_tree->GetEntriesFast(); }

   void registerCollection(nlohmann::json& j, const std::string& name, Color_t color, const std::string& filter)
   {
      // std::cout << "\n\n\n\nregister collection " << name << " " << j.dump(3) << "\\n\n";
      std::string purpose = j["purpose"];
      std::string desc = name;
      std::string vsdClassType = purpose; // AMT currently the same

      nlohmann::json f = j["fields"];
      
      try
      {
         using namespace nlohmann;
         TString fillFunc = "  virtual void fill() {\n VSDReader &i = *m_data;\n";
         // std::cout << j.dump(4) << "\n";
         std::string numKey = "single";
         try
         {
            numKey = j["size"];
         }
         catch (std::exception &e)
         {
            std::cout << "missing collection size info " << e.what() << "\n";
         }

         // single element collection
         if (numKey == "single")
         {
            fillFunc += TString::Format("auto vsdObj = new VSD%s();\n", vsdClassType.c_str());
            for (json::iterator it = f.begin(); it != f.end(); ++it)
            {
               std::string k = it.key(), v = it.value();
               fillFunc += TString::Format("vsdObj->m_%s = %s;\n", k.c_str(), v.c_str());
            }
            fillFunc += "m_list.push_back(vsdObj);\n";
         }
         else // create from array
         {
            fillFunc += TString::Format("for (int a = 0; a < %s; ++a) {\n", numKey.c_str());
            fillFunc += TString::Format("auto vsdObj = new VSD%s();\n", vsdClassType.c_str());
            for (json::iterator it = f.begin(); it != f.end(); ++it)
            {
               std::string k = it.key(), v = it.value();
               fillFunc += TString::Format("vsdObj->m_%s = %s[a]; \n",k.c_str(), v.c_str());
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
         exp += "};\n";

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

void bootstrap(TFile *file, nlohmann::json *cList)
{
   auto tree = (TTree *)file->Get("Events");
   NanoProvider *provider = new NanoProvider(tree, cList);
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

      try
      {
         provider->registerCollection(col, it.key(), color, filter);
      }
      catch (std::exception &e)
      {
         std::cout << "RegisterCollection caught exception " << e.what() << "\n";
      }
   }
}
