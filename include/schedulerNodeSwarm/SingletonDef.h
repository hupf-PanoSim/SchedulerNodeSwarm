#pragma once

#include <memory>

#define DECLARE_SINGLETON(ClassName)                                \
public:                                                             \
    ClassName(ClassName const&) = delete;                           \
    ClassName& operator= (ClassName const&) = delete;               \
    ClassName(ClassName&&) noexcept = delete;                       \
    ClassName& operator= (ClassName&&) noexcept = delete;           \
    static inline ClassName& GetInstance() {                        \
        if (!static_pInstance) {                                    \
            static_pInstance.reset(new ClassName);                  \
        }                                                           \
        return *static_pInstance;                                   \
    }                                                               \
    static void ReleaseInstance() {                                 \
        if (static_pInstance) { static_pInstance.reset(); }         \
    }                                                               \
private:                                                            \
    static std::unique_ptr<ClassName> static_pInstance;


#define IMPLEMENT_SINGLETON(ClassName)                              \
std::unique_ptr<ClassName> ClassName::static_pInstance = nullptr;
