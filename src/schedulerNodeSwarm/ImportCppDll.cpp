#include <ImportCppDll.h>

#ifdef _WIN32
#include <Windows.h>
#include <excpt.h>
#endif // _WIN32

#include <SchedulerNode.h>
#include <RuntimeSettings.h>

#define INSERT_FUNCTION(f)	functions.insert(std::make_pair(FunctionName::##f, f))

#ifdef _WIN32
static int FilterException(unsigned long exceptionCode, PEXCEPTION_POINTERS ptrException) {
    if (exceptionCode == EXCEPTION_ACCESS_VIOLATION) {
        printf("ModelOutput->access violation!\n");
        return EXCEPTION_EXECUTE_HANDLER;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}
static void ModelOutputWithException(std::add_pointer<void(UserData*)>::type fn, UserData* pUserData) {
    __try {
        __try {
            fn(pUserData);
        }
        __finally {
            AbnormalTermination();
        }
    }
    __except (FilterException(GetExceptionCode(), GetExceptionInformation())) {
        printf("plugin(%s)->time(%d)->runtime exception!\n", pUserData->name.c_str(), pUserData->time);
    }
}
#endif // _WIN32


const char* const CImportCppDll::FunctionNameInitialize = "Initialize";
const char* const CImportCppDll::FunctionNameModelStart = "ModelStart";
const char* const CImportCppDll::FunctionNameModelOutput = "ModelOutput";
const char* const CImportCppDll::FunctionNameModelTerminate = "ModelTerminate";


CImportCppDll::Pointer CImportCppDll::Create(const std::string& strStem, const std::string& strParameterName, MapParam&& mapParameter)
{
    return std::make_unique<CImportCppDll>(strStem, strParameterName, std::forward<MapParam>(mapParameter));
}

CImportCppDll::CImportCppDll(const std::string& strStem, const std::string& strParameterName, MapParam&& mapParameter)
    : CAbstractImport(strStem), m_strParameterName(strParameterName)
{
    m_mapParameter.clear();
    m_mapParameter.merge(mapParameter);

    m_hMobule = nullptr;

    user_data.time = 0;
    user_data.busId = 0;
    user_data.state = nullptr;
    user_data.parameters.clear();
    user_data.name.clear();
    user_data.outputPath.clear();

    FunctionInitialize = nullptr;
    FunctionModelStart = nullptr;
    FunctionModelOutput = nullptr;
    FunctionModelTerminate = nullptr;
}

CImportCppDll::~CImportCppDll()
{
    Free();

    if (user_data.state != nullptr)
    {
        delete[] user_data.state;
        user_data.state = nullptr;
    }
    m_mapParameter.clear();
}

void CImportCppDll::Free()
{
    if (m_hMobule != nullptr)
    {
        ::FreeLibrary(static_cast<HMODULE>(m_hMobule));
        m_hMobule = nullptr;
    }
}

bool CImportCppDll::Load(const std::string& strFile)
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

    FunctionInitialize = (fpInitialize)::GetProcAddress((HMODULE)(m_hMobule), FunctionNameInitialize);
    if (FunctionInitialize == nullptr)
    {
        return false;
    }

    FunctionModelStart = (fpModelStart)::GetProcAddress((HMODULE)(m_hMobule), FunctionNameModelStart);
    if (FunctionModelStart == nullptr) {
        return false;
    }

    FunctionModelOutput = (fpModelOutput)::GetProcAddress((HMODULE)(m_hMobule), FunctionNameModelOutput);
    if (FunctionModelOutput == nullptr) {
        return false;
    }

    FunctionModelTerminate = (fpModelTerminate)::GetProcAddress((HMODULE)(m_hMobule), FunctionNameModelTerminate);
    if (FunctionModelTerminate == nullptr) {
        return false;
    }

    return true;
}

void CImportCppDll::GenerateFunctionMap(MapFunction& functions)
{
    INSERT_FUNCTION(GetObjectSize);
    INSERT_FUNCTION(GetObjectSubtype);
    INSERT_FUNCTION(StopSimulation);
}

bool CImportCppDll::Initialize()
{
    MapFunction functions;
    GenerateFunctionMap(functions);
    VectorParam vctParam;
    return FunctionInitialize(vctParam, functions);
}

void CImportCppDll::ModelStart()
{
    user_data.time = 0;
    user_data.busId = CSchedulerNode::Instance()->GetBusId();
    user_data.outputPath = RuntimeSettings.GetOutputPath();

    user_data.name = m_strParameterName;
    user_data.name.append(".");
    user_data.name.append(std::to_string(GetIndex()));

    user_data.state = nullptr;
    user_data.parameters.merge(m_mapParameter);

    try
    {
        FunctionModelStart(&user_data);
    }
    catch (...)
    {
        printf("%s-->ModelStart-->Exception\n", user_data.name.c_str());
    }
}

void CImportCppDll::ModelOutput(std::uint32_t nCurrentTime)
{
    user_data.time = nCurrentTime;

    try
    {
#ifdef _WIN32
        ModelOutputWithException(FunctionModelOutput, &user_data);
#else
        FunctionModelOutput(&user_data);
#endif // _WIN32
    }
    catch (...)
    {
        printf("%s-->ModelOutput-->Exception: Time=%d\n", user_data.name.c_str(), nCurrentTime);
    }
}

void CImportCppDll::ModelTerminate(std::uint32_t nCurrentTime)
{
    user_data.time = nCurrentTime;

    try
    {
        FunctionModelTerminate(&user_data);
    }
    catch (...)
    {
        printf("%s-->ModelTerminate-->Exception\n", user_data.name.c_str());
    }
}

std::string CImportCppDll::AppendDllSuffix(const std::string& strDll) {
    std::string strResult(strDll);
#if (_MSC_VER >= 1200 && _MSC_VER < 1300)
    strResult.append("_60");//Microsoft Visual C++ 6.0	Visual Studio 6.0

#elif (_MSC_VER >= 1300 && _MSC_VER < 1310)
    strResult.append("_70");//Microsoft Visual C++ 2002	Visual Studio .NET 2002 (7.0)

#elif (_MSC_VER >= 1310 && _MSC_VER < 1400)
    strResult.append("_71");//Microsoft Visual C++ 2002	Visual Studio .NET 2002 (7.0)

#elif (_MSC_VER >= 1400 && _MSC_VER < 1500)
    strResult.append("_80");//Microsoft Visual C++ 2005	Visual Studio 2005 (8.0)

#elif (_MSC_VER >= 1500 && _MSC_VER < 1600)
    strResult.append("_90");//Microsoft Visual C++ 2008	Visual Studio 2008 (9.0)

#elif (_MSC_VER >= 1600 && _MSC_VER < 1700)
    strResult.append("_100");//Microsoft Visual C++ 2010	Visual Studio 2010 (10.0)

#elif (_MSC_VER >= 1700 && _MSC_VER < 1800)
    strResult.append("_110");//Microsoft Visual C++ 2012	Visual Studio 2012 (11.0)

#elif (_MSC_VER >= 1800 && _MSC_VER < 1900)
    strResult.append("_120");//Microsoft Visual C++ 2013	Visual Studio 2013 (12.0)

#elif (_MSC_VER >= 1900 && _MSC_VER < 1910)
    strResult.append("_140");//Microsoft Visual C++ 2015	Visual Studio 2015 (14.0)

#elif (_MSC_VER >= 1910 && _MSC_VER < 1920)
    strResult.append("_141");//Microsoft Visual C++ 2017	Visual Studio 2017 (15)

#elif (_MSC_VER >= 1920)
    strResult.append("_142");
#endif

#ifdef _DEBUG
    strResult.append("d");
#endif // _DEBUG
    strResult.append(".dll");
    return strResult;
}
