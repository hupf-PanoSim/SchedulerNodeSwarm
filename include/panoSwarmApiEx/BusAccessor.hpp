#pragma once

#include <Windows.h>
#include <sstream>
#include <vector>
#include "bus_format.hpp"


template<typename Format>
class BusAccessor final {
public:
    using SharedPtr = std::shared_ptr<BusAccessor<Format>>;
    BusAccessor(std::int32_t busId, std::string_view key) {
        Init(std::to_string(busId), key, panosim_bus::BusFormatTrait<Format>::str());
    }
    BusAccessor(std::string busId, std::string_view key) {
        Init(busId, key, panosim_bus::BusFormatTrait<Format>::str());
    }
    virtual ~BusAccessor() {
        TotalSize = 0;
        HeaderSize = 0;
        BodyItemSize = 0;
        Close();
    }
    void* GetHeader() const noexcept {
        if (!Valid) {
            return nullptr;
        }
        return Memory;
    }
    void* GetBody(std::uint32_t index) const noexcept {
        if (!Valid) {
            return nullptr;
        }
        return (unsigned char*)(Memory)+(HeaderSize + index * BodyItemSize);
    }
    const std::string& GetName() const noexcept {
        return m_strName;
    }

    auto ReadHeader() const {
        return panosim_bus::read_header(
            BusFormat(panosim_bus::BusFormatTrait<Format>::str()),
            reinterpret_cast<char*>(GetHeader())
        );
    }

    auto ReadBody(std::uint32_t index) const {
        return panosim_bus::read_body(
            BusFormat(panosim_bus::BusFormatTrait<Format>::str()),
            reinterpret_cast<char*>(GetBody(index))
        );
    }

    template <typename... Args>
    void WriteHeader(Args&&... args) {
        panosim_bus::write_header(
            BusFormat(panosim_bus::BusFormatTrait<Format>::str()),
            reinterpret_cast<char*>(GetHeader()),
            std::forward<Args>(args)...
        );
    }

    template <typename... Args>
    void WriteBody(std::uint32_t index, Args&&... args) {
        panosim_bus::write_body(
            BusFormat(panosim_bus::BusFormatTrait<Format>::str()),
            reinterpret_cast<char*>(GetBody(index)),
            std::forward<Args>(args)...
        );
    }

protected:
    bool Open(std::string_view busId, std::string_view key, std::string_view format) {
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
    void Close() {
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
    void Split(std::vector<std::string>& result, const std::string_view split, const char separator) const noexcept {
        result.clear();
        std::string strSplit(split);
        std::stringstream stream_split(strSplit);
        std::string item;
        while (std::getline(stream_split, item, separator)) {
            result.push_back(item);
        }
    }
    bool ParseFormat(std::string_view format) {
        TotalSize = 0;
        HeaderSize = 0;
        BodyItemSize = 0;

        auto string2size = [&](const std::string& str, std::int32_t& nSize) -> bool {
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
        auto format2size = [&](const std::vector<std::string>& vctFormat, std::int32_t& nSize) -> bool {
            nSize = 0;
            for (auto& item : vctFormat) {
                std::vector<std::string> vctSplitItem;
                Split(vctSplitItem, item, '@');
                if (vctSplitItem.empty()) {
                    return false;
                }
                std::int32_t nItemSize = 0;
                if (string2size(vctSplitItem.back(), nItemSize)) {
                    nSize += nItemSize;
                }
            }
            return nSize > 0;
        };
        auto parseHeader = [&](const std::vector<std::string>& vctFormat, std::int32_t& nHeaderSize, std::int32_t& nItemCount) -> bool {
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
                    std::int32_t nSize = 0;
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
            std::int32_t nItemSize = 0;
            std::vector<std::string> vctForamt;
            Split(vctForamt, format, ',');
            if (!format2size(vctForamt, nItemSize)) {
                return false;
            }
            TotalSize = nItemSize;
            HeaderSize = TotalSize;
        }
        else {
            std::string_view strHeader = format.substr(0, nPos + 1);
            std::vector<std::string> vctHeaderForamt;
            Split(vctHeaderForamt, strHeader, ',');
            std::int32_t nHeaderSize = 0, nItemCount = 0;;
            if (!parseHeader(vctHeaderForamt, nHeaderSize, nItemCount)) {
                return false;
            }

            std::string_view strBodyWithSize = format.substr(nPos + 2, format.size() - nPos - 2);
            std::vector<std::string> vctBodyForamt;
            Split(vctBodyForamt, strBodyWithSize, ',');
            std::int32_t nItemSize = 0;
            if (format2size(vctBodyForamt, nItemSize)) {
                HeaderSize = nHeaderSize;
                BodyItemSize = nItemSize;
                TotalSize = HeaderSize + nItemSize * nItemCount;
            }
            else {
                return false;
            }
        }
        return TotalSize > 0;
    }
    void Init(std::string_view busId, std::string_view key, std::string_view format) {
        Valid = false;
        FileMapping = nullptr;
        Memory = nullptr;
        TotalSize = 0;
        HeaderSize = 0;
        BodyItemSize = 0;

        m_strName = "panosim.";
        m_strName.append(busId);
        m_strName.append(".");
        m_strName.append(key);

        if (ParseFormat(format)) {
            Open(busId, key, format);
        }
    }

protected:
    bool Valid;
    void* FileMapping;
    mutable void* Memory;
    std::string m_strName;
    std::uint32_t TotalSize;
    std::uint32_t HeaderSize;
    std::uint32_t BodyItemSize;
};


template<typename Format>
class DoubleBusReader final {
public:
    explicit DoubleBusReader(std::int32_t busId, std::string_view key) {
        std::string key0 = key.data();
        key0.append(".0");
        std::string key1 = key.data();
        key1.append(".1");
        reader0 = new BusAccessor<Format>(std::to_string(busId), key0);
        reader1 = new BusAccessor<Format>(std::to_string(busId), key1);
    }

    virtual ~DoubleBusReader() {
        if (reader0) {
            delete reader0;
            reader0 = nullptr;
        }
        if (reader1) {
            delete reader1;
            reader1 = nullptr;
        }
    }

    auto GetReader(std::uint32_t nCurrentTime) const noexcept {
        auto nTime0 = *(static_cast<std::uint32_t*>(reader0->GetHeader()));
        auto nTime1 = *(static_cast<std::uint32_t*>(reader1->GetHeader()));
        if (nTime0 == nCurrentTime) {
            return reader0;
        }
        if (nTime1 == nCurrentTime) {
            return reader1;
        }
        if (nTime0 > nTime1) {
            return reader0;
        }
        return reader1;
    }

protected:
    BusAccessor<Format>* reader0;
    BusAccessor<Format>* reader1;
};
