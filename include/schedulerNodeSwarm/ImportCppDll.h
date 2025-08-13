#pragma once

#include <AbstractImport.h>


class CImportCppDll final : public CAbstractImport
{
    DEFINE_STD_UNIQUE_PTR(CImportCppDll)
public:
    static CImportCppDll::Pointer Create(const std::string& strStem, const std::string& strParameterName, MapParam&& mapParameter);
    explicit CImportCppDll(const std::string& strStem, const std::string& strParameterName, MapParam&& mapParameter);
    virtual ~CImportCppDll() override;

    bool Load(const std::string& strFile);
    bool Initialize();
    void ModelStart() override;
    void ModelOutput(std::uint32_t nCurrentTime) override;
    void ModelTerminate(std::uint32_t nCurrentTime) override;

protected:
    static std::string AppendDllSuffix(const std::string& strDllFileName);
    void GenerateFunctionMap(MapFunction& functions);
    void Free();

protected:
    using fpInitialize = std::add_pointer<bool(const VectorParam&, const MapFunction&)>::type;
    using fpModelStart = std::add_pointer<void(UserData*)>::type;
    using fpModelOutput = std::add_pointer<void(UserData*)>::type;
    using fpModelTerminate = std::add_pointer<void(UserData*)>::type;

private:
    const std::string m_strParameterName;
    MapParam m_mapParameter;
    void* m_hMobule;
    UserData user_data;

    fpInitialize FunctionInitialize;
    fpModelStart FunctionModelStart;
    fpModelOutput FunctionModelOutput;
    fpModelTerminate FunctionModelTerminate;

    static const char* const FunctionNameInitialize;
    static const char* const FunctionNameModelStart;
    static const char* const FunctionNameModelOutput;
    static const char* const FunctionNameModelTerminate;
};
