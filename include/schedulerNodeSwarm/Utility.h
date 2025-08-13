#pragma once

#include <string>
#include <vector>

namespace Utility {
    bool GetPythonLocation(const std::string& strCurrentPath, std::string& strPythonLocation);
    bool GetCurrentFullPath(std::string& strCurrentPath);

    std::wstring str2wstr(const std::string& strSrc) noexcept;
    std::string wstr2str(const std::wstring& wstrSrc) noexcept;

    bool GetDatabaseHome(std::string& strPanoSwarmDatabaseHome) noexcept;
} // namespace Utility
