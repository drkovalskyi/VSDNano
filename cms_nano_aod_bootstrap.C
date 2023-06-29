#include "VSDBase.h"
#include "nlohmann/json.hpp"

void cms_nano_aod_bootstrap(TTree *tree, nlohmann::json* mconfig, nlohmann::json* cList)
{
   g_provider = new VSDProvider(tree, mconfig);

   for (nlohmann::json::iterator it = cList->begin(); it != cList->end(); ++it)
   {
      // optional color
      nlohmann::json col = *it;
      Color_t color = kBlue;
      try
      {
         color = col.at("color");
      }
      catch (std::exception &e)
      {
      }
      // optional filter
      std::string filter = "";
      try
      {
         filter = col.at("filter");
      }
      catch (std::exception &e)
      {
      }

      printf("----------------------------------------------------------\n");
      g_provider->registerCollection(col["prefix"], col["type"], color, filter);
      printf("----------------------------------------------------------\n");
   }
}
