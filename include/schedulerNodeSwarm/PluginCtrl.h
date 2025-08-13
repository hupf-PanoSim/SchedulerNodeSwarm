#pragma once

#include <unordered_set>
#include <AbstractImport.h>


enum class PluginType : char {
    Cpp,
    Python,
    Simulink,
    Unknown
};

enum class Plugin : char {
    Agent = 0,
    Evaluator,
    Sensor
};

constexpr char const* DefaultNodeId = "0";
constexpr char const* DefaultHostId = "0";

class CPlugin final
{
    DEFINE_STD_UNIQUE_PTR(CPlugin)
public:
    static CPlugin::Pointer Create(const std::string& strFile);
    explicit CPlugin(const std::string& strFile) noexcept;
    virtual ~CPlugin() noexcept;

    bool Load(Plugin emPlugin, MapParam&& mapParameter);

    PluginType Type() const noexcept;
    const std::string& GetStem() const noexcept;
    void SetIndex(std::int32_t nIndex) noexcept;
    void DelFromPyModules(bool bDelFromModules) noexcept;

    void OnModelStart();
    void OnModelOutput(std::uint32_t nCurrentTime);
    void OnModelTerminate(std::uint32_t nCurrentTime);

protected:
    void Clear() noexcept;
    bool LoadPython(const std::string& strStem, const std::string& strParameterName, MapParam&& mapParameter);
    bool LoadCppDll(const std::string& strStem, const std::string& strParameterName, MapParam&& mapParameter);

private:
    PluginType m_emType;
    const std::string m_strFile;
    CAbstractImport::Pointer m_pImport;
};

class CPluginCtrl final
{
public:
    CPluginCtrl(CPluginCtrl const&) = delete;
    CPluginCtrl& operator= (CPluginCtrl const&) = delete;
    CPluginCtrl(CPluginCtrl&&) noexcept = delete;
    CPluginCtrl& operator=(CPluginCtrl&&) noexcept = delete;
    static bool Valid() noexcept;
    static void CreateInstance(std::int32_t nNodeId, std::int32_t nHostId);
    static CPluginCtrl& GetInstance() noexcept;
    static void ReleaseInstance() noexcept;
    virtual ~CPluginCtrl() noexcept;

    const std::string& GetNodeId() const noexcept;
    const std::string& GetHostId() const noexcept;
    bool IsSkipAttr(const std::string& strKey) const noexcept;
    bool LoadPlugin(Plugin emPlugin, const std::string& strLink, std::int32_t nIndex, MapParam&& mapParameter);

    void OnModelStart();
    void OnModelOutput(std::uint32_t nCurrentTime);
    void OnModelTerminate(std::uint32_t nCurrentTime);
    
protected:
    explicit CPluginCtrl(std::int32_t nNodeId, std::int32_t nHostId) noexcept;
    void Clear() noexcept;

private:
    static volatile bool static_bValid;
    const std::string m_strNodeId;
    const std::string m_strHostId;
    std::unordered_set<std::string_view> m_setSkipAttr;
    std::unordered_map<Plugin, std::vector<CPlugin::Pointer>> m_mapPlugin;

    /*< singleton object */
    static std::unique_ptr<CPluginCtrl> static_pInstance;
};

#define Plugins    CPluginCtrl::GetInstance()
