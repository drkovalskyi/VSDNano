#include "VsdProvider.h"

void testevd()
{
    try {
        g_provider = new VsdProvider("vsd-nano.root");
        gROOT->LoadMacro("evd.h");
        gROOT->ProcessLine("evd()");
    }
    catch (const std::exception& e)
    {
        std::cout << "failed to run event display: " << e.what();
    }
}
