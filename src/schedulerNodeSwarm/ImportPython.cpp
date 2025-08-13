#include <ImportPython.h>

#include <boost/format.hpp>
#include <boost/winapi/time.hpp>
#include <boost/algorithm/string.hpp>

#include <pybind11/stl.h>

#include <RuntimeSettings.h>
#include <SchedulerNode.h>


static void Debug(bool once = true);


PYBIND11_EMBEDDED_MODULE(DataInterface, m) {
    // `m` is a `pybind11::module_` which is used to bind functions and classes
    pybind11::enum_<object_type>(m, "object_type")
        .value("vehicle", object_type::equipment)
        //.value("pedestrian", object_type::pedestrian)
        //.value("other", object_type::other)
        //.value("obstacle", object_type::obstacle)
        .export_values();

    pybind11::enum_<object_subtype>(m, "object_subtype")
        .value("Car", object_subtype::Car)
        .value("Van", object_subtype::Van)
        .value("Bus", object_subtype::Bus)
        .value("OtherVehicle", object_subtype::OtherVehicle)
        .value("Pedestrian", object_subtype::Pedestrian)
        .value("NonMotorVehicle", object_subtype::NonMotorVehicle)
        .value("Others", object_subtype::Others)
        .export_values();

    m.def("getObjectSize", GetObjectSize, pybind11::return_value_policy::reference);
    m.def("getObjectSubtype", GetObjectSubtype, pybind11::return_value_policy::reference);

    m.def("stopSimulation", StopSimulation);

    m.def("debug", Debug, pybind11::arg("once") = true);
}


CImportPython::Pointer CImportPython::Create(const std::string& strStem, const std::string& strParameterName, MapParam&& mapParameter)
{
    return std::make_unique<CImportPython>(strStem, strParameterName, std::forward<MapParam>(mapParameter));
}

CImportPython::CImportPython(const std::string& strStem, const std::string& strParameterName, MapParam&& mapParameter)
    : CAbstractImport(strStem), m_strParameterName(strParameterName)
{
    m_mapParameter.clear();
    m_mapParameter.merge(mapParameter);
}

CImportPython::~CImportPython() noexcept
{
    m_mapParameter.clear();
}

bool CImportPython::Load()
{
    try {
        import_module = pybind11::module::import(GetStem().c_str());
        model_output = import_module.attr(PyModelOutput);
    }
    catch (std::runtime_error& _error) {
        printf("%s\n", _error.what());
        return false;
    }
    return true;
}

void CImportPython::GenerateUserData()
{
    using namespace pybind11::literals;

    user_data = pybind11::dict("time"_a = 0);

    user_data[param_key_bus_id] = CSchedulerNode::Instance()->GetBusId();
    user_data[param_key_outputPath] = RuntimeSettings.GetOutputPath();

    std::string strName = m_strParameterName;
    strName.append(".");
    strName.append(std::to_string(GetIndex()));
    user_data[param_key_name] = strName;

    pybind11::dict parameters;
    for (const auto& pairItem : m_mapParameter) {
        parameters[pairItem.first.c_str()] = pairItem.second;
    }
    user_data[param_key_parameters] = parameters;
}

void CImportPython::ModelStart()
{
    GenerateUserData();

    try {
        import_module.attr(PyModelStart)(user_data);
    }
    catch (std::runtime_error& _error) {
        printf("%s\n", _error.what());
    }
}

void CImportPython::ModelOutput(std::uint32_t nCurrentTime)
{
    try {
        user_data[param_key_time] = nCurrentTime;
        model_output(user_data);
    }
    catch (std::runtime_error& _error) {
        printf("%s\n", _error.what());
    }
}

void CImportPython::ModelTerminate(std::uint32_t nCurrentTime)
{
    try {
        user_data[param_key_time] = nCurrentTime;
        import_module.attr(PyModelTerminate)(user_data);

        if (DelFromPyModules()) {
            std::string strPythonCmd = "del sys.modules['";
            strPythonCmd.append(GetStem());
            strPythonCmd.append("']");
            PyRun_SimpleString(strPythonCmd.c_str());
        }
    }
    catch (std::runtime_error& _error) {
        printf("%s\n", _error.what());
    }
}

void Debug(bool once)
{
    if (CSchedulerNode::Instance()->PythonDebugEnable())
    {
        if (once)
        {
            CSchedulerNode::Instance()->DisablePythonDebug();
        }
        CSchedulerNode::Instance()->PauseSimulation();

        std::string_view script = "import pdb; pdb.set_trace()";
        PyRun_SimpleString(script.data());
    }

    //std::cout << "\nDebug(" << (once ? "True" : "False") << ") return\n" << std::endl;
}
