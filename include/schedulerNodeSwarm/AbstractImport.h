#pragma once

#include <PointerDef.h>
#include <Export.h>

using MapParam = std::map<std::string, std::string>;

class CAbstractImport
{
    DEFINE_STD_UNIQUE_PTR(CAbstractImport)
public:
    explicit CAbstractImport(const std::string& strStem);
    virtual ~CAbstractImport() = default;
    virtual void ModelStart() = 0;
    virtual void ModelOutput(std::uint32_t nCurrentTime) = 0;
    virtual void ModelTerminate(std::uint32_t nCurrentTime) = 0;

    virtual const std::string& GetStem() const noexcept;
    virtual void SetIndex(std::int32_t nIndex) noexcept;
    virtual std::int32_t GetIndex() const noexcept;

    void DelFromPyModules(bool bDelFromModules) noexcept {
        m_bDelFromModules = bDelFromModules;
    }
    bool DelFromPyModules() const noexcept {
        return m_bDelFromModules;
    }

private:
    std::int32_t m_nIndex;
    const std::string m_strStem;
    bool m_bDelFromModules;
};
