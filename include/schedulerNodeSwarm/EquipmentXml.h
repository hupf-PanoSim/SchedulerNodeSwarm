#pragma once

#include <PointerDef.h>

class CEquipmentXml final
{
    DEFINE_STD_UNIQUE_PTR(CEquipmentXml)
    DECLARE_DEFAULT_UNIQUE_CONSTRUCTION(CEquipmentXml)
public:
    bool Parse(const std::string& strDatabaseHome, const std::string& strExperimentXml);
};
