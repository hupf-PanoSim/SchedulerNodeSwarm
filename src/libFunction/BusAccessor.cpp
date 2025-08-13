#include "pch.h"
#include "framework.h"
#include <Windows.h>

#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>

#include <Export.h>


static std::unordered_map<BusAccessor*, CBusAccessor::SharedPtr> g_mapAccessor;
static std::unordered_map<DoubleBusReader*, std::pair<CBusAccessor::SharedPtr, CBusAccessor::SharedPtr>> g_mapDoubleAccessor;

BusAccessor::BusAccessor(int busId, std::string_view key, std::string_view format)
{
    std::string strName = "panoswarm.";
    strName.append(std::to_string(busId));
    strName.append(".");
    strName.append(key);

    auto findAccessor = std::find_if(
        g_mapAccessor.begin(), g_mapAccessor.end(),
        [&](auto& pairItem) -> bool {
            return strName.compare(pairItem.second->GetName()) == 0;
        }
    );
    if (findAccessor == g_mapAccessor.end())
    {
        g_mapAccessor.insert(std::make_pair(this, std::make_shared<CBusAccessor>(std::to_string(busId), key, format)));
    }
    else {
        g_mapAccessor.insert(std::make_pair(this, findAccessor->second));
    }
}

BusAccessor::~BusAccessor()
{
    auto findAccessor = g_mapAccessor.find(const_cast<BusAccessor*>(this));
    if (findAccessor != g_mapAccessor.end()) {
        g_mapAccessor.erase(findAccessor);
    }
}

void* BusAccessor::GetHeader() const noexcept {
    auto findAccessor = g_mapAccessor.find(const_cast<BusAccessor*>(this));
    if (findAccessor != g_mapAccessor.end()) {
        return findAccessor->second->GetHeader();
    }

    return nullptr;
}

void* BusAccessor::GetBody() const noexcept {
    auto findAccessor = g_mapAccessor.find(const_cast<BusAccessor*>(this));
    if (findAccessor != g_mapAccessor.end()) {
        return findAccessor->second->GetBody();
    }
    return nullptr;
}

DoubleBusReader::DoubleBusReader(int busId, std::string_view key, std::string_view format)
{
    std::string strName = "panoswarm.";
    strName.append(std::to_string(busId));
    strName.append(".");
    strName.append(key);

    auto findAccessor = std::find_if(
        g_mapDoubleAccessor.begin(), g_mapDoubleAccessor.end(),
        [&](auto& pairItem) -> bool {
            return strName.compare(pairItem.second.first->GetName()) == 0;
        }
    );
    if (findAccessor == g_mapDoubleAccessor.end()) {
        std::string key0 = key.data();
        key0.append(".0");
        std::string key1 = key.data();
        key1.append(".1");
        auto reader0 = std::make_shared<CBusAccessor>(std::to_string(busId), key0, format, true);
        auto reader1 = std::make_shared<CBusAccessor>(std::to_string(busId), key1, format, false);
        g_mapDoubleAccessor.insert(std::make_pair(this, std::make_pair(std::move(reader0), std::move(reader1))));
    }
    else {
        g_mapDoubleAccessor.insert(std::make_pair(this, findAccessor->second));
    }
}

DoubleBusReader::~DoubleBusReader()
{}

CBusAccessor* DoubleBusReader::GetReader(std::uint32_t nCurrentTime) const noexcept
{
    auto findAccessor = g_mapDoubleAccessor.find(const_cast<DoubleBusReader*>(this));
    if (findAccessor != g_mapDoubleAccessor.end()) {
        auto reader0 = findAccessor->second.first;
        auto reader1 = findAccessor->second.second;
        std::uint32_t nTime0 = *(static_cast<std::uint32_t*>(reader0->GetHeader()));
        std::uint32_t nTime1 = *(static_cast<std::uint32_t*>(reader1->GetHeader()));
        if (nTime0 == nCurrentTime) {
            return reader0.get();
        }
        if (nTime1 == nCurrentTime) {
            return reader1.get();
        }
        if (nTime0 > nTime1) {
            return reader0.get();
        }
        return reader1.get();
    }
    return nullptr;
}

CBusAccessor::CBusAccessor(std::string_view busId, std::string_view key, std::string_view format)
{
    Init(busId, key, format);
}

CBusAccessor::CBusAccessor(std::string_view busId, std::string_view key, std::string_view format, bool flag)
{
    doubleBusFlag = flag;
    Init(busId, key, format);
}

CBusAccessor::~CBusAccessor()
{
    Close();
    BodyOffset = 0;
    TotalSize = 0;
}

void CBusAccessor::Init(std::string_view busId, std::string_view key, std::string_view format)
{
    Valid = false;
    FileMapping = nullptr;
    Memory = nullptr;
    TotalSize = 0;
    BodyOffset = 0;

    m_strName = "panoswarm.";
    m_strName.append(busId);
    m_strName.append(".");
    m_strName.append(key);

    if (ParseFormat(format)) {
        Open(busId, key, format);
    }
}

inline void* CBusAccessor::GetHeader() const noexcept
{
    if (!Valid) {
        return nullptr;
    }

    return Memory;
}

inline void* CBusAccessor::GetBody() const noexcept
{
    if (!Valid) {
        return nullptr;
    }

    return (unsigned char*)Memory + BodyOffset;
}

inline const std::string& CBusAccessor::GetName() const noexcept
{
    return m_strName;
}

bool CBusAccessor::Open(std::string_view busId, std::string_view key, std::string_view format)
{
    if (Valid) {
        return true;
    }

    HANDLE hFileMapping = ::CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        TotalSize,
        m_strName.data()
    );
    if (hFileMapping == NULL || hFileMapping == INVALID_HANDLE_VALUE) {
        return false;
    }

    void* pMemory = ::MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, TotalSize);
    if (pMemory == nullptr) {
        return false;
    }

    Memory = (BYTE*)(pMemory);
    std::memset(Memory, '\0', TotalSize);

    FileMapping = hFileMapping;
    Valid = true;

    return Valid;
}

void CBusAccessor::Close()
{
    if (Valid) {
        if (Memory != nullptr) {
            ::UnmapViewOfFile(Memory);
            Memory = nullptr;
        }

        if (FileMapping != nullptr && FileMapping != INVALID_HANDLE_VALUE) {
            ::CloseHandle(FileMapping);
            FileMapping = nullptr;
        }

        Valid = false;
    }
}

void CBusAccessor::Split(std::vector<std::string>& result, const std::string_view split, const char separator) const noexcept
{
    result.clear();
    std::string strSplit(split);
    std::stringstream stream_split(strSplit);
    std::string item;
    while (std::getline(stream_split, item, separator)) {
        result.push_back(item);
    }
}

bool CBusAccessor::ParseFormat(std::string_view format)
{
    TotalSize = 0;
    BodyOffset = 0;

    auto string2size = [&](const std::string& str, int& nSize) -> bool {
        nSize = 0;
        switch (str.front()) {
        case 'b':
            nSize = 1;
            break;
        case 'i':
        case '[':
            nSize = 4;
            break;
        case 'f':
            nSize = 4;
            break;
        case 'd':
            nSize = 8;
            break;
        default:
            break;
        }
        return nSize > 0;
    };

    auto format2size = [&](const std::vector<std::string>& vctFormat, int& nSize) -> bool {
        nSize = 0;
        for (auto& item : vctFormat) {
            std::vector<std::string> vctSplitItem;
            Split(vctSplitItem, item, '@');
            if (vctSplitItem.empty()) {
                return false;
            }
            int nItemSize = 0;
            if (string2size(vctSplitItem.back(), nItemSize)) {
                nSize += nItemSize;
            }
        }
        return nSize > 0;
    };

    auto parseHeader = [&](const std::vector<std::string>& vctFormat, int& nHeaderSize, int& nItemCount) -> bool {
        nHeaderSize = 0;
        nItemCount = 0;
        for (auto& item : vctFormat) {
            std::vector<std::string> vctSplitItem;
            Split(vctSplitItem, item, '@');
            if (vctSplitItem.empty()) {
                return false;
            }
            if (vctSplitItem.back().compare("[") == 0) {
                nItemCount = std::stoi(vctSplitItem.front());
                nHeaderSize += 4;
            }
            else {
                int nSize = 0;
                if (!string2size(vctSplitItem.back(), nSize)) {
                    return false;
                }
                nHeaderSize += nSize;
            }
        }
        return nHeaderSize > 0;
    };

    auto nPos = format.find("[");
    if (nPos == format.npos) {
        int nItemSize = 0;
        std::vector<std::string> vctForamt;
        Split(vctForamt, format, ',');
        if (!format2size(vctForamt, nItemSize)) {
            return false;
        }
        TotalSize = nItemSize;
    }
    else {
        std::string_view strHeader = format.substr(0, nPos + 1);
        std::vector<std::string> vctHeaderForamt;
        Split(vctHeaderForamt, strHeader, ',');
        int nHeaderSize = 0, nItemCount = 0;;
        if (!parseHeader(vctHeaderForamt, nHeaderSize, nItemCount)) {
            return false;
        }

        std::string_view strBodyWithSize = format.substr(nPos + 2, format.size() - nPos - 2);
        std::vector<std::string> vctBodyForamt;
        Split(vctBodyForamt, strBodyWithSize, ',');
        int nItemSize = 0;
        if (format2size(vctBodyForamt, nItemSize)) {
            TotalSize = nHeaderSize + nItemSize * nItemCount;
            BodyOffset = nHeaderSize;
        }
        else {
            return false;
        }
    }

    return TotalSize > 0;
}
