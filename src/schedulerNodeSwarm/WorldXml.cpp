#include <WorldXml.h>

#include <type_traits>
#include <iostream>
#include <cmath>

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/math/constants/constants.hpp>

#pragma warning(disable : 4819)
#include <boost/geometry.hpp>

#include <CommonDef.h>
#include <CsvFiles.h>

#define PrintParkingSpotData    0

//#define ReadOption(v)	\
//{v.clear(); auto xmlOptional = trafficNode.get_optional<std::string>("<xmlattr>."#v); if(xmlOptional) { v = xmlOptional.get();}}

CREATE_STD_UNIQUE_IMPLEMENT(CWorldXml)


static bool findInteger(const boost::property_tree::ptree& attributes, const std::string_view strKey, std::int32_t& nValue);
static bool findDouble(const boost::property_tree::ptree& attributes, const std::string_view strKey, double& doubleValue);
static bool findString(const boost::property_tree::ptree& attributes, const std::string_view strKey, std::string& strValue);


CWorldXml::CWorldXml()
{
    Clear();
}

CWorldXml::~CWorldXml()
{
    Clear();
}

void CWorldXml::Clear() noexcept
{
}

bool CWorldXml::Parse(const std::string& strDatabaseHome, const std::string& strExperimentXml)
{
    //const char* const WorldNet = "Experiment.World.Net";
    //const char* const WorldEnvironment = "Experiment.World.Environment";

    //try {
    //    boost::property_tree::ptree propertyTree;

    //    boost::property_tree::xml_parser::read_xml(
    //        strExperimentXml.c_str(),
    //        propertyTree,
    //        boost::property_tree::xml_parser::trim_whitespace,
    //        std::locale()
    //    );

    //    m_strNetworkXml.clear();
    //    const auto& netNode = propertyTree.get_child_optional(WorldNet);
    //    if (!netNode) {
    //        return false;
    //    }
    //    const auto& netOptional = netNode.get().get_optional<std::string>("<xmlattr>.link");
    //    if (netOptional) {
    //        m_strNetworkXml = netOptional.get();
    //    }

    //}
    //catch (boost::property_tree::ptree_error& e) {
    //    std::cerr << e.what() << std::endl;
    //    return false;
    //}

    //boost::filesystem::path pathNetwork(strDatabaseHome);
    //pathNetwork /= "World";
    //pathNetwork /= m_strNetworkXml;
    //if (!boost::filesystem::exists(pathNetwork)) {
    //    return false;
    //}
    //m_strNetworkXml = pathNetwork.string();
    //std::cout << m_strNetworkXml << std::endl;

    return true;
}

bool findInteger(const boost::property_tree::ptree& attributes, const std::string_view strKey, std::int32_t& nValue)
{
    nValue = 0;
    auto findResult = attributes.find(strKey.data());
    if (findResult == attributes.not_found())
    {
        return false;
    }
    nValue = findResult->second.get_value<std::int32_t>();
    return true;
}

bool findDouble(const boost::property_tree::ptree& attributes, const std::string_view strKey, double& doubleValue)
{
    doubleValue = 0;
    auto findResult = attributes.find(strKey.data());
    if (findResult != attributes.not_found())
    {
        const std::string& strValue = findResult->second.get_value<std::string>();
        if (!strValue.empty())
        {
            doubleValue = std::stod(strValue);
            return true;
        }
    }
    return false;
}

bool findString(const boost::property_tree::ptree& attributes, const std::string_view strKey, std::string& strValue)
{
    strValue.clear();
    auto findResult = attributes.find(strKey.data());
    if (findResult != attributes.not_found())
    {
        strValue = findResult->second.get_value<std::string>();
        return true;
    }
    return false;
}
