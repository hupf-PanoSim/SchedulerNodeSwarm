#pragma once

#include <pybind11/embed.h>

#include <AbstractImport.h>


class CImportPython final : public CAbstractImport
{
    DEFINE_STD_UNIQUE_PTR(CImportPython)
public:
    static CImportPython::Pointer Create(const std::string& strStem, const std::string& strParameterName, MapParam&& mapParameter);
    explicit CImportPython(const std::string& strStem, const std::string& strParameterName, MapParam&& mapParameter);
    virtual ~CImportPython() noexcept override;
    bool Load();
    void ModelStart() override;
    void ModelOutput(std::uint32_t nCurrentTime) override;
    void ModelTerminate(std::uint32_t nCurrentTime) override;

protected:
    void GenerateUserData();

private:
    const std::string m_strParameterName;
    MapParam m_mapParameter;
    pybind11::dict user_data;
    pybind11::module import_module;

    pybind11::object model_output;

    const char* const PyModelStart = "ModelStart";
    const char* const PyModelOutput = "ModelOutput";
    const char* const PyModelTerminate = "ModelTerminate";

    const char* const param_key_time = "time";
    const char* const param_key_bus_id = "busId";
    const char* const param_key_name = "name";
    const char* const param_key_outputPath = "outputPath";
    const char* const param_key_parameters = "parameters";
};
