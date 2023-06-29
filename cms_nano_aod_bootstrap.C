#include "VSDBase.h"
#include "nlohmann/json.hpp"

void cms_nano_aod_bootstrap(TTree *tree)
{
   g_provider = new VSDProvider(tree);
   
   nlohmann::json &j = g_provider->RefVSDMemberConfig();

   j["EventInfo"] = {{"run", "run"}, {"lumi", "luminosityBlock"}, {"event", "event"}, {"N", "undef"}};
   j["Jet"] = {{"pt", "_pt"}, {"eta", "_eta"}, {"phi", "_phi"}, {"hadFraction", "_chHEF"}, {"N", "n"}};
   j["Muon"] = {{"pt", "_pt"}, {"eta", "_eta"}, {"phi", "_phi"}, {"charge", "_charge"}, {"global", "_isGlobal"}, {"N", "n"}};
   j["Candidate"] = {{"pt", "_pt"}, {"eta", "_eta"}, {"phi", "_phi"},{"N", "n"}};
   j["MET"] = {{"pt", "_pt"}, {"phi", "_phi"}, {"sumEt", "_sumEt"}, {"N", "undef"}};
   j["Vertex"] = {{"x", "_x"}, {"y", "_y"}, {"z", "_z"}, {"N", "undef"}};

   g_provider->registerCollection("", "EventInfo");
   g_provider->registerCollection("Jet", "Jet", kYellow);
   g_provider->registerCollection("PV", "Vertex", kGreen);
   g_provider->registerCollection("ChsMET", "MET", kViolet);
   g_provider->registerCollection("Electron", "Candidate", kViolet);
   g_provider->registerCollection("Muon", "Muon", kRed);
}
