#include <ExperimentXml.h>

#include <iostream>

#include <filesystem>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/math/constants/constants.hpp>

#include <CommonDef.h>
#include <PluginCtrl.h>

#ifndef _DEBUG
#define COMPARE_ID
#endif // !_DEBUG


CREATE_STD_UNIQUE_IMPLEMENT(CExperimentXml)

CExperimentXml::CExperimentXml()
{
    Clear();
    m_pWorldXml = CWorldXml::Create();
    m_pEquipmentXml = CEquipmentXml::Create();
}

CExperimentXml::~CExperimentXml()
{
    Clear();

    if (m_pWorldXml)
    {
        m_pWorldXml.reset();
    }

    if (m_pEquipmentXml)
    {
        m_pEquipmentXml.reset();
    }
}

void CExperimentXml::Clear() noexcept
{
    //m_bPalyback = false;
    m_nEgoShape = 0;
    m_bSyncBlock = false;
    m_strExperimentXml.clear();
}


template<typename T>
bool GetChildStringValue(const T& node, const std::string& child, std::string& value)
{
    value.clear();
    if (node.get_child_optional(child))
    {
        value = node.get_child(child).get_value<std::string>();
        return true;
    }
    return false;
}

template<typename T>
bool ParsePlugin(const T& plugin, const std::string& strDatabaseHome, std::map<std::string, std::int32_t>& mapAgentLink2Cnt)
{
    if (!plugin.get_child_optional("<xmlattr>"))
    {
        return false;
    }
    auto&& attrs = plugin.get_child("<xmlattr>");
    if (!attrs.get_child_optional("nodeId"))
    {
        return false;
    }
    auto&& node_id = attrs.get_child("nodeId");
#ifdef COMPARE_ID
    if (!boost::algorithm::iequals(node_id.get_value<std::string>(), Plugins.GetNodeId()))
    {
        return false;
    }
#endif // COMPARE_ID

    std::string strLink;
    MapParam mapParameter;
    for (const auto& attr : attrs)
    {
        if (boost::algorithm::iequals(attr.first, "link"))
        {
            strLink = attr.second.get_value<std::string>();
            continue;
        }
        if (!Plugins.IsSkipAttr(attr.first))
        {
            mapParameter.insert(std::make_pair(attr.first, attr.second.get_value<std::string>()));
        }
    }
    if (strLink.empty())
    {
        return false;
    }
    std::filesystem::path pathPlugin(strDatabaseHome);
    pathPlugin /= "Plugin";
    pathPlugin /= "Agent";
    pathPlugin /= strLink;
    strLink = pathPlugin.string();
    auto findLinkCnt = mapAgentLink2Cnt.find(strLink);
    if (findLinkCnt == mapAgentLink2Cnt.end())
    {
        mapAgentLink2Cnt.insert(std::make_pair(strLink, 0));
    }
    else
    {
        findLinkCnt->second += 1;
    }
    if (!Plugins.LoadPlugin(Plugin::Agent, strLink, mapAgentLink2Cnt[strLink], std::move(mapParameter)))
    {
        printf("load failed, %s\n", strLink.c_str());
        return false;
    }
    printf("load success, %s\n", strLink.c_str());
    return true;
}

template<typename T>
bool ParseSensor(const T& sensor, const std::string& strDatabaseHome, std::map<std::string, std::int32_t>& mapSensorLink2Cnt)
{
    if (!sensor.get_child_optional("<xmlattr>"))
    {
        return false;
    }
    auto&& attrs = sensor.get_child("<xmlattr>");
    if (!attrs.get_child_optional("nodeId"))
    {
        return false;
    }
    auto&& node_id = attrs.get_child("nodeId");
#ifdef COMPARE_ID
    if (!boost::algorithm::iequals(node_id.get_value<std::string>(), Plugins.GetNodeId()))
    {
        return false;
    }
#endif // COMPARE_ID
    std::string strLink;
    MapParam mapParameter;
    for (const auto& attr : attrs)
    {
        if (boost::algorithm::iequals(attr.first, "link"))
        {
            strLink = attr.second.get_value<std::string>();
            continue;
        }
        if (!Plugins.IsSkipAttr(attr.first))
        {
            mapParameter.insert(std::make_pair(attr.first, attr.second.get_value<std::string>()));
        }
    }
    if (strLink.empty())
    {
        return false;
    }
    std::filesystem::path pathPlugin(strDatabaseHome);
    pathPlugin /= "Plugin";
    pathPlugin /= "Sensor";
    pathPlugin /= strLink;
    strLink = pathPlugin.string();
    auto findLinkCnt = mapSensorLink2Cnt.find(strLink);
    if (findLinkCnt == mapSensorLink2Cnt.end())
    {
        mapSensorLink2Cnt.insert(std::make_pair(strLink, 0));
    }
    else
    {
        findLinkCnt->second += 1;
    }
    if (!Plugins.LoadPlugin(Plugin::Sensor, strLink, mapSensorLink2Cnt[strLink], std::move(mapParameter)))
    {
        printf("load failed, %s\n", strLink.c_str());
        return false;
    }
    printf("load success, %s\n", strLink.c_str());
    return true;
}

bool CExperimentXml::Parse(const std::string& strDatabaseHome, const std::string& strExperimentXml)
{
    try {
        boost::property_tree::ptree propertyTree;
        boost::property_tree::xml_parser::read_xml(
            strExperimentXml.c_str(),
            propertyTree,
            boost::property_tree::xml_parser::trim_whitespace,
            std::locale()
        );

        const char* EquipmentsNode = "Experiment.Equipments";
        if (!propertyTree.get_child_optional(EquipmentsNode))
        {
            return false;
        }

        std::string strLink;
        std::string strValue;
        MapParam mapParameter;
        std::string strNodeId = DefaultNodeId;
        std::string strHostId = DefaultHostId;
        std::map<std::string, std::int32_t> mapAgentLink2Cnt;
        std::map<std::string, std::int32_t> mapSensorLink2Cnt;
        const auto& NodeId = Plugins.GetNodeId();
        const auto& HostId = Plugins.GetHostId();

        BOOST_FOREACH(auto const& equipment, propertyTree.get_child(EquipmentsNode))
        {
            mapParameter.clear();
            if (boost::algorithm::iequals(equipment.first, "Equipment"))
            {
#ifdef COMPARE_ID
                if (GetChildStringValue<>(equipment.second, "<xmlattr>.hostId", strValue))
                {
                    strHostId = strValue;
                }
                if (!boost::algorithm::iequals(strHostId, HostId))
                {
                    continue;
                }
#endif // COMPARE_ID

                if (equipment.second.get_child_optional("Agent"))
                {
                    auto&& agent = equipment.second.get_child("Agent");
                    for (const auto& plugin : agent)
                    {
                        ParsePlugin(plugin.second, strDatabaseHome, mapAgentLink2Cnt);
                    }
                }

                if (equipment.second.get_child_optional("Sensors"))
                {
                    auto&& sensors = equipment.second.get_child("Sensors");
                    for (const auto& sensor : sensors)
                    {
                        ParseSensor(sensor.second, strDatabaseHome, mapSensorLink2Cnt);
                    }
                }
            }
        }
    }
    catch (boost::property_tree::ptree_error& e)
    {
        std::cerr << e.what() << std::endl;
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    m_strExperimentXml = strExperimentXml;

    return true;
}
