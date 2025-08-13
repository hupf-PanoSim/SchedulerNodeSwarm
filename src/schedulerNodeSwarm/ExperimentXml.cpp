#include <ExperimentXml.h>

#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/math/constants/constants.hpp>

#include <CommonDef.h>
#include <PluginCtrl.h>


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


bool CExperimentXml::Parse(const std::string& strDatabaseHome, const std::string& strExperimentXml)
{
    if (m_pEquipmentXml)
    {
        if (!m_pEquipmentXml->Parse(strDatabaseHome, strExperimentXml))
        {
            return false;
        }
    }

    try
    {
        std::int32_t nJudgeIndex = 0;
        std::map<std::string, std::int32_t> mapLink2Cnt;
        auto loadPlugins = [&](const boost::property_tree::ptree& pluginNode, std::string_view strSubFolder, Plugin emPlguin) -> bool {
            std::string strLink;
            std::string strNodeId = DefaultNodeId;
            std::string strHostId = DefaultHostId;
            MapParam mapParameter;
            const auto& attributes = pluginNode.get_child("<xmlattr>");
            for (const auto& attr : attributes) {
                if (boost::algorithm::iequals(attr.first, "link")) {
                    strLink = attr.second.get_value<std::string>();
                    continue;
                }

                if (boost::algorithm::iequals(attr.first, "nodeId")) {
                    strNodeId = attr.second.get_value<std::string>();
                    continue;
                }

                if (boost::algorithm::iequals(attr.first, "hostId")) {
                    strHostId = attr.second.get_value<std::string>();
                    continue;
                }

                if (Plugins.IsSkipAttr(attr.first)) {
                    continue;
                }
                mapParameter.insert(std::make_pair(attr.first, attr.second.get_value<std::string>()));
            }

            if (strLink.empty()) {
                return false;
            }

            std::int32_t nIndex = 0;
            auto findLink = mapLink2Cnt.find(strLink);
            if (findLink == mapLink2Cnt.end()) {
                mapLink2Cnt.insert(std::make_pair(strLink, 0));
            }
            else {
                nIndex = findLink->second + 1;
                findLink->second = nIndex;
            }

            if (!boost::algorithm::iequals(Plugins.GetNodeId(), strNodeId)) {
                return false;
            }

            if (!boost::algorithm::iequals(Plugins.GetHostId(), strHostId)) {
                return false;
            }

            boost::filesystem::path pathPlugin(strDatabaseHome);
            pathPlugin /= "Plugin";
            pathPlugin /= strSubFolder.data();
            pathPlugin /= strLink;
            strLink = pathPlugin.string();

            return Plugins.LoadPlugin(
                emPlguin, strLink, emPlguin == Plugin::Evaluator ? nJudgeIndex++ : nIndex, std::move(mapParameter)
            );
        };

        boost::property_tree::ptree propertyTree;
        boost::property_tree::xml_parser::read_xml(
            strExperimentXml.c_str(),
            propertyTree, boost::property_tree::xml_parser::trim_whitespace,
            std::locale()
        );

        //m_bPalyback = false;
        //{
        //    auto&& mode = propertyTree.get_child_optional("Experiment.<xmlattr>.mode");
        //    if (mode != boost::none) {
        //        m_bPalyback = boost::algorithm::iequals(mode.get().get_value<std::string>(), "playback");
        //    }
        //}

        auto loadPlugin = [&](std::string_view strXPath, std::string_view strPlugin, Plugin emPlguin) {
            if (propertyTree.get_child_optional(strXPath.data())) {
                BOOST_FOREACH(auto const& child, propertyTree.get_child(strXPath.data())) {
                    loadPlugins(child.second, strPlugin, emPlguin);
                }
            }
        };
        loadPlugin("Experiment.Agent", "Agent", Plugin::Agent);
        //loadPlugin("Experiment.Evaluator", "Evaluator", Plugin::Evaluator);
    }
    catch (boost::property_tree::ptree_error& e) {
        std::cerr << e.what() << std::endl;
    }

    if (m_pWorldXml)
    {
        if (!m_pWorldXml->Parse(strDatabaseHome, strExperimentXml))
        {
            return false;
        }
    }

    m_strExperimentXml = strExperimentXml;

    return true;
}
