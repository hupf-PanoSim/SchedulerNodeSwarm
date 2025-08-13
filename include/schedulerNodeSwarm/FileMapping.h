#pragma once

#include <Windows.h>
#include <string>
#include <memory>
#include <CommonDef.h>

class CFileMapping final
{
public:
    typedef std::unique_ptr<CFileMapping> Pointer;
    static CFileMapping::Pointer Create(
        const std::string& strName,
        const std::uint32_t nMaxSize = DEFAULT_SHARED_MEMORY_MAX_SIZE,
        const unsigned long dwDesiredAccess = FILE_MAP_ALL_ACCESS
    ) {
        return std::make_unique<CFileMapping>(strName, nMaxSize, dwDesiredAccess);
    }

    explicit CFileMapping(const std::string& strName, const std::uint32_t nMaxSize, const unsigned long dwDesiredAccess);

    virtual ~CFileMapping();

    const std::string& Name() const noexcept;

    bool Open();

    void Close();

    bool ReadByte(std::uint8_t& byteRead, std::uint32_t nOffset = 0);

    bool WriteByte(std::uint8_t byteWrite, std::uint32_t nOffset = 0);

    bool ReadInteger(std::uint32_t& nRead, std::uint32_t nOffset = 0);

    bool WriteInteger(std::uint32_t nWrite, std::uint32_t nOffset = 0);

    bool ReadData(std::uint8_t* pszData, std::uint32_t nDataLength, std::uint32_t nOffset = 0);

    bool WriteData(std::uint8_t* pszData, std::uint32_t nDataLength, std::uint32_t nOffset = 0);

    bool ReadData(std::string& strData, std::uint32_t nOffset = 0);

    void Clear() noexcept;

    void* Memory() const noexcept {
        return m_pMemory;
    }

protected:
    CFileMapping(const CFileMapping&) = delete;

    CFileMapping& operator= (const CFileMapping&) = delete;

private:
    const std::string FILE_MAPPING_NAME;
    const std::uint32_t MAX_SIZE;
    const unsigned long DESIRED_ACCESS;
    HANDLE m_hFileMapping;
    std::uint8_t* m_pMemory;
};
