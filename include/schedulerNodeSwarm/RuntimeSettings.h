#pragma once

#include <ExperimentXml.h>
#include <SingletonDef.h>
#include <CommonDef.h>


class CRuntimeSettings final
{
    DECLARE_SINGLETON(CRuntimeSettings)
public:
    bool Generate(const std::string& strExperimentName);

    std::int32_t GetEgoShape() const noexcept;

    const std::string& GetOutputPath() const noexcept;

    bool SyncBlock() const noexcept {
        return m_pExperimentXml->m_bSyncBlock;
    }

    ~CRuntimeSettings();

protected:
    explicit CRuntimeSettings();
    bool GetExperimentXml(const std::string& strExperimentName, std::string& strExperimentXml) const noexcept;

private:
    std::string m_strPanoSwarmDatabaseHome;

    // Demo/Temp/Data
    std::string m_strOutputPath;

    // D:\PanoSwarm\PanoSwarmDatabase\Experiment\Demo.experiment.xml
    CExperimentXml::Pointer m_pExperimentXml;
};

#define RuntimeSettings    CRuntimeSettings::GetInstance()
