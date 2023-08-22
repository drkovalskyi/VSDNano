
#include "nlohmann/json.hpp"
void reader()
{ 
std::string opath = gSystem->pwd();
std::cout  << "orig path ........ " << opath << "\n";
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
 auto file = TFile::Open("nano-CMSSW_11_0_0-RelValZTT-mcRun.root");
 //gDebug = 1;
 std::string hash = file->GetUUID().AsString();
 auto tree = (TTree *)file->Get("Events");
 std::string readerPath = Form("%s/%s", opath.c_str(), hash.c_str());
 std::string readerPathMacro = readerPath + "/VSDReader.C";
 if (gSystem->AccessPathName(readerPathMacro.c_str()))
 {
    std::cout << "======================== reader path " << readerPath << "\n";
    gSystem->mkdir(readerPath.c_str());
    gSystem->cd(readerPath.c_str());
    printf("Creating class from TTree.h !!!!\n");
    tree->MakeClass("VSDReader");
    gSystem->cd("..");
 }
 else {
   printf("reusing tree create macro \n");
 }
 gROOT->LoadMacro(readerPathMacro.c_str());
 gROOT->LoadMacro("bootstrap.C");
 TString cmd = TString::Format("bootstrap((TFile*)%p, (nlohmann::json*)%p)", file, &j);

 // printf("CMD %s \n", cmd.Data());
 gROOT->ProcessLine(cmd.Data());
 gROOT->LoadMacro("evd.h");
 gROOT->ProcessLine("evd()");
}
