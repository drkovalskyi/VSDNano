void reader()
{
    auto file = TFile::Open("nano-CMSSW_11_0_0-RelValZTT-mcRun.root");
    auto tree = (TTree *)file->Get("Events");

    // setup data access
    /*
    auto readerSrcFile = TFile::Open("VSDReader.h");
    if (!readerSrcFile)
    {
        printf("Creating class from TTree.h !!!!\n");
        tree->MakeClass("VSDReader");
    }*/
    gROOT->LoadMacro("VSDReader.C");
    gROOT->LoadMacro("cms_nano_aod_bootstrap.C");
    TString cmd = TString::Format("cms_nano_aod_bootstrap((TTree*)%p)", tree);
    printf("CMD %s \n", cmd.Data());
    gROOT->ProcessLine(cmd.Data());
}
