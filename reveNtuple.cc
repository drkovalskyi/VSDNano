
#include <iostream>
#include <fstream>

#include "TRint.h"
#include "TApplication.h"
#include "TROOT.h"
#include "TFile.h"
#include "TSystem.h"
#include "TTree.h"
#include "nlohmann/json.hpp"

int main(int argc, char *argv[])
{
   if (argc < 3)
   {
      std::cout << "Need input data file and json file !" << std::endl;
      return 1;
   }
   const char *dummyArgvArray[] = {argv[0]};
   char **dummyArgv = const_cast<char **>(dummyArgvArray);
   int dummyArgc = 1;

   auto app = new TRint("reveNtuple", &dummyArgc, dummyArgv);
   const char* dataPath = argv[1];
   const char* jsonPath = argv[2];

   ROOT::EnableThreadSafety();

   // save original path
   std::string opath = gSystem->pwd();

   // read json config
   std::ifstream ifs(jsonPath);
   nlohmann::json j = nlohmann::json::parse(ifs);
   // std::cout << j.dump(4) << "\n";

   // setup data access
   auto file = TFile::Open(dataPath);
   std::string hash = file->GetUUID().AsString();
   auto tree = (TTree *)file->Get("Events");
   std::string readerPath = Form("%s/%s", opath.c_str(), hash.c_str());
   std::string readerPathMacro = readerPath + "/VSDReader.C";
   if (gSystem->AccessPathName(readerPathMacro.c_str()))
   {
      printf("Creating class from TTree.\n");
      gSystem->mkdir(readerPath.c_str());
      gSystem->cd(readerPath.c_str());
      tree->MakeClass("VSDReader");
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
   app->Run();
}
