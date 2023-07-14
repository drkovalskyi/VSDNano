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

   void registerCollection(const std::string &desc, const std::string &vsdClassType, Color_t color = kBlue, std::string filter = "")
   {
      try
      {
         using namespace nlohmann;
         TString cmd;
         json &j = m_config->at(vsdClassType);

         std::cout << j.dump(4) << "\n";
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
            cmd += TString::Format("auto vsdObj = new VSD%s();\n", vsdClassType.c_str());
            for (json::iterator it = j.begin(); it != j.end(); ++it)
            {
               if (it.key() == "N")
                  continue;

               std::string k = it.key(), v = it.value();
               cmd += TString::Format("vsdObj->m_%s = r.ZZZ%s;\n", k.c_str(), v.c_str());
            }
            cmd += "m_list.push_back(vsdObj);\n";
         }
         else // create from array
         {
            cmd += TString::Format("for (int i = 0; i < r.%sZZZ; ++i) {\n", numKey.c_str());
            cmd += TString::Format("auto vsdObj = new VSD%s();\n", vsdClassType.c_str());

            for (json::iterator it = j.begin(); it != j.end(); ++it)
            {
               if (it.key() == "N")
                  continue;

               std::string k = it.key(), v = it.value();
               cmd += TString::Format("vsdObj->m_%s = r.ZZZ%s[i]; \n", k.c_str(), v.c_str());
            }
            cmd += "m_list.push_back(vsdObj);\n";
            cmd += "}\n // end loop through vsd array";
         }
         printf("VSD collection fill body  %s \n ", cmd.Data());

         // make sources for class
         std::string cname = desc;
         cname += vsdClassType;
         cname += "Collection";
         std::stringstream ss;
         ss << "class " << cname << " : public NanoCollection \n"
            << "{\n"
            << "public:\n"
            << cname << "(const std::string &n, const std::string &p) : NanoCollection(n, p) {}\n"
            << "  virtual void fill() {\n VSDReader &r = *m_data;"
            << cmd.Data() << "\n}\n"
            << "};\n"
            << "\n"
            << "g_provider->addCollection(new " << cname << "(\"ZZZ\",\"" << vsdClassType << "\"));\n";

       //  std::cout << ss.str() << "\n\n.....\n";
         std::string exp = std::regex_replace(ss.str(), std::regex("ZZZ"), desc);
         std::cout << "Expression to evaluate" << exp << "\n";
         gROOT->ProcessLine(exp.c_str());

         // config collection
         NanoCollection *coll = dynamic_cast<NanoCollection*>(RefColl(desc));
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

void cms_nano_aod_bootstrap(TTree *tree, nlohmann::json *mconfig, nlohmann::json *cList)
{
   NanoProvider *provider = new NanoProvider(tree, mconfig);
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

      printf("----------------------------------------------------------\n");
      provider->registerCollection(col["prefix"], col["type"], color, filter);
      printf("----------------------------------------------------------\n");
   }
}
