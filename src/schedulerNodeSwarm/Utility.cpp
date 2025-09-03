#include <Utility.h>

#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/process/environment.hpp>
#include <boost/locale.hpp>


bool Utility::GetCurrentFullPath(std::string& strCurrentPath)
{
    strCurrentPath.clear();
    boost::dll::fs::error_code errorCode;
    try {
        boost::dll::fs::path programLocation = boost::dll::program_location(errorCode);
        if (!errorCode) {
            if (programLocation.has_parent_path()) {
                strCurrentPath = programLocation.parent_path().string();
                return !(strCurrentPath.empty());
            }
        }
    }
    catch (const std::runtime_error& e) {
        printf("%s\n", e.what());
        return false;
    }
    return false;
}

bool Utility::GetPythonLocation(const std::string& strCurrentPath, std::string& strPythonLocation)
{
    strPythonLocation.clear();
#ifdef _DEBUG
    strPythonLocation = "D:\\PanoSwarm\\git\\Release\\py36";
#else
    boost::dll::fs::path parentPath(strCurrentPath);
    if (!parentPath.has_parent_path())
    {
        return false;
    }
    parentPath = parentPath.parent_path();

    constexpr char const* PanoSwarm_Python_Folder = "py36";
    parentPath /= PanoSwarm_Python_Folder;
    if (!boost::filesystem::exists(parentPath))
    {
        return false;
    }

    strPythonLocation = parentPath.string();
    if (strPythonLocation.empty())
    {
        return false;
    }
#endif // _DEBUG

    return true;
}

std::wstring Utility::str2wstr(const std::string& strSrc) noexcept
{
    return boost::locale::conv::to_utf<wchar_t>(strSrc, "GBK");
}

std::string Utility::wstr2str(const std::wstring& wstrSrc) noexcept
{
    return boost::locale::conv::from_utf(wstrSrc, "GBK");
}

bool Utility::GetDatabaseHome(std::string& strPanoSimDatabaseHome) noexcept
{
    constexpr char const* ENVIRONMENT_KEY = "PanoSwarmDatabaseHome";

    strPanoSimDatabaseHome.clear();
    const boost::process::environment& environmentVars = boost::this_process::environment();
    const auto resultVar = std::find_if(
        environmentVars.cbegin(), environmentVars.cend(),
        [&](auto item) -> bool {
            return boost::algorithm::iequals(ENVIRONMENT_KEY, item.get_name());
        }
    );
    if (resultVar == environmentVars.cend())
    {
        return false;
    }
    strPanoSimDatabaseHome = resultVar->to_string();
    if (!boost::filesystem::exists(boost::filesystem::path(strPanoSimDatabaseHome)))
    {
        return false;
    }
    return true;
}


bool Utility::GetEnvResource(std::string& strEnvResource) noexcept
{
    constexpr char const* ENVIRONMENT_KEY = "PanoSwarmResource";

    strEnvResource.clear();
    const boost::process::environment& environmentVars = boost::this_process::environment();
    const auto resultVar = std::find_if(
        environmentVars.cbegin(), environmentVars.cend(),
        [&](auto item) -> bool {
            return boost::algorithm::iequals(ENVIRONMENT_KEY, item.get_name());
        }
    );
    if (resultVar == environmentVars.cend())
    {
        return false;
    }
    strEnvResource = resultVar->to_string();
    if (!boost::filesystem::exists(boost::filesystem::path(strEnvResource)))
    {
        return false;
    }
    return true;
}
