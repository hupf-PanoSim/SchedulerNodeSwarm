#include <ImportSimulinkDll.h>
#include <Windows.h>
#include <SchedulerNode.h>


CImportSimulinkDll::Pointer CImportSimulinkDll::Create(const std::string& strStem)
{
    return std::make_unique<CImportSimulinkDll>(strStem);
}

CImportSimulinkDll::CImportSimulinkDll(const std::string& strStem) : CAbstractImport(strStem)
{
    m_hMobule = nullptr;
    FunctionModelStart = nullptr;
    FunctionModelOutput = nullptr;
    FunctionModelTerminate = nullptr;
}

CImportSimulinkDll::~CImportSimulinkDll()
{
    Free();
}

void CImportSimulinkDll::Free()
{
    if (m_hMobule != nullptr)
    {
        ::FreeLibrary(static_cast<HMODULE>(m_hMobule));
        m_hMobule = nullptr;
    }
}

bool CImportSimulinkDll::Load(const std::string& strFile)
{
    if (m_hMobule != nullptr)
    {
        return true;
    }
    m_hMobule = ::LoadLibrary(strFile.c_str());
    if (m_hMobule == nullptr)
    {
        return false;
    }

    std::string strFunctionStart(FunctionPrefix);
    strFunctionStart.append(GetStem());
    strFunctionStart.append("_initialize");
    FunctionModelStart = (fpModelStart)::GetProcAddress((HMODULE)(m_hMobule), strFunctionStart.c_str());
    if (FunctionModelStart == nullptr) {
        return false;
    }

    std::string strFunctionOutput(FunctionPrefix);
    strFunctionOutput.append(GetStem());
    strFunctionOutput.append("_step");
    FunctionModelOutput = (fpModelOutput)::GetProcAddress((HMODULE)(m_hMobule), strFunctionOutput.c_str());
    if (FunctionModelOutput == nullptr) {
        return false;
    }

    std::string strFunctionTerminate(FunctionPrefix);
    strFunctionTerminate.append(GetStem());
    strFunctionTerminate.append("_terminate");
    FunctionModelTerminate = (fpModelTerminate)::GetProcAddress((HMODULE)(m_hMobule), strFunctionTerminate.c_str());
    if (FunctionModelTerminate == nullptr) {
        return false;
    }

    return true;
}

void CImportSimulinkDll::ModelStart()
{
    try
    {
        FunctionModelStart();
    }
    catch (...)
    {
        printf("%s:ModelStart:Exception\n", GetStem().c_str());
    }
}

void CImportSimulinkDll::ModelOutput(std::uint32_t nCurrentTime)
{
    try
    {
        FunctionModelOutput();
    }
    catch (...)
    {
        printf("%s:ModelOutput:Exception:Time=%d\n", GetStem().c_str(), nCurrentTime);
    }
}

void CImportSimulinkDll::ModelTerminate(std::uint32_t nCurrentTime)
{
    try
    {
        FunctionModelTerminate();
    }
    catch (...)
    {
        printf("%s:ModelTerminate:Exception\n", GetStem().c_str());
    }
}
