#include <EquipmentXml.h>

#include <iostream>
#include <filesystem>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <PluginCtrl.h>

#ifndef _DEBUG
#define COMPARE_ID
#endif // !_DEBUG

CREATE_STD_UNIQUE_IMPLEMENT(CEquipmentXml)

CEquipmentXml::CEquipmentXml()
{
}

CEquipmentXml::~CEquipmentXml()
{
}
