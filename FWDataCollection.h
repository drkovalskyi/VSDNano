#ifndef FWVCollection_h
#define FWVCollection_h

#include "ROOT/REveDataCollection.hxx"
#include "ROOT/REveDataProxyBuilderBase.hxx"
#include "nlohmann/json.hpp"


class FWDataCollection : public ROOT::Experimental::REveDataCollection
{
    private:
    ROOT::Experimental::REveDataProxyBuilderBase * m_builder {nullptr};

    public:
    FWDataCollection(const std::string &n = "FWDataCollection", const std::string &varconfig = "") : 
      ROOT::Experimental::REveDataCollection(n, "")
    {
        if (varconfig.empty())
        {
            m_config = nlohmann::json::array();
        }
        else {
            m_config = nlohmann::json::parse(varconfig);
        }
    }
    ~FWDataCollection() override {}

    // config handling
    nlohmann::json m_config;

    bool hasConfigWithName(const std::string& n)
    {
        for (auto &elem : m_config)
            if (elem["name"] == n)
                return true;
        return false;
    }

    void assertParamter(nlohmann::json j)
    {
        if (hasConfigWithName(j["name"]))
        return;
        m_config.push_back(j);
    }


    void setGLBuilder(ROOT::Experimental::REveDataProxyBuilderBase * ib)
    {
       m_builder = ib;
    }

    int WriteCoreJson(nlohmann::json &j, int rnr_offset) override
    {
        int res = REveDataCollection::WriteCoreJson(j, -1);
        j["var"] = m_config;

        return res;
    }

    void UpdatePBParameter(char *name, char *val)
    {
        printf("Udate PB paramter %s %s \n", name, val);
        std::string ssn = name;
        for (auto &elem : m_config)
        {
            std::string confn = elem["name"];
            if (confn == ssn)
            {
                std::cout << "match " << elem.dump(3) << "\n";
                std::string typen = elem["type"];
                if (typen == "Bool")
                {
                    int x = strcmp(val, "true");
                    elem["val"] = (x == 0);
                }
                else if (typen == "Long")
                {
                    char *eptr;
                    long x = strtol(val, &eptr, 10);
                    if (errno == EINVAL)
                    {
                        printf("Conversion error occurred: %d\n", errno);
                        return;
                    }
                    else
                    {
                        elem["val"] = x;
                    }
                }
            }
        }
        StampObjProps();
        if (m_builder) m_builder->Build();
    }

    long getLongParameter(const std::string &name)
    {

        for (auto &elem : m_config)
        {
            std::string confn = elem["name"];
            if (confn == name)
            {
                // std::cout << "match " << elem.dump(3) << "\n";
                std::string typen = elem["type"];
                if (typen == "Long")
                {
                    /*
                const char* val = elem["val"];
                    char *eptr;
                    long x = strtol(val, &eptr, 10);
                    if (errno == EINVAL)
                    {
                        printf("Conversion error occurred: %d\n", errno);
                        return;
                    }
                 return x;*/
                 return elem["val"];
                }
            }
        }
        printf("can't locate long paramter\n");
        return 0;
    }


    bool getBoolParameter(const std::string &name)
    {

        for (auto &elem : m_config)
        {
            std::string confn = elem["name"];
            if (confn == name)
            {
                // std::cout << "gert bool match " << elem.dump(3) << "\n";
                std::string typen = elem["type"];
                /*
                const char* val = elem["val"];
                if (typen == "Bool")
                {
                    int x = strcmp("val", "true");
                    return x;
                }*/
                return  elem["val"];
            }
        }
        printf("can't locate bool paramter\n");
        return false;
    }

};


#endif
