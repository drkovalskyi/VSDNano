#include "VsdProvider.h"

void testevd(const char *vsd_file="vsd-nano.root")
{
    try {
        g_provider = new VsdProvider(vsd_file);
        gROOT->LoadMacro("evd.h");
        gROOT->ProcessLine("evd()");
    }
    catch (const std::exception& e)
    {
        std::cout << "failed to run event display: " << e.what();
    }
}
