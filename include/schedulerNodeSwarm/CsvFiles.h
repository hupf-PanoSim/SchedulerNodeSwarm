#pragma once


#include <unordered_map>
#include <vector>

#include <PointerDef.h>
#include <SingletonDef.h>
#include <Interface.h>


class CEquipmentCsvData final {
    DEFINE_STD_UNIQUE_PTR(CEquipmentCsvData)
public:
    static CEquipmentCsvData::Pointer Create();
    explicit CEquipmentCsvData() noexcept;
    virtual ~CEquipmentCsvData() noexcept;
    void Clear() noexcept;
public:
    std::uint32_t nId;
    object_subtype emSubtype;
    std::string strName;
    std::string strType;
    std::string strLength;
    std::string strWidth;
    std::string strHeight;
    std::string strFrontOverhang;
    std::string strPackName;
    std::string strLightControl;
    std::string strDisplayName;
    std::string strIcon;

    static const CEquipmentCsvData::Pointer NullPointer;
};

class CPedestrianCsvData final {
    DEFINE_STD_UNIQUE_PTR(CPedestrianCsvData)
public:
    static CPedestrianCsvData::Pointer Create();
    explicit CPedestrianCsvData() noexcept;
    virtual ~CPedestrianCsvData() noexcept;
    void Clear() noexcept;
public:
    std::uint32_t nId;
    object_subtype emSubtype;
    std::string strName;
    std::string strType;
    std::string strLength;
    std::string strWidth;
    std::string strHeight;
    std::string strPackName;
    std::string strDisplayName;
    std::string strIcon;

    static const CPedestrianCsvData::Pointer NullPointer;
};

class COtherCsvData final {
    DEFINE_STD_UNIQUE_PTR(COtherCsvData)
public:
    static COtherCsvData::Pointer Create();
    explicit COtherCsvData() noexcept;
    virtual ~COtherCsvData() noexcept;
    void Clear() noexcept;
public:
    std::uint32_t nId;
    object_subtype emSubtype;
    std::string strName;
    std::string strType;
    std::string strLength;
    std::string strWidth;
    std::string strHeight;
    std::string strPackName;
    std::string strDisplayName;
    std::string strIcon;

    static const COtherCsvData::Pointer NullPointer;
};


class CEquipmentCsv final
{
    DEFINE_STD_UNIQUE_PTR(CEquipmentCsv)
    DECLARE_DEFAULT_UNIQUE_CONSTRUCTION(CEquipmentCsv)
public:
    bool Parse(const std::string& strCsvFile);

    bool GetObjectSize(std::int32_t shape, std::tuple<double, double, double>& tupleSize) const noexcept;
    bool GetObjectSubtype(object_type type, std::int32_t shape, object_subtype& emResult) const noexcept;

    double GetFrontOverhang(std::int32_t shape) const noexcept;

protected:
    void Clear() noexcept;

private:
    std::string m_strCsvFile;
    std::vector<CEquipmentCsvData::Pointer> m_vctAll;

    const char* const typeCar = "Car";
    const char* const typeVan = "Van";
    const char* const typeBus = "Bus";
    const char* const typeOther = "OtherVehicle";
};


class CPedestrianCsv final
{
    DEFINE_STD_UNIQUE_PTR(CPedestrianCsv)
    DECLARE_DEFAULT_UNIQUE_CONSTRUCTION(CPedestrianCsv)
public:
    bool Parse(const std::string& strCsvFile);

    bool GetObjectSize(std::int32_t shape, std::tuple<double, double, double>& tupleSize) const noexcept;
    bool GetObjectSubtype(object_type type, std::int32_t shape, object_subtype& emResult) const noexcept;

protected:
    void Clear() noexcept;

private:
    std::string m_strCsvFile;

    std::vector<CPedestrianCsvData::Pointer> m_vctAll;
};


enum class other_csv_type : unsigned char {
    NonMotorVehicle = 0,
    Others
};

class COtherCsv final
{
    DEFINE_STD_UNIQUE_PTR(COtherCsv)
    DECLARE_DEFAULT_UNIQUE_CONSTRUCTION(COtherCsv)
public:
    bool Parse(const std::string& strCsvFile);

    bool GetObjectSize(std::int32_t shape, std::tuple<double, double, double>& tupleSize) const noexcept;
    bool GetObjectSubtype(object_type type, std::int32_t shape, object_subtype& emResult) const noexcept;

protected:
    void Clear() noexcept;

private:
    std::string m_strCsvFile;

    std::vector<COtherCsvData::Pointer> m_vctAll;

    const char* const typeOther = "Others";
    const char* const typeNonMotor = "NonMotorVehicle";
};


class CObstacleCsvData final {
    DEFINE_STD_UNIQUE_PTR(CObstacleCsvData)
public:
    static CObstacleCsvData::Pointer Create();
    explicit CObstacleCsvData() noexcept;
    virtual ~CObstacleCsvData() noexcept;
    void Clear() noexcept;
public:
    std::uint32_t nId;
    std::string strName;
    std::string strLength;
    std::string strWidth;
    std::string strHeight;
    std::string strPackName;
    std::string strDisplayName;
    std::string strIcon;

    static const CObstacleCsvData::Pointer NullPointer;
};

class CObstacleCsv final
{
    DEFINE_STD_UNIQUE_PTR(CObstacleCsv)
    DECLARE_DEFAULT_UNIQUE_CONSTRUCTION(CObstacleCsv)
public:
    bool Parse(const std::string& strCsvFile);

    bool GetObjectSize(std::int32_t shape, std::tuple<double, double, double>& tupleSize) const noexcept;

protected:
    void Clear() noexcept;

private:
    std::string m_strCsvFile;
    std::vector<CObstacleCsvData::Pointer> m_vctAll;
};


class CFacilityCsvData final {
    DEFINE_STD_UNIQUE_PTR(CFacilityCsvData)
public:
    static CFacilityCsvData::Pointer Create();
    explicit CFacilityCsvData() noexcept;
    virtual ~CFacilityCsvData() noexcept;
    void Clear() noexcept;
public:
    std::uint32_t nId;
    std::string strName;
    std::string strLength;
    std::string strWidth;
    std::string strAngle;
    std::string strType;
    std::string strPackName;
    std::string strDisplayName;
    std::string strIcon;

    static const CFacilityCsvData::Pointer NullPointer;
};


class CFacilityCsv final
{
    DEFINE_STD_UNIQUE_PTR(CFacilityCsv)
    DECLARE_DEFAULT_UNIQUE_CONSTRUCTION(CFacilityCsv)
public:
    bool Parse(const std::string& strCsvFile);

    //bool GetParkingSpot(std::int32_t id, double& length, double& width, double& angle) const noexcept;

protected:
    void Clear() noexcept;

private:
    std::string m_strCsvFile;
    std::vector<CFacilityCsvData::Pointer> m_vctAll;
};

class CCsvFileCtrl final
{
    DECLARE_SINGLETON(CCsvFileCtrl)
public:
    bool ParseEquipmentCsv(const std::string& strEquipmentCsvFile);
    //bool ParsePedestrianCsv(const std::string& strPedestrianCsvFile);
    //bool ParseOtherCsv(const std::string& strOtherCsvFile);
    //bool ParseObstacleCsv(const std::string& strObstacleCsvFile);
    //bool ParseFacility(const std::string& strFacilityCsvFile);

    bool GetObjectSize(object_type type, std::int32_t shape, std::tuple<double, double, double>& tupleSize) const noexcept;
    bool GetObjectSubtype(object_type type, std::int32_t shape, object_subtype& emResult) const noexcept;

    double GetFrontOverhang(std::int32_t shape) const noexcept;

    //bool GetParkingSpot(std::int32_t id, double& length, double& width, double& angle) const noexcept;

    ~CCsvFileCtrl();

protected:
    explicit CCsvFileCtrl();

private:
    CEquipmentCsv::Pointer m_pEquipmentCsv;
    CPedestrianCsv::Pointer m_pPedestrianCsv;
    COtherCsv::Pointer m_pOtherCsv;
    CObstacleCsv::Pointer m_pObstacleCsv;
    CFacilityCsv::Pointer m_pFacilityCsv;
};

#define CsvFileCtrl  CCsvFileCtrl::GetInstance()
