#include <RuntimeSettings.h>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <CommonDef.h>
#include <Utility.h>
#include <CsvFiles.h>


static const char* const EquipmentCsvFileRelativePath = "Equipment/official.csv";
//static const char* const PedestrianCsvFileRelativePath = "Resource/Traffic/Pedestrian/official.csv";
//static const char* const OtherCsvFileRelativePath = "Resource/Traffic/Other/official.csv";
//static const char* const ObstacleCsvFileRelativePath = "Resource/Static/Obstacle/official.csv";
//static const char* const FacilityCsvFileRelativePath = "Resource/Static/Facility/official.csv";


IMPLEMENT_SINGLETON(CRuntimeSettings)

CRuntimeSettings::CRuntimeSettings()
{
    m_strPanoSwarmDatabaseHome.clear();
    m_strOutputPath.clear();
    m_pExperimentXml = CExperimentXml::Create();
}

CRuntimeSettings::~CRuntimeSettings()
{
    if (m_pExperimentXml)
    {
        m_pExperimentXml.reset();
    }
    m_strOutputPath.clear();
}

bool CRuntimeSettings::GetExperimentXml(const std::string& strExperimentName, std::string& strExperimentXml) const noexcept
{
    strExperimentXml.clear();

    boost::filesystem::path pathExperiment(m_strPanoSwarmDatabaseHome);
    pathExperiment /= "Experiment";
    if (!boost::filesystem::exists(pathExperiment))
    {
        return false;
    }
    strExperimentXml = strExperimentName;
    strExperimentXml.append(".experiment.xml");
    pathExperiment /= strExperimentXml;
    if (!boost::filesystem::exists(pathExperiment))
    {
        return false;
    }
    strExperimentXml = pathExperiment.string();
    return true;
}

bool CRuntimeSettings::Generate(const std::string& strExperimentName)
{
    if (!Utility::GetDatabaseHome(m_strPanoSwarmDatabaseHome))
    {
        return false;
    }

    std::string strExperimentXml;
    if (!GetExperimentXml(strExperimentName, strExperimentXml))
    {
        return false;
    }

    //{
    //    // ../PanoSimDatabase/Resource/Static/Facility/official.csv
    //    boost::filesystem::path facilityCsv(m_strPanoSwarmDatabaseHome);
    //    facilityCsv /= FacilityCsvFileRelativePath;
    //    if (!boost::filesystem::exists(facilityCsv))
    //    {
    //        return false;
    //    }
    //    if (!CsvFileCtrl.ParseFacility(facilityCsv.string()))
    //    {
    //        return false;
    //    }
    //}

    if (!m_pExperimentXml->Parse(m_strPanoSwarmDatabaseHome, strExperimentXml))
    {
        return false;
    }

    std::string strEnvResource;
    if (!Utility::GetEnvResource(strEnvResource))
    {
        return false;
    }
    // Equipment--official.csv
    boost::filesystem::path equipmentCsv(strEnvResource);
    equipmentCsv /= EquipmentCsvFileRelativePath;
    if (!boost::filesystem::exists(equipmentCsv))
    {
        return false;
    }
    if (!CsvFileCtrl.ParseEquipmentCsv(equipmentCsv.string()))
    {
        return false;
    }

    //{
    //    // Pedestrian--official.csv
    //    boost::filesystem::path pedestrianCsv(m_strPanoSwarmDatabaseHome);
    //    pedestrianCsv /= PedestrianCsvFileRelativePath;
    //    if (!boost::filesystem::exists(pedestrianCsv))
    //    {
    //        return false;
    //    }
    //    if (!CsvFileCtrl.ParsePedestrianCsv(pedestrianCsv.string()))
    //    {
    //        return false;
    //    }
    //}

    //{
    //    // Other--official.csv
    //    boost::filesystem::path otherCsv(m_strPanoSwarmDatabaseHome);
    //    otherCsv /= OtherCsvFileRelativePath;
    //    if (!boost::filesystem::exists(otherCsv))
    //    {
    //        return false;
    //    }
    //    if (!CsvFileCtrl.ParseOtherCsv(otherCsv.string()))
    //    {
    //        return false;
    //    }
    //}

    //{
    //    // D:\PanoSwarm\Database\Resource\Static\Obstacle\official.csv
    //    boost::filesystem::path obstacleCsv(m_strPanoSwarmDatabaseHome);
    //    obstacleCsv /= ObstacleCsvFileRelativePath;
    //    if (!boost::filesystem::exists(obstacleCsv))
    //    {
    //        return false;
    //    }
    //    if (!CsvFileCtrl.ParseObstacleCsv(obstacleCsv.string()))
    //    {
    //        return false;
    //    }
    //}

    m_strOutputPath = strExperimentName;
    m_strOutputPath.append("/Data");

    return true;
}

std::int32_t CRuntimeSettings::GetEgoShape() const noexcept
{
    return m_pExperimentXml->m_nEgoShape;
}

const std::string& CRuntimeSettings::GetOutputPath() const noexcept
{
    return m_strOutputPath;
}
