#pragma once

#include <unordered_map>

#include <Interface.h>
#include <PointerDef.h>

class CWorldXml final
{
    DEFINE_STD_UNIQUE_PTR(CWorldXml)
    DECLARE_DEFAULT_UNIQUE_CONSTRUCTION(CWorldXml)
public:
    bool Parse(const std::string& strDatabaseHome, const std::string& strExperimentXml);

protected:
    void Clear() noexcept;

public:
};
