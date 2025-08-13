#pragma once

#include <memory>
#include <unordered_map>

class CBusAccessor final
{
public:
    CBusAccessor(std::string_view busId, std::string_view key, std::string_view format);
    CBusAccessor(std::string_view busId, std::string_view key, std::string_view format, bool flag);
    virtual ~CBusAccessor();
    using SharedPtr = std::shared_ptr<CBusAccessor>;

    void* GetHeader() const noexcept;

    void* GetBody() const noexcept;

    const std::string& GetName() const noexcept;

protected:
    bool Open(std::string_view busId, std::string_view key, std::string_view format);
    void Close();
    void Split(std::vector<std::string>& result, const std::string_view split, const char separator) const noexcept;
    bool ParseFormat(std::string_view format);

    void Init(std::string_view busId, std::string_view key, std::string_view format);

protected:
    bool Valid;
    void* FileMapping;
    mutable void* Memory;
    unsigned long TotalSize;
    unsigned int BodyOffset;
    std::string m_strName;
    bool doubleBusFlag;
};
