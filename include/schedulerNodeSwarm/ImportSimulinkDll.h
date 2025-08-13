#pragma once

#include <AbstractImport.h>


class CImportSimulinkDll final : public CAbstractImport
{
    DEFINE_STD_UNIQUE_PTR(CImportSimulinkDll)
public:
    static CImportSimulinkDll::Pointer Create(const std::string& strStem);
    explicit CImportSimulinkDll(const std::string& strStem);
    virtual ~CImportSimulinkDll() override;

    bool Load(const std::string& strFile);
    void ModelStart() override;
    void ModelOutput(std::uint32_t nCurrentTime) override;
    void ModelTerminate(std::uint32_t nCurrentTime) override;

protected:
    void Free();

protected:
    using fpModelStart = std::add_pointer<void()>::type;
    using fpModelOutput = std::add_pointer<void()>::type;
    using fpModelTerminate = std::add_pointer<void()>::type;

private:
    fpModelStart FunctionModelStart;
    fpModelOutput FunctionModelOutput;
    fpModelTerminate FunctionModelTerminate;

    void* m_hMobule;
    const char* const FunctionPrefix = "panosim_";
};
