
#include "nlohmann/json.hpp"

void reader(const char *dataPath, const char *jsonPath)
{
   std::string opath = gSystem->pwd();

   // read json config
   std::ifstream ifs(jsonPath);
   nlohmann::json j = nlohmann::json::parse(ifs);
   std::cout << j.dump(4) << "\n";

   // setup data access
   auto file = TFile::Open(dataPath);
   std::string hash = file->GetUUID().AsString();
   auto tree = (TTree *)file->Get("Events");
   std::string readerPath = Form("%s/%s", opath.c_str(), hash.c_str());
   std::string readerPathMacro = readerPath + "/VsdReader.C";
   if (gSystem->AccessPathName(readerPathMacro.c_str()))
   {
      printf("Creating class from TTree.\n");
      gSystem->mkdir(readerPath.c_str());
      gSystem->cd(readerPath.c_str());
      tree->MakeClass("VsdReader");
      gSystem->cd("..");
   }
   else
   {
      printf("reusing tree created class.\n");
   }
   gROOT->LoadMacro(readerPathMacro.c_str());
   gROOT->LoadMacro("bootstrap.C");
   TString cmd = TString::Format("bootstrap((TFile*)%p, (nlohmann::json*)%p)", file, &j);
   // printf("CMD %s \n", cmd.Data());
   gROOT->ProcessLine(cmd.Data());
   gROOT->LoadMacro("evd.h");
   gROOT->ProcessLine("evd()");
}
