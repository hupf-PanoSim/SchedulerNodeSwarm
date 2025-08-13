#include <PluginCtrl.h>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <ImportCppDll.h>
#include <ImportPython.h>
#include <ImportSimulinkDll.h>
#include <Utility.h>
#include <RuntimeSettings.h>


static const std::array<std::string_view, 3> arrSkipAttr = {
    "hostId",
    "nodeId",
    "internal"
};

volatile bool CPluginCtrl::static_bValid = false;
std::unique_ptr<CPluginCtrl> CPluginCtrl::static_pInstance = nullptr;


bool CPluginCtrl::Valid() noexcept
{
    return static_bValid;
}

void CPluginCtrl::CreateInstance(std::int32_t nNodeId, std::int32_t nHostId)
{
    if (!static_pInstance)
    {
        static_pInstance.reset(new CPluginCtrl(nNodeId, nHostId));
    }
    if (static_pInstance)
    {
        static_bValid = true;
    }
}

CPluginCtrl& CPluginCtrl::GetInstance() noexcept
{
    return *static_pInstance;
}

void CPluginCtrl::ReleaseInstance() noexcept
{
    static_bValid = false;
    if (static_pInstance)
    {
        static_pInstance.reset();
    }
}

CPluginCtrl::CPluginCtrl(std::int32_t nNodeId, std::int32_t nHostId) noexcept
    : m_strNodeId(std::to_string(nNodeId)), m_strHostId(std::to_string(nHostId))
{
    Clear();
    for (const auto& strSkip : arrSkipAttr)
    {
        m_setSkipAttr.insert(strSkip);
    }
}

CPluginCtrl::~CPluginCtrl() noexcept
{
    Clear();
}

inline void CPluginCtrl::Clear() noexcept
{
    m_setSkipAttr.clear();
    m_mapPlugin.clear();
}

bool CPluginCtrl::IsSkipAttr(const std::string& strKey) const noexcept
{
    return m_setSkipAttr.find(strKey) != m_setSkipAttr.end();
}

const std::string& CPluginCtrl::GetNodeId() const noexcept
{
    return m_strNodeId;
}

const std::string& CPluginCtrl::GetHostId() const noexcept
{
    return m_strHostId;
}

bool CPluginCtrl::LoadPlugin(Plugin emPlugin, const std::string& strLink, std::int32_t nIndex, MapParam&& mapParameter)
{
    CPlugin::Pointer pPlugin = CPlugin::Create(strLink);
    if (!pPlugin)
    {
        return false;
    }

    if (!pPlugin->Load(emPlugin, std::forward<MapParam>(mapParameter)))
    {
        return false;
    }
    pPlugin->SetIndex(nIndex);

    auto findPlugin = m_mapPlugin.find(emPlugin);
    if (findPlugin == m_mapPlugin.end())
    {
        std::vector<CPlugin::Pointer> vct;
        vct.emplace_back(std::move(pPlugin));
        m_mapPlugin.insert(std::make_pair(emPlugin, std::move(vct)));
    }
    else
    {
        findPlugin->second.emplace_back(std::move(pPlugin));
    }

    return true;
}

void CPluginCtrl::OnModelStart()
{
    for (const auto& pairItem : m_mapPlugin)
    {
        for (const auto& pPlugin : pairItem.second)
        {
            pPlugin->OnModelStart();
        }
    }
}

void CPluginCtrl::OnModelOutput(std::uint32_t nCurrentTime)
{
    for (const auto& pairItem : m_mapPlugin)
    {
        for (const auto& pPlugin : pairItem.second)
        {
            pPlugin->OnModelOutput(nCurrentTime);
        }
    }
}

void CPluginCtrl::OnModelTerminate(std::uint32_t nCurrentTime)
{
    std::unordered_set<std::string> setPythonPlugin;
    for (const auto& pairItem : m_mapPlugin)
    {
        for (const auto& pPlugin : pairItem.second)
        {
            if (pPlugin->Type() == PluginType::Python)
            {
                auto isTerminate = setPythonPlugin.find(pPlugin->GetStem());
                if (isTerminate == setPythonPlugin.end())
                {
                    setPythonPlugin.emplace(pPlugin->GetStem());
                    pPlugin->DelFromPyModules(true);
                }
                else
                {
                    pPlugin->DelFromPyModules(false);
                }
            }
            pPlugin->OnModelTerminate(nCurrentTime);
        }
    }
}


////----CPlugin----//////////////////////////////////////////////////////////////////////
CPlugin::CPlugin(const std::string& strFile) noexcept : m_strFile(strFile)
{
    Clear();
}

CPlugin::~CPlugin() noexcept
{
    Clear();
}

CPlugin::Pointer CPlugin::Create(const std::string& strFile)
{
    return std::make_unique<CPlugin>(strFile);
}

inline void CPlugin::Clear() noexcept
{
    if (m_pImport)
    {
        m_pImport.reset();
    }
    m_emType = PluginType::Unknown;
}

bool CPlugin::LoadPython(const std::string& strStem, const std::string& strParameterName, MapParam&& mapParameter)
{
    CImportPython::Pointer pImport = CImportPython::Create(strStem, strParameterName, std::forward<MapParam>(mapParameter));
    if (pImport)
    {
        if (pImport->Load())
        {
            m_emType = PluginType::Python;
            m_pImport = std::move(pImport);
            return true;
        }
    }

    return false;
}

bool CPlugin::LoadCppDll(const std::string& strStem, const std::string& strParameterName, MapParam&& mapParameter)
{
#ifdef _DEBUG
    if (boost::algorithm::equals(strStem, "xDriver"))
    {
        return false;
    }
#endif // _DEBUG

    CImportCppDll::Pointer pImportCppDll = CImportCppDll::Create(strStem, strParameterName, std::forward<MapParam>(mapParameter));
    if (pImportCppDll)
    {
        if (pImportCppDll->Load(m_strFile))
        {
            if (pImportCppDll->Initialize())
            {
                m_emType = PluginType::Cpp;
                m_pImport = std::move(pImportCppDll);
                return true;
            }
        }
    }

    CImportSimulinkDll::Pointer pImportSimulinkDll = CImportSimulinkDll::Create(strStem);
    if (pImportSimulinkDll)
    {
        if (pImportSimulinkDll->Load(m_strFile))
        {
            m_emType = PluginType::Simulink;
            m_pImport = std::move(pImportSimulinkDll);
            return true;
        }
    }

    return false;
}

bool CPlugin::Load(Plugin emPlugin, MapParam&& mapParameter)
{
    boost::filesystem::path pathPlugin(m_strFile);

    if (!pathPlugin.has_extension())
    {
        return false;
    }
    const std::string& strExtension = pathPlugin.extension().string();

    if (!pathPlugin.has_stem())
    {
        return false;
    }
    const std::string& strStem = pathPlugin.stem().string();

    const char* const EvaluatorParameterName = "judge";
    const std::string& strParameterName = emPlugin == Plugin::Evaluator ? EvaluatorParameterName : strStem;

    const char* const DLL_SUFFIX = ".dll";
    const char* const PYTHON_SUFFIX = ".py";
    const char* const PYTHON_DLL_SUFFIX = ".pyd";
    if (boost::algorithm::iequals(strExtension, DLL_SUFFIX))
    {
        if (boost::filesystem::exists(pathPlugin))
        {
            return LoadCppDll(strStem, strParameterName, std::forward<MapParam>(mapParameter));
        }
    }
    else if (boost::algorithm::iequals(strExtension, PYTHON_SUFFIX))
    {
        if (boost::filesystem::exists(pathPlugin))
        {
            return LoadPython(strStem, strParameterName, std::forward<MapParam>(mapParameter));
        }

        if (pathPlugin.has_parent_path())
        {
            auto pathPyd = pathPlugin.parent_path();
            std::string pydFile = strStem;
            pydFile.append(PYTHON_DLL_SUFFIX);
            pathPyd.append(pydFile);
            if (boost::filesystem::exists(pathPyd))
            {
                return LoadPython(strStem, strParameterName, std::forward<MapParam>(mapParameter));
            }
        }
    }

    return false;
}

void CPlugin::OnModelStart()
{
    if (m_pImport)
    {
        m_pImport->ModelStart();
    }
}

void CPlugin::OnModelOutput(std::uint32_t nCurrentTime)
{
    if (m_pImport)
    {
        m_pImport->ModelOutput(nCurrentTime);
    }
}

void CPlugin::OnModelTerminate(std::uint32_t nCurrentTime)
{
    if (m_pImport)
    {
        m_pImport->ModelTerminate(nCurrentTime);
    }
}

inline void CPlugin::SetIndex(std::int32_t nIndex) noexcept
{
    m_pImport->SetIndex(nIndex);
}

inline const std::string& CPlugin::GetStem() const noexcept
{
    return m_pImport->GetStem();
}

inline PluginType CPlugin::Type() const noexcept
{
    return m_emType;
}

inline void CPlugin::DelFromPyModules(bool bDelFromModules) noexcept
{
    m_pImport->DelFromPyModules(bDelFromModules);
}
