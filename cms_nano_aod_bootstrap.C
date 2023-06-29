#include "VSDBase.h"
#include "nlohmann/json.hpp"

void cms_nano_aod_bootstrap(TTree *tree, nlohmann::json* cList)
{
   g_provider = new VSDProvider(tree);
   
   nlohmann::json &j = g_provider->RefVSDMemberConfig();

   j["EventInfo"] = {{"run", "run"}, {"lumi", "luminosityBlock"}, {"event", "event"}, {"N", "undef"}};
   j["Jet"] = {{"pt", "_pt"}, {"eta", "_eta"}, {"phi", "_phi"}, {"hadFraction", "_chHEF"}, {"N", "n"}};
   j["Muon"] = {{"pt", "_pt"}, {"eta", "_eta"}, {"phi", "_phi"}, {"charge", "_charge"}, {"global", "_isGlobal"}, {"N", "n"}};
   j["Candidate"] = {{"pt", "_pt"}, {"eta", "_eta"}, {"phi", "_phi"},{"N", "n"}};
   j["MET"] = {{"pt", "_pt"}, {"phi", "_phi"}, {"sumEt", "_sumEt"}, {"N", "undef"}};
   j["Vertex"] = {{"x", "_x"}, {"y", "_y"}, {"z", "_z"}, {"N", "undef"}};

   /*
      g_provider->registerCollection("", "EventInfo");
      g_provider->registerCollection("Jet", "Jet", kYellow);
      g_provider->registerCollection("PV", "Vertex", kGreen);
      g_provider->registerCollection("ChsMET", "MET", kViolet);
      g_provider->registerCollection("Electron", "Candidate", kViolet);
      g_provider->registerCollection("Muon", "Muon", kRed);
      */

   for (nlohmann::json::iterator it = cList->begin(); it != cList->end(); ++it)
   {
      // std::cout << *it << '\n';
      nlohmann::json col = *it;
      Color_t color = kBlue;
      std::string filter;
      try
      {
         color = col.at("color");
      }
      catch (std::exception &e)
      {
         //std::cout << "access color " << e.what() << "\n";
      }
      try
      {
         filter = col.at("filter");
      }
      catch (std::exception &e)
      {
         // std::cout << " acces filter " << e.what() << "\n";
      }
printf("----------------------------------------------------------\n");
      g_provider->registerCollection(col["prefix"], col["type"], color, filter);
printf("----------------------------------------------------------\n");
   }
}
