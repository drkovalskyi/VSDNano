
#include "nlohmann/json.hpp"
void reader()
{ 
    TFile::SetCacheFileDir(".");
    auto file = TFile::Open("http://amraktad.web.cern.ch/amraktad/nano-CMSSW_11_0_0-RelValZTT-mcRun.root");

    // setup data access
    auto tree = (TTree *)file->Get("Events");
    auto readerSrcFile = TFile::Open("VSDReader.h");
    if (!readerSrcFile)
    {
        printf("Creating class from TTree.h !!!!\n");
        tree->MakeClass("VSDReader");
    }
    gROOT->LoadMacro("VSDReader.C");

    // nlohmann::json* cList = new nlohmann::json::array();
    nlohmann::json cList = nlohmann::json::array();
    cList.push_back({{"prefix", ""}, {"type", "EventInfo"}});
    cList.push_back({{"prefix", "Jet"}, {"type", "Jet"}, {"color", kYellow}});
    cList.push_back({{"prefix", "PV"}, {"type", "Vertex"}, {"color", kGreen}});
    cList.push_back({{"prefix", "ChsMET"}, {"type", "MET"}, {"color", kViolet}});
    cList.push_back({{"prefix", "CaloMET"}, {"type", "MET"}, {"color", kRed}});
    cList.push_back({{"prefix", "Electron"}, {"type", "Candidate"}, {"color", kCyan}});
    cList.push_back({{"prefix", "Muon"}, {"type", "Muon"}, {"color", kRed}});

    gROOT->LoadMacro("cms_nano_aod_bootstrap.C");
    TString cmd = TString::Format("cms_nano_aod_bootstrap((TTree*)%p, (nlohmann::json*)%p)", tree, &cList);
    printf("CMD %s \n", cmd.Data());
    gROOT->ProcessLine(cmd.Data());
    gROOT->LoadMacro("evd.C");
    gROOT->ProcessLine("evd()");
}
