#include <FileMapping.h>


CFileMapping::CFileMapping(
    const std::string& strName, const std::uint32_t nMaxSize, const unsigned long dwDesiredAccess)
    : FILE_MAPPING_NAME(strName), MAX_SIZE(nMaxSize), DESIRED_ACCESS(dwDesiredAccess)
{
    m_hFileMapping = nullptr;
    m_pMemory = nullptr;
}

CFileMapping::~CFileMapping()
{
    Close();
}

inline const std::string& CFileMapping::Name() const noexcept
{
    return FILE_MAPPING_NAME;
}

bool CFileMapping::Open()
{
    HANDLE hFileMapping = ::CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        MAX_SIZE,
        FILE_MAPPING_NAME.c_str()
    );
    if (hFileMapping != NULL) {
        void* pMemory = ::MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, MAX_SIZE);
        if (pMemory != nullptr) {
            m_pMemory = (std::uint8_t*)(pMemory);
            m_hFileMapping = hFileMapping;
            return true;
        }
    }

    return false;
}

void CFileMapping::Close()
{
    if (m_pMemory != nullptr)
    {
        ::UnmapViewOfFile(m_pMemory);
        m_pMemory = nullptr;
    }
    if (m_hFileMapping != nullptr)
    {
        ::CloseHandle(m_hFileMapping);
        m_hFileMapping = nullptr;
    }
}

bool CFileMapping::ReadByte(std::uint8_t& byteRead, std::uint32_t nOffset)
{
    if (m_pMemory)
    {
        byteRead = m_pMemory[nOffset];
        return true;
    }

    return false;
}

bool CFileMapping::WriteByte(std::uint8_t byteWrite, std::uint32_t nOffset)
{
    if (m_pMemory)
    {
        m_pMemory[nOffset] = byteWrite;
        return true;
    }

    return false;
}

bool CFileMapping::ReadInteger(std::uint32_t& nRead, std::uint32_t nOffset)
{
    if (m_pMemory)
    {
        memcpy(&nRead, m_pMemory + nOffset, sizeof(nRead));
        return true;
    }

    return false;
}

bool CFileMapping::WriteInteger(std::uint32_t nWrite, std::uint32_t nOffset)
{
    if (m_pMemory)
    {
        memcpy(m_pMemory + nOffset, &nWrite, sizeof(nWrite));
        return true;
    }

    return false;
}

bool CFileMapping::ReadData(std::uint8_t* pszData, std::uint32_t nDataLength, std::uint32_t nOffset)
{
    if (m_pMemory)
    {
        memcpy(pszData, m_pMemory + nOffset, nDataLength);
        return true;
    }

    return false;
}

bool CFileMapping::WriteData(std::uint8_t* pszData, std::uint32_t nDataLength, std::uint32_t nOffset)
{
    if (m_pMemory)
    {
        memcpy(m_pMemory + nOffset, pszData, nDataLength);
        return true;
    }

    return false;
}

bool CFileMapping::ReadData(std::string& strData, std::uint32_t nOffset)
{
    strData.clear();
    if (m_pMemory)
    {
        const std::uint32_t MAX_DATA_LENGTH = 256;
        char szBuf[MAX_DATA_LENGTH + 1];
        memcpy(szBuf, m_pMemory + nOffset, MAX_DATA_LENGTH);
        szBuf[MAX_DATA_LENGTH] = '\0';
        strData.assign(szBuf);
        return !(strData.empty());
    }

    return false;
}

void CFileMapping::Clear() noexcept
{
    if (m_pMemory)
    {
        std::memset(m_pMemory, '\0', MAX_SIZE);
    }
}
