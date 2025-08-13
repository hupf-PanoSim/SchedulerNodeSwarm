#include <SchedulerNode.h>

#include <iostream>

#include <boost/process.hpp>
#include <boost/thread.hpp>
#include <boost/interprocess/detail/os_thread_functions.hpp>

#include <Utility.h>
#include <RuntimeSettings.h>
#include <PluginCtrl.h>
#include <CsvFiles.h>

#ifdef _DEBUG
#define INDEPENDENT_RUN 1
#else
#define INDEPENDENT_RUN 0
#endif // _DEBUG


static constexpr std::int32_t MANAGER_BUS_SIZE = 1024;
static constexpr std::int32_t MANAGER_STATUS_OFFSET = 0;
static constexpr std::int32_t MANAGER_EXP_OFFSET = 1;

static constexpr std::int32_t NODE_BUS_SIZE = 1024;
static constexpr std::int32_t NODE_STATUS_OFFSET = 0;
static constexpr std::int32_t NODE_BUS_ID_OFFSET = 1;
static constexpr std::int32_t NODE_NODE_ID_OFFSET = 5;

static constexpr std::uint8_t STATUS_UNKNOWN = 0;
static constexpr std::uint8_t STATUS_READY = 1;
static constexpr std::uint8_t STATUS_RUNNING = 2;
static constexpr std::uint8_t STATUS_PAUSED = 3;

static constexpr std::uint8_t STATUS_START_REQ = 101;
static constexpr std::uint8_t STATUS_STOP_REQ = 102;
static constexpr std::uint8_t STATUS_EXIT_REQ = 103;

static constexpr std::uint8_t STATUS_CLOCK_START_REQ = 201;
static constexpr std::uint8_t STATUS_CLOCK_PAUSE_REQ = 202;
static constexpr std::uint8_t STATUS_CLOCK_CONTINUE_REQ = 203;
static constexpr std::uint8_t STATUS_CLOCK_STOP_REQ = 204;

constexpr std::uint32_t READ_CONTROL_CMD_INTERVAL = 100;
constexpr std::uint32_t DEFAULT_INTERVAL = 10;
constexpr std::uint32_t MAX_STEP_INTERVAL = 500;
constexpr std::uint32_t MAX_SKIP_COUNT = (MAX_STEP_INTERVAL / DEFAULT_INTERVAL);

static std::string LinkString(const std::string& str1, const std::string& str2, const std::string& str3);

CSchedulerNode* CSchedulerNode::Instance() noexcept
{
    static CSchedulerNode obj;
    return &obj;
}

CSchedulerNode::CSchedulerNode() : StopReqTimeAndDelayStepCounts{ -1, 0 }
{
    m_pBusManager = nullptr;
    m_pMemoryProcessId = nullptr;
    m_pBusEgo = nullptr;
    m_hEventClock = nullptr;
    m_bRunning = false;
    m_strPythonPath.clear();
    m_nBusId = 0;
    m_nNodeId = 0;
    m_nHostId = 0;
    m_nCurrentTime = 0;
    m_bDisablePythonDebug = false;
}

CSchedulerNode::~CSchedulerNode()
{
    ReleaseData();
    if (m_pMemoryProcessId)
    {
        m_pMemoryProcessId.reset();
    }
}

bool CSchedulerNode::Initialize()
{
    using namespace boost::interprocess;
    ipcdetail::OS_process_id_t pId = ipcdetail::get_current_process_id();
    std::string strBusName = "panoswarm.node.";
    strBusName.append(std::to_string(pId));
    m_pMemoryProcessId = CFileMapping::Create(strBusName, NODE_BUS_SIZE);
    if (m_pMemoryProcessId)
    {
        if (m_pMemoryProcessId->Open())
        {
            return m_pMemoryProcessId->WriteByte(STATUS_READY, NODE_STATUS_OFFSET);
        }
    }

    return false;
}

bool CSchedulerNode::DoStep(std::uint32_t& nLastTime)
{
    m_nCurrentTime = *(reinterpret_cast<std::uint32_t*>(m_pBusEgo->Memory()));

#if !INDEPENDENT_RUN
    if (m_nCurrentTime == nLastTime)
    {
        return false;
    }
#endif

    if (StopReqTimeAndDelayStepCounts.first > 0)
    {
        if (m_nCurrentTime > StopReqTimeAndDelayStepCounts.first + StopReqTimeAndDelayStepCounts.second * DEFAULT_INTERVAL)
        {
            m_pBusManager->WriteByte(STATUS_STOP_REQ, NODE_STATUS_OFFSET);
            StopReqTimeAndDelayStepCounts.first = -1;
        }
    }

    //bool bSyncBlock = RuntimeSettings.SyncBlock() && m_nNodeId == 0;
    //if (bSyncBlock)
    //{
    //    WaitForBusWriteComplete();
    //}

    Plugins.OnModelOutput(m_nCurrentTime);

    //if (bSyncBlock)
    //{
    //    m_pBusClockBlock->WriteInteger(m_nCurrentTime);
    //}

    nLastTime = m_nCurrentTime;

    return true;
}

bool CSchedulerNode::InitializePython()
{
    m_strPythonPath.clear();
    std::string strCurrentPath;
    if (!Utility::GetCurrentFullPath(strCurrentPath))
    {
        return false;
    }
    if (!Utility::GetPythonLocation(strCurrentPath, m_strPythonPath))
    {
        return false;
    }

    std::string strDatabaseHome;
    if (!Utility::GetDatabaseHome(strDatabaseHome))
    {
        return false;
    }

    boost::algorithm::replace_all(m_strPythonPath, "\\", "/");

    std::string strLib = m_strPythonPath;
    strLib.append("/Lib");

    std::vector<std::string> vctPythonPath;
    auto getPluginPath = [&](const std::string& strSubFolder) {
        boost::filesystem::path pathPlugin(strDatabaseHome);
        std::string strPythonFilePath = pathPlugin.string();
        pathPlugin /= "Plugin";
        pathPlugin /= strSubFolder;
        if (boost::filesystem::exists(pathPlugin)) {
            std::string strTemp = pathPlugin.string();
            boost::algorithm::replace_all(strTemp, "\\", "/");
            vctPythonPath.emplace_back(std::move(strTemp));
        }
    };

    auto getPythonDependPath = [&](std::string_view strPath) {
        std::string strDependPath = m_strPythonPath;
        strDependPath.append(strPath);
        vctPythonPath.push_back(strDependPath);
    };

    getPythonDependPath("/DLLs");
    getPythonDependPath("/Lib/site-packages");
    getPythonDependPath("/python36.zip");

    getPluginPath("Agent");
    getPluginPath("Agent/Library");
    //getPluginPath("Evaluator");
    //getPluginPath("Evaluator/Library");
    getPluginPath("Sensor");
    getPluginPath("Sensor/Library");

    try {
        ::Py_SetPythonHome((wchar_t*)(Utility::str2wstr(m_strPythonPath).c_str()));
        ::Py_SetPath((wchar_t*)(Utility::str2wstr(strLib).c_str()));
        m_pPythonInterpreter = std::make_unique<pybind11::scoped_interpreter>();

        PyRun_SimpleString("import sys");

        std::string strCmd = "sys.exec_prefix = '";
        strCmd.append(m_strPythonPath);
        strCmd.append("'");
        PyRun_SimpleString(strCmd.c_str());

        strCmd = "sys.base_exec_prefix = '";
        strCmd.append(m_strPythonPath);
        strCmd.append("'");
        PyRun_SimpleString(strCmd.c_str());

        strCmd = "sys.argv  = ['']";
        PyRun_SimpleString(strCmd.c_str());

        for (const auto& strPythonPath : vctPythonPath) {
            strCmd = "sys.path.append('";
            strCmd.append(strPythonPath);
            strCmd.append("')");
#if INDEPENDENT_RUN
            std::cout << "SchedulerNode:" << strCmd << std::endl;
#endif
            PyRun_SimpleString(strCmd.c_str());
        }
    }
    catch (std::runtime_error& _error) {
        printf("%s\n", _error.what());
        return false;
    }

    return true;
}

void CSchedulerNode::ReadRequest(std::uint32_t& nLoop, std::uint32_t& nLastTime, bool& bExit)
{
    std::uint8_t request = 0;
    m_pMemoryProcessId->ReadByte(request, NODE_STATUS_OFFSET);
    switch (request)
    {
    case STATUS_START_REQ:
    {
        OnRecvStartReq();
        break;
    }
    case STATUS_STOP_REQ:
    {
        OnRecvStopReq();
        nLoop = 0;
        nLastTime = 0;
        break;
    }
    case STATUS_EXIT_REQ:
    {
        OnRecvExitReq();
        bExit = true;
        break;
    }
    default:
        break;
    }
}

void CSchedulerNode::Run()
{
    static constexpr std::uint32_t LOOP_MAX_COUNT = READ_CONTROL_CMD_INTERVAL / DEFAULT_INTERVAL;

    InitializePython();

#if INDEPENDENT_RUN
    {
        std::uint8_t arrData[2];
        arrData[0] = STATUS_START_REQ;
        m_pMemoryProcessId->WriteData(arrData, 1, NODE_STATUS_OFFSET);
    }
#endif // INDEPENDENT_RUN

    bool bFlagExit = false;
    std::uint32_t nLoop = 0;
    std::uint32_t nLastTime = 0;
    m_nCurrentTime = 0;

    while (true)
    {
        if (m_bRunning)
        {
            DoStep(nLastTime);

            ::WaitForSingleObject(m_hEventClock, DEFAULT_INTERVAL + 1);

            if (++nLoop > LOOP_MAX_COUNT)
            {
                ReadRequest(nLoop, nLastTime, bFlagExit);
                nLoop = 0;
            }
        }
        else
        {
            boost::this_thread::sleep_for(boost::chrono::milliseconds(READ_CONTROL_CMD_INTERVAL));
            ReadRequest(nLoop, nLastTime, bFlagExit);
        }
        if (bFlagExit)
        {
            break;
        }
    }
}

bool CSchedulerNode::OnRecvStartReq()
{
    std::string strBusId;
    std::string strExperimentName;
    m_nBusId = 0;
    m_nNodeId = 0;
    m_nHostId = 0;
    m_nCurrentTime = 0;
    StopReqTimeAndDelayStepCounts.first = -1;
    StopReqTimeAndDelayStepCounts.second = 0;

    if (!m_pMemoryProcessId->ReadInteger(m_nBusId, NODE_BUS_ID_OFFSET)) {
        return false;
    }
    strBusId = std::to_string(m_nBusId);

    if (!m_pMemoryProcessId->ReadInteger(m_nNodeId, NODE_NODE_ID_OFFSET)) {
        return false;
    }

    if (!m_pMemoryProcessId->ReadInteger(m_nHostId, NODE_NODE_ID_OFFSET + 4)) {
        return false;
    }

    m_pBusManager = CFileMapping::Create(LinkString("panoswarm.", strBusId, ".manager"), DEFAULT_SHARED_MEMORY_MAX_SIZE);
    if (!m_pBusManager->Open()) {
        return false;
    }

    m_pBusEgo = CFileMapping::Create(LinkString("panoswarm.", strBusId, ".ego"), sizeof(EgoBus));
    if (!m_pBusEgo->Open()) {
        return false;
    }

    //m_pBusClockBlock = CFileMapping::Create(LinkString("panoswarm.", strBusId, ".clock_block"), sizeof(ClockBlockBus));
    //if (!m_pBusClockBlock->Open()) {
    //    return false;
    //}

#if INDEPENDENT_RUN
    strExperimentName = "Desert_Train/Temp";
#else
    if (!m_pBusManager->ReadData(strExperimentName, MANAGER_EXP_OFFSET)) {
        return false;
    }
#endif // INDEPENDENT_RUN

    CreateData();
    if (!Plugins.Valid()) {
        return false;
    }

    if (!RuntimeSettings.Generate(strExperimentName)) {
        return false;
    }

    m_hEventClock = ::CreateEvent(nullptr, TRUE, FALSE, LinkString("panoswarm.", strBusId, ".step").c_str());
    if (m_hEventClock == nullptr || m_hEventClock == INVALID_HANDLE_VALUE) {
        return false;
    }

    Plugins.OnModelStart();

    m_bDisablePythonDebug = false;

    std::cout << "SchedulerNode: running" << std::endl;
    m_bRunning = true;
    m_pMemoryProcessId->WriteByte(STATUS_RUNNING, NODE_STATUS_OFFSET);
    return true;
}

void CSchedulerNode::OnRecvStopReq()
{
    std::uint32_t nCurrentTime = 0;
    EgoBus egoBus;
    if (ReadEgoData(egoBus))
    {
        nCurrentTime = egoBus.time + DEFAULT_INTERVAL;
    }

    if (Plugins.Valid())
    {
        Plugins.OnModelTerminate(nCurrentTime);
    }

    ReleaseData();

    std::cout << "SchedulerNode: stopped" << std::endl;

    m_bRunning = false;

    if (m_pMemoryProcessId)
    {
        m_pMemoryProcessId->WriteByte(STATUS_READY, NODE_STATUS_OFFSET);
    }
}

void CSchedulerNode::OnRecvExitReq()
{
    std::uint32_t nCurrentTime = 0;
    EgoBus egoBus;
    if (ReadEgoData(egoBus))
    {
        nCurrentTime = egoBus.time + DEFAULT_INTERVAL;
    }

    if (Plugins.Valid())
    {
        Plugins.OnModelTerminate(nCurrentTime);
    }

    ReleaseData();

    m_bRunning = false;

    if (m_pMemoryProcessId)
    {
        m_pMemoryProcessId->WriteByte(STATUS_UNKNOWN, NODE_STATUS_OFFSET);
    }
}

void CSchedulerNode::CreateData()
{
    CPluginCtrl::CreateInstance(m_nNodeId, m_nHostId);
    CRuntimeSettings::GetInstance();
    CCsvFileCtrl::GetInstance();
}

void CSchedulerNode::ReleaseData()
{
    m_nCurrentTime = 0;
    StopReqTimeAndDelayStepCounts.first = -1;
    StopReqTimeAndDelayStepCounts.second = 0;
    if (m_hEventClock != nullptr)
    {
        ::CloseHandle(m_hEventClock);
        m_hEventClock = nullptr;
    }

    if (m_pBusManager)
    {
        m_pBusManager.reset();
    }
    if (m_pBusEgo)
    {
        m_pBusEgo.reset();
    }
    //if (m_pBusClockBlock)
    //{
    //    m_pBusClockBlock.reset();
    //}

    CPluginCtrl::ReleaseInstance();
    CRuntimeSettings::ReleaseInstance();
    CCsvFileCtrl::ReleaseInstance();
}

bool CSchedulerNode::ReadEgoData(EgoBus& egoData) const
{
    auto pData = reinterpret_cast<std::uint8_t*>(&egoData);
    if (pData)
    {
        if (m_pBusEgo)
        {
            if (m_pBusEgo->ReadData(pData, sizeof(egoData), 0))
            {
                return true;
            }
        }
    }
    return false;
}

std::uint32_t CSchedulerNode::GetBusId() const noexcept
{
    return m_nBusId;
}

void CSchedulerNode::RequestStopSimulation(std::int32_t nDelayStepCounts)
{
    if (StopReqTimeAndDelayStepCounts.first < 0) {
        StopReqTimeAndDelayStepCounts.first = m_nCurrentTime;
        StopReqTimeAndDelayStepCounts.second = nDelayStepCounts;
    }
}

void CSchedulerNode::PauseSimulation()
{
    m_pBusManager->WriteByte(STATUS_CLOCK_PAUSE_REQ, NODE_STATUS_OFFSET);
}

void CSchedulerNode::ContinueSimulation()
{
    m_pBusManager->WriteByte(STATUS_CLOCK_CONTINUE_REQ, NODE_STATUS_OFFSET);
}

void CSchedulerNode::WaitForBusWriteComplete()
{
    //while (true)
    //{
    //    volatile std::uint32_t nEgoTrafficBusTime = *(reinterpret_cast<std::uint32_t*>(m_pBusEgoTraffic->Memory()));
    //    if (nEgoTrafficBusTime == m_nCurrentTime)
    //    {
    //        volatile std::uint32_t nRoadObstacleBusTime = *(reinterpret_cast<std::uint32_t*>(m_pBusRoadObstacle->Memory()));
    //        if (nRoadObstacleBusTime == m_nCurrentTime || m_nCurrentTime == 10)
    //        {
    //            break;
    //        }
    //    }
    //}
}

std::string LinkString(const std::string& str1, const std::string& str2, const std::string& str3)
{
    std::string str = str1;
    str.append(str2);
    str.append(str3);
    return str;
}
