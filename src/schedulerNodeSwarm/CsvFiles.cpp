#include <CsvFiles.h>

#include <iostream>
#include <boost/algorithm/string.hpp>
#include <csv.h>


IMPLEMENT_SINGLETON(CCsvFileCtrl)

CCsvFileCtrl::CCsvFileCtrl()
{
    m_pEquipmentCsv = CEquipmentCsv::Create();
    m_pPedestrianCsv = CPedestrianCsv::Create();
    m_pOtherCsv = COtherCsv::Create();
    m_pObstacleCsv = CObstacleCsv::Create();
    m_pFacilityCsv = CFacilityCsv::Create();
}

CCsvFileCtrl::~CCsvFileCtrl()
{
    if (m_pEquipmentCsv)
    {
        m_pEquipmentCsv.reset();
    }
    if (m_pPedestrianCsv)
    {
        m_pPedestrianCsv.reset();
    }
    if (m_pOtherCsv)
    {
        m_pOtherCsv.reset();
    }
    if (m_pObstacleCsv)
    {
        m_pObstacleCsv.reset();
    }
    if (m_pFacilityCsv)
    {
        m_pFacilityCsv.reset();
    }
}

bool CCsvFileCtrl::ParseEquipmentCsv(const std::string& strVehicleCsvFile)
{
    return m_pEquipmentCsv->Parse(strVehicleCsvFile);
}

//bool CCsvFileCtrl::ParsePedestrianCsv(const std::string& strPedestrianCsvFile)
//{
//    return m_pPedestrianCsv->Parse(strPedestrianCsvFile);
//}
//
//bool CCsvFileCtrl::ParseOtherCsv(const std::string& strOtherCsvFile)
//{
//    return m_pOtherCsv->Parse(strOtherCsvFile);
//}
//
//bool CCsvFileCtrl::ParseObstacleCsv(const std::string& strObstacleCsvFile)
//{
//    return m_pObstacleCsv->Parse(strObstacleCsvFile);
//}
//
//bool CCsvFileCtrl::ParseFacility(const std::string& strFacilityCsvFile)
//{
//    return m_pFacilityCsv->Parse(strFacilityCsvFile);
//}

bool CCsvFileCtrl::GetObjectSize(object_type type, std::int32_t shape, std::tuple<double, double, double>& tupleSize) const noexcept
{
    switch (type)
    {
    case object_type::equipment:
        return m_pEquipmentCsv->GetObjectSize(shape, tupleSize);
        break;
    //case object_type::pedestrian:
    //    return m_pPedestrianCsv->GetObjectSize(shape, tupleSize);
    //    break;
    //case object_type::other:
    //    return m_pOtherCsv->GetObjectSize(shape, tupleSize);
    //    break;
    //case object_type::obstacle:
    //    return m_pObstacleCsv->GetObjectSize(shape, tupleSize);
    //    break;
    //case object_type::unknown:
    //    break;
    default:
        break;
    }
    return false;
}

bool CCsvFileCtrl::GetObjectSubtype(object_type type, std::int32_t shape, object_subtype& emResult) const noexcept
{
    switch (type)
    {
    case object_type::equipment:
        return m_pEquipmentCsv->GetObjectSubtype(type, shape, emResult);
        break;
    //case object_type::pedestrian:
    //    return m_pPedestrianCsv->GetObjectSubtype(type, shape, emResult);
    //    break;
    //case object_type::other:
    //    return m_pOtherCsv->GetObjectSubtype(type, shape, emResult);
    //    break;
    case object_type::unknown:
    default:
        emResult = object_subtype::unknown;
        return true;
        break;
    }
    
    return false;
}

double CCsvFileCtrl::GetFrontOverhang(std::int32_t shape) const noexcept
{
    return m_pEquipmentCsv->GetFrontOverhang(shape);
}

//bool CCsvFileCtrl::GetParkingSpot(std::int32_t id, double& length, double& width, double& angle) const noexcept
//{
//    return m_pFacilityCsv->GetParkingSpot(id, length, width, angle);
//}

////----CEquipmentCsv----//////////////////////////////////////////////////////////////////////
CREATE_STD_UNIQUE_IMPLEMENT(CEquipmentCsv)

CEquipmentCsv::CEquipmentCsv()
{
    Clear();
}

CEquipmentCsv::~CEquipmentCsv()
{
    Clear();
}

void CEquipmentCsv::Clear() noexcept
{
    m_strCsvFile.clear();
    m_vctAll.clear();
}

bool CEquipmentCsv::Parse(const std::string& strCsvFile)
{
    constexpr std::int32_t column_count = 11;

    m_vctAll.clear();
    CEquipmentCsvData::Pointer pCsvData;

    try {
        io::CSVReader<column_count> csvFile(strCsvFile);
        csvFile.next_line();// skip header
        do
        {
            pCsvData = CEquipmentCsvData::Create();
            if (pCsvData) {
                if (!csvFile.read_row(
                    pCsvData->nId,
                    pCsvData->strName,
                    pCsvData->strType,
                    pCsvData->strLength,
                    pCsvData->strWidth,
                    pCsvData->strHeight,
                    pCsvData->strFrontOverhang,
                    pCsvData->strPackName,
                    pCsvData->strLightControl,
                    pCsvData->strDisplayName,
                    pCsvData->strIcon
                )) {
                    break;
                }

                if (boost::algorithm::iequals(pCsvData->strType, typeCar)) {
                    pCsvData->emSubtype = object_subtype::Car;
                }
                else if (boost::algorithm::iequals(pCsvData->strType, typeVan)) {
                    pCsvData->emSubtype = object_subtype::Van;
                }
                else if (boost::algorithm::iequals(pCsvData->strType, typeBus)) {
                    pCsvData->emSubtype = object_subtype::Bus;
                }
                else if (boost::algorithm::iequals(pCsvData->strType, typeOther)) {
                    pCsvData->emSubtype = object_subtype::OtherVehicle;
                }
                m_vctAll.emplace_back(std::move(pCsvData));
            }
        } while (true);
    }
    catch (io::error::base& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }

    m_strCsvFile = strCsvFile;
    return true;
}

bool CEquipmentCsv::GetObjectSize(std::int32_t shape, std::tuple<double, double, double>& tupleSize) const noexcept
{
    std::get<0>(tupleSize) = 0;
    std::get<1>(tupleSize) = 0;
    std::get<2>(tupleSize) = 0;

    auto findId = std::find_if(
        m_vctAll.begin(), m_vctAll.end(), [&](const auto& pItem) -> bool {
            return pItem->nId == shape;
        }
    );
    if (findId != m_vctAll.end())
    {
        const auto& pItem = *findId;
        std::get<0>(tupleSize) = std::stod(pItem->strLength);
        std::get<1>(tupleSize) = std::stod(pItem->strWidth);
        std::get<2>(tupleSize) = std::stod(pItem->strHeight);
        return true;
    }

    return false;
}

bool CEquipmentCsv::GetObjectSubtype(object_type type, std::int32_t shape, object_subtype& emResult) const noexcept
{
    auto findId = std::find_if(
        m_vctAll.begin(), m_vctAll.end(), [&](const auto& pItem) -> bool {
            return pItem->nId == shape;
        }
    );
    if (findId != m_vctAll.end())
    {
        emResult = (*findId)->emSubtype;
        return true;
    }

    return false;
}

double CEquipmentCsv::GetFrontOverhang(std::int32_t shape) const noexcept
{
    auto findId = std::find_if(
        m_vctAll.begin(), m_vctAll.end(), [&](const auto& pItem) -> bool {
            return pItem->nId == shape;
        }
    );
    if (findId != m_vctAll.end())
    {
        std::string strFrontOverhang = (*findId)->strFrontOverhang;
        return std::stod(strFrontOverhang);
    }

    return 0;
}


////----CPedestrianCsv----//////////////////////////////////////////////////////////////////////
CREATE_STD_UNIQUE_IMPLEMENT(CPedestrianCsv)

CPedestrianCsv::CPedestrianCsv()
{
    Clear();
}

CPedestrianCsv::~CPedestrianCsv()
{
    Clear();
}

void CPedestrianCsv::Clear() noexcept
{
    m_strCsvFile.clear();
    m_vctAll.clear();
}

bool CPedestrianCsv::Parse(const std::string& strCsvFile)
{
    constexpr std::int32_t column_count = 9;

    m_vctAll.clear();
    CPedestrianCsvData::Pointer pCsvData;

    try {
        io::CSVReader<column_count> csvFile(strCsvFile);
        csvFile.next_line();// skip header
        do {
            pCsvData = CPedestrianCsvData::Create();
            if (pCsvData) {
                if (!csvFile.read_row(
                    pCsvData->nId,
                    pCsvData->strName,
                    pCsvData->strType,
                    pCsvData->strLength,
                    pCsvData->strWidth,
                    pCsvData->strHeight,
                    pCsvData->strPackName,
                    pCsvData->strDisplayName,
                    pCsvData->strIcon
                )) {
                    break;
                }

                pCsvData->emSubtype = object_subtype::Pedestrian;
                m_vctAll.emplace_back(std::move(pCsvData));
            }
        } while (true);
    }
    catch (io::error::base& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }

    m_strCsvFile = strCsvFile;
    return true;
}

bool CPedestrianCsv::GetObjectSize(std::int32_t shape, std::tuple<double, double, double>& tupleSize) const noexcept
{
    std::get<0>(tupleSize) = 0;
    std::get<1>(tupleSize) = 0;
    std::get<2>(tupleSize) = 0;

    auto findId = std::find_if(
        m_vctAll.begin(), m_vctAll.end(), [&](const auto& pItem) -> bool {
            return pItem->nId == shape;
        }
    );
    if (findId != m_vctAll.end())
    {
        const auto& pItem = *findId;
        std::get<0>(tupleSize) = std::stod(pItem->strLength);
        std::get<1>(tupleSize) = std::stod(pItem->strWidth);
        std::get<2>(tupleSize) = std::stod(pItem->strHeight);
        return true;
    }

    return false;
}

bool CPedestrianCsv::GetObjectSubtype(object_type type, std::int32_t shape, object_subtype& emResult) const noexcept
{
    auto findId = std::find_if(
        m_vctAll.begin(), m_vctAll.end(), [&](const auto& pItem) -> bool {
            return pItem->nId == shape;
        }
    );
    if (findId != m_vctAll.end())
    {
        emResult = (*findId)->emSubtype;
        return true;
    }

    return false;
}


////----COtherCsv----//////////////////////////////////////////////////////////////////////

CREATE_STD_UNIQUE_IMPLEMENT(COtherCsv)

COtherCsv::COtherCsv()
{
    Clear();
}

COtherCsv::~COtherCsv()
{
    Clear();
}

void COtherCsv::Clear() noexcept
{
    m_strCsvFile.clear();
    m_vctAll.clear();
}

bool COtherCsv::Parse(const std::string& strCsvFile)
{
    constexpr std::int32_t column_count = 9;

    m_vctAll.clear();

    COtherCsvData::Pointer pCsvData;

    try {
        io::CSVReader<column_count> csvFile(strCsvFile);
        csvFile.next_line();// skip header
        do {
            pCsvData = COtherCsvData::Create();
            if (pCsvData) {
                if (!csvFile.read_row(
                    pCsvData->nId,
                    pCsvData->strName,
                    pCsvData->strType,
                    pCsvData->strLength,
                    pCsvData->strWidth,
                    pCsvData->strHeight,
                    pCsvData->strPackName,
                    pCsvData->strDisplayName,
                    pCsvData->strIcon
                )) {
                    break;
                }

                if (boost::algorithm::iequals(pCsvData->strType, typeOther)) {
                    pCsvData->emSubtype = object_subtype::Others;
                }
                else if (boost::algorithm::iequals(pCsvData->strType, typeNonMotor)) {
                    pCsvData->emSubtype = object_subtype::NonMotorVehicle;
                }
                m_vctAll.emplace_back(std::move(pCsvData));
            }
        } while (true);
    }
    catch (io::error::base& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }

    m_strCsvFile = strCsvFile;
    return true;
}

bool COtherCsv::GetObjectSize(std::int32_t shape, std::tuple<double, double, double>& tupleSize) const noexcept
{
    std::get<0>(tupleSize) = 0;
    std::get<1>(tupleSize) = 0;
    std::get<2>(tupleSize) = 0;

    auto findId = std::find_if(
        m_vctAll.begin(), m_vctAll.end(), [&](const auto& pItem) -> bool {
            return pItem->nId == shape;
        }
    );
    if (findId != m_vctAll.end())
    {
        const auto& pItem = *findId;
        std::get<0>(tupleSize) = std::stod(pItem->strLength);
        std::get<1>(tupleSize) = std::stod(pItem->strWidth);
        std::get<2>(tupleSize) = std::stod(pItem->strHeight);
        return true;
    }

    return false;
}

bool COtherCsv::GetObjectSubtype(object_type type, std::int32_t shape, object_subtype& emResult) const noexcept
{
    auto findId = std::find_if(
        m_vctAll.begin(), m_vctAll.end(), [&](const auto& pItem) -> bool {
            return pItem->nId == shape;
        }
    );
    if (findId != m_vctAll.end())
    {
        emResult = (*findId)->emSubtype;
        return true;
    }

    return false;
}


////----CObstacleCsv----//////////////////////////////////////////////////////////////////////
CREATE_STD_UNIQUE_IMPLEMENT(CObstacleCsv)
CObstacleCsv::CObstacleCsv()
{
    Clear();
}

CObstacleCsv::~CObstacleCsv()
{
    Clear();
}

void CObstacleCsv::Clear() noexcept
{
    m_strCsvFile.clear();
    m_vctAll.clear();
}

bool CObstacleCsv::Parse(const std::string& strCsvFile)
{
    constexpr std::int32_t column_count = 8;

    m_vctAll.clear();

    CObstacleCsvData::Pointer pCsvData;

    try {
        io::CSVReader<column_count> csvFile(strCsvFile);
        csvFile.next_line();// skip header
        do {
            pCsvData = CObstacleCsvData::Create();
            if (pCsvData) {
                if (!csvFile.read_row(
                    pCsvData->nId,
                    pCsvData->strName,
                    pCsvData->strLength,
                    pCsvData->strWidth,
                    pCsvData->strHeight,
                    pCsvData->strPackName,
                    pCsvData->strDisplayName,
                    pCsvData->strIcon
                )) {
                    break;
                }

                m_vctAll.emplace_back(std::move(pCsvData));
            }
        } while (true);
    }
    catch (io::error::base& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }

    m_strCsvFile = strCsvFile;
    return true;
}

bool CObstacleCsv::GetObjectSize(std::int32_t shape, std::tuple<double, double, double>& tupleSize) const noexcept
{
    std::get<0>(tupleSize) = 0;
    std::get<1>(tupleSize) = 0;
    std::get<2>(tupleSize) = 0;

    auto findId = std::find_if(
        m_vctAll.begin(), m_vctAll.end(), [&](const auto& pItem) -> bool {
            return pItem->nId == shape;
        }
    );
    if (findId != m_vctAll.end())
    {
        const auto& pItem = *findId;
        std::get<0>(tupleSize) = std::stod(pItem->strLength);
        std::get<1>(tupleSize) = std::stod(pItem->strWidth);
        std::get<2>(tupleSize) = std::stod(pItem->strHeight);
        return true;
    }

    return false;
}


////----CFacilityCsv----//////////////////////////////////////////////////////////////////////
CREATE_STD_UNIQUE_IMPLEMENT(CFacilityCsv)
CFacilityCsv::CFacilityCsv()
{
    Clear();
}

CFacilityCsv::~CFacilityCsv()
{
    Clear();
}

void CFacilityCsv::Clear() noexcept
{
    m_strCsvFile.clear();
    m_vctAll.clear();
}

bool CFacilityCsv::Parse(const std::string& strCsvFile)
{
    constexpr std::int32_t column_count = 9;

    m_vctAll.clear();

    CFacilityCsvData::Pointer pCsvData = nullptr;

    constexpr char const* FilterType = "ParkingSpot";

    try {
        io::CSVReader<column_count> csvFile(strCsvFile);
        csvFile.next_line();// skip header
        do {
            pCsvData = CFacilityCsvData::Create();
            if (pCsvData) {
                if (!csvFile.read_row(
                    pCsvData->nId,
                    pCsvData->strName,
                    pCsvData->strLength,
                    pCsvData->strWidth,
                    pCsvData->strAngle,
                    pCsvData->strType,
                    pCsvData->strPackName,
                    pCsvData->strDisplayName,
                    pCsvData->strIcon
                )) {
                    break;
                }

                if (pCsvData->strType.compare(FilterType) == 0) {
                    m_vctAll.emplace_back(std::move(pCsvData));
                }
            }
        } while (true);
    }
    catch (io::error::base& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }

    m_strCsvFile = strCsvFile;
    return true;
}

//bool CFacilityCsv::GetParkingSpot(std::int32_t id, double& length, double& width, double& angle) const noexcept
//{
//    length = 0;
//    width = 0;
//    angle = 0;
//
//    auto findId = std::find_if(
//        m_vctAll.cbegin(), m_vctAll.cend(), [&](const auto& pItem) -> bool {
//            return pItem->nId == id;
//        }
//    );
//    if (findId == m_vctAll.end())
//    {
//        return false;
//    }
//
//    const auto& pItem = *findId;
//    length = std::stod(pItem->strLength);
//    width = std::stod(pItem->strWidth);
//    angle = std::stod(pItem->strAngle);
//
//    return true;
//}

////----CEquipmentCsvData----//////////////////////////////////////////////////////////////////////
const CEquipmentCsvData::Pointer CEquipmentCsvData::NullPointer;
CREATE_STD_UNIQUE_IMPLEMENT(CEquipmentCsvData)

CEquipmentCsvData::CEquipmentCsvData() noexcept
{
    Clear();
}

CEquipmentCsvData::~CEquipmentCsvData() noexcept
{
    Clear();
}

void CEquipmentCsvData::Clear() noexcept
{
    nId = 0;
    strName.clear();
    emSubtype = object_subtype::unknown;
    strType.clear();
    strLength.clear();
    strWidth.clear();
    strHeight.clear();
    strFrontOverhang.clear();
    strPackName.clear();
    strLightControl.clear();

    strDisplayName.clear();
    strIcon.clear();
}

////----CPedestrianCsvData----//////////////////////////////////////////////////////////////////////
const CPedestrianCsvData::Pointer CPedestrianCsvData::NullPointer;
CREATE_STD_UNIQUE_IMPLEMENT(CPedestrianCsvData)

CPedestrianCsvData::CPedestrianCsvData() noexcept
{
    Clear();
}

CPedestrianCsvData::~CPedestrianCsvData() noexcept
{
    Clear();
}

void CPedestrianCsvData::Clear() noexcept
{
    nId = 0;
    strName.clear();
    emSubtype = object_subtype::unknown;
    strType.clear();
    strLength.clear();
    strWidth.clear();
    strHeight.clear();
    strPackName.clear();

    strDisplayName.clear();
    strIcon.clear();
}

////----COtherCsvData----//////////////////////////////////////////////////////////////////////
const COtherCsvData::Pointer COtherCsvData::NullPointer;
CREATE_STD_UNIQUE_IMPLEMENT(COtherCsvData)

COtherCsvData::COtherCsvData() noexcept
{
    Clear();
}

COtherCsvData::~COtherCsvData() noexcept
{
    Clear();
}

void COtherCsvData::Clear() noexcept
{
    nId = 0;
    strName.clear();
    emSubtype = object_subtype::unknown;
    strType.clear();
    strLength.clear();
    strWidth.clear();
    strHeight.clear();
    strPackName.clear();

    strDisplayName.clear();
    strIcon.clear();
}

////----CObstacleCsvData----//////////////////////////////////////////////////////////////////////
const CObstacleCsvData::Pointer CObstacleCsvData::NullPointer;
CREATE_STD_UNIQUE_IMPLEMENT(CObstacleCsvData)

CObstacleCsvData::CObstacleCsvData() noexcept
{
    Clear();
}

CObstacleCsvData::~CObstacleCsvData() noexcept
{
    Clear();
}

void CObstacleCsvData::Clear() noexcept
{
    nId = 0;
    strName.clear();
    strLength.clear();
    strWidth.clear();
    strHeight.clear();
    strPackName.clear();

    strDisplayName.clear();
    strIcon.clear();
}


////----CFacilityCsvData----//////////////////////////////////////////////////////////////////////
const CFacilityCsvData::Pointer CFacilityCsvData::NullPointer;
CREATE_STD_UNIQUE_IMPLEMENT(CFacilityCsvData)

CFacilityCsvData::CFacilityCsvData() noexcept
{
    Clear();
}

CFacilityCsvData::~CFacilityCsvData() noexcept
{
    Clear();
}

void CFacilityCsvData::Clear() noexcept
{
    nId = 0;
    strName.clear();
    strLength.clear();
    strWidth.clear();
    strAngle.clear();
    strType.clear();
    strPackName.clear();
    strDisplayName.clear();
    strIcon.clear();
}
