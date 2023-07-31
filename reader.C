
#include "nlohmann/json.hpp"
void reader()
{ 
    TFile::SetCacheFileDir(".");
   // auto file = TFile::Open("http://amraktad.web.cern.ch/amraktad/nano-CMSSW_11_0_0-RelValZTT-mcRun.root");
    auto file = TFile::Open("nano-CMSSW_11_0_0-RelValZTT-mcRun.root");

 nlohmann::json j = nlohmann::json::parse(R"(
{
   "CaloMET": {
      "color": 616,
      "fields": {
         "phi": "i.CaloMET_phi",
         "pt": "i.CaloMET_pt",
         "sumEt": "i.CaloMET_sumEt"
      },
      "purpose": "MET",
      "size": "single"
   },
   "ChsMET": {
      "color": 632,
      "fields": {
         "phi": "i.ChsMET_phi",
         "pt": "i.ChsMET_pt",
         "sumEt": "i.ChsMET_sumEt"
      },
      "purpose": "MET",
      "size": "single"
   },
   "Electron": {
      "color": 432,
      "fields": {
         "eta": "i.Electron_eta",
         "phi": "i.Electron_phi",
         "pt": "i.Electron_pt"
      },
      "purpose": "Candidate",
      "size": "i.nElectron"
   },
   "EventInfo": {
      "fields": {
         "event": "i.event",
         "lumi": "i.luminosityBlock",
         "run": "i.run"
      },
      "purpose": "EventInfo",
      "size": "single"
   },
   "Jet": {
      "color": 400,
      "fields": {
         "eta": "i.Jet_eta",
         "phi": "i.Jet_phi",
         "pt": "i.Jet_pt"
      },
      "purpose": "Jet",
      "size": "i.nJet"
   },
   "Muon": {
      "color": 632,
      "fields": {
         "eta": "i.Muon_eta",
         "phi": "i.Muon_phi",
         "pt": "i.Muon_pt"
      },
      "purpose": "Muon",
      "size": "i.nMuon"
   },
   "Vertex": {
      "color": 416,
      "fields": {
         "x": "i.PV_x",
         "y": "i.PV_y",
         "z": "i.PV_z"
      },
      "purpose": "Vertex",
      "size": "single"
   }
})");

   // setup data access
   auto tree = (TTree *)file->Get("Events");
   auto readerSrcFile = TFile::Open("VSDReader.h");
   if (!readerSrcFile)
   {
       printf("Creating class from TTree.h !!!!\n");
       tree->MakeClass("VSDReader");
    }
    gROOT->LoadMacro("VSDReader.C");

    gROOT->LoadMacro("cms_nano_aod_bootstrap.C");
    TString cmd = TString::Format("cms_nano_aod_bootstrap((TFile*)%p, (nlohmann::json*)%p)", file, &j);
    
    // printf("CMD %s \n", cmd.Data());
    gROOT->ProcessLine(cmd.Data());
    gROOT->LoadMacro("evd.h");
    gROOT->ProcessLine("evd()");
}
