#pragma once

#if USE_BOOST_SHARED_PTR
#include <boost/shared_ptr.hpp>
#else
#include <memory>
#endif // USE_BOOST_SHARED_PTR

#include <string>


#if USE_BOOST_SHARED_PTR
/************************************************************************/
/* boost::shared_ptr                                                    */
/************************************************************************/
/*!< 申明BOOST智能指针 */
#define DECLARE_BOOST_SHARED_PTR(ClassName)                 \
public:                                                     \
    using SharedPointer = boost::shared_ptr<ClassName>;     \
protected:                                                  \
    ClassName(const ClassName &) = delete;                  \
    ClassName &operator= (const ClassName &) = delete;

/*!< 定义BOOST智能指针 */
#define DEFINE_BOOST_SHARED_PTR(ClassName)                  \
public:                                                     \
    using SharedPointer = boost::shared_ptr<ClassName>;     \
    static ClassName::SharedPointer Create() {              \
        return ClassName::SharedPointer(new ClassName);     \
    }                                                       \
protected:                                                  \
    ClassName(const ClassName &) = delete;                  \
    ClassName & operator= (const ClassName &) = delete;

#else

#define DISABLE_DEFAULT_COPY_METHOD(ClassName)              \
protected:                                                  \
    ClassName(const ClassName &) = delete;                  \
    ClassName &operator= (const ClassName &) = delete;


#define DEFINE_STD_SHARED_PTR(ClassName)                    \
public:                                                     \
    using SharedPointer = std::shared_ptr<ClassName>;       \
protected:                                                  \
    DISABLE_DEFAULT_COPY_METHOD(ClassName)

#define DECLARE_DEFAULT_SHARED_CONSTRUCTION(ClassName)      \
public:                                                     \
    static ClassName::SharedPointer CreateSharedPointer();  \
    explicit ClassName();                                   \
    virtual ~ClassName();

#define CREATE_STD_SHARED_IMPLEMENT(ClassName)              \
ClassName::SharedPointer ClassName::CreateSharedPointer()   \
{ return std::make_shared<ClassName>(); }


#define DEFINE_STD_UNIQUE_PTR(ClassName)                    \
public:                                                     \
    using Pointer = std::unique_ptr<ClassName>;             \
protected:                                                  \
    DISABLE_DEFAULT_COPY_METHOD(ClassName)

#define DECLARE_DEFAULT_UNIQUE_CONSTRUCTION(ClassName)      \
public:                                                     \
    static ClassName::Pointer Create();                     \
    explicit ClassName();                                   \
    virtual ~ClassName();

#define CREATE_STD_UNIQUE_IMPLEMENT(ClassName)              \
ClassName::Pointer ClassName::Create()                      \
{ return std::make_unique<ClassName>(); }


#endif // USE_BOOST_SHARED_PTR
