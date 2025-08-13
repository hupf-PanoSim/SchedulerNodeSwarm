#include <AbstractImport.h>

CAbstractImport::CAbstractImport(const std::string& strStem) : m_strStem(strStem), m_nIndex(0), m_bDelFromModules(true)
{}

inline void CAbstractImport::SetIndex(std::int32_t nIndex) noexcept
{
    m_nIndex = nIndex;
}

inline std::int32_t CAbstractImport::GetIndex() const noexcept
{
    return m_nIndex;
}

inline const std::string& CAbstractImport::GetStem() const noexcept
{
    return m_strStem;
}
