#pragma once

#include <boost/atomic.hpp>
#include <pybind11/embed.h>
#include <FileMapping.h>


class CSchedulerNode final
{
public:

    static CSchedulerNode* Instance() noexcept;
    bool Initialize();
    void Run();
    bool ReadEgoData(EgoBus& egoData) const;
    std::uint32_t GetBusId() const noexcept;
    void RequestStopSimulation(std::int32_t nDelayStepCounts = 1);
    void PauseSimulation();
    void ContinueSimulation();
    bool PythonDebugEnable() {
        return !m_bDisablePythonDebug;
    }
    void DisablePythonDebug(bool bDisable = true) {
        m_bDisablePythonDebug = bDisable;
    }
    virtual ~CSchedulerNode();

protected:
    explicit CSchedulerNode();
    CSchedulerNode(const CSchedulerNode&) = delete;
    CSchedulerNode& operator= (const CSchedulerNode&) = delete;
    bool OnRecvStartReq();
    void OnRecvStopReq();
    void OnRecvExitReq();
    void CreateData();
    void ReleaseData();
    bool DoStep(std::uint32_t& nLastTime);
    bool InitializePython();
    void ReadRequest(std::uint32_t &nLoop,  std::uint32_t& nLastTime, bool& bExit);
    void WaitForBusWriteComplete();

private:
    CFileMapping::Pointer m_pMemoryProcessId;
    std::uint32_t m_nBusId;
    std::uint32_t m_nNodeId;
    std::uint32_t m_nHostId;
    CFileMapping::Pointer m_pBusManager;
    CFileMapping::Pointer m_pBusEgo;
    CFileMapping::Pointer m_pBusClockBlock;
    HANDLE m_hEventClock;
    boost::atomic<bool> m_bRunning;
    std::uint32_t m_nCurrentTime;
    std::pair<std::int32_t, std::int32_t> StopReqTimeAndDelayStepCounts;
    std::string m_strPythonPath;
    bool m_bDisablePythonDebug;
    std::unique_ptr<pybind11::scoped_interpreter> m_pPythonInterpreter;
};
