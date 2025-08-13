#pragma once

#include <WorldXml.h>
#include <EquipmentXml.h>

class CExperimentXml final
{
    DEFINE_STD_UNIQUE_PTR(CExperimentXml)
    DECLARE_DEFAULT_UNIQUE_CONSTRUCTION(CExperimentXml)
public:
    bool Parse(const std::string& strDatabaseHome, const std::string& strExperimentXml);

protected:
    void Clear() noexcept;

public:
    std::string m_strExperimentXml;

    //bool m_bPalyback;
    std::int32_t m_nEgoShape;
    bool m_bSyncBlock;

    CWorldXml::Pointer m_pWorldXml;
    CEquipmentXml::Pointer m_pEquipmentXml;
};
