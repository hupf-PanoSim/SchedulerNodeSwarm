#pragma once

#include <string>
#include <unordered_map>

#include "Interface.h"

//////////////////////////////////////////////////////////////////////////
/// @brief 返回目标尺寸
typedef std::tuple<double, double, double> (*fpGetObjectSize)(object_type type, int shape);

/// @brief 返回目标子类型
typedef object_subtype (*fpGetObjectSubtype)(object_type type, int shape);

/// @brief 停止当前仿真实验
typedef void (*fpStopSimulation)();

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
extern fpGetObjectSize getObjectSize;
extern fpGetObjectSubtype getObjectSubtype;

extern fpStopSimulation stopSimulation;

//////////////////////////////////////////////////////////////////////////

/*!
 *  @brief     index, runtime export functions to dll
 *  @remark    do not change order, append new item at the end
 */
enum class FunctionName : unsigned char {
    GetObjectSize = 0,
    GetObjectSubtype,
    StopSimulation
};

#ifdef _WIN32
    #ifndef PANOSIM_API
        #ifdef PANOSIM_API_EXPORTS
            #define PANOSIM_API __declspec(dllexport)
        #else // PANOSIM_API_EXPORTS
            #define PANOSIM_API __declspec(dllimport)
        #endif // PANOSIM_API_EXPORTS
    #endif // !PANOSIM_API
#else // _WIN32
    #define PANOSIM_API 
#endif // _WIN32

using MapFunction = std::unordered_map<FunctionName, void*>;
bool SetFunctions(const MapFunction& functions);
using VectorParam = std::vector<std::string>;
extern "C" bool PANOSIM_API Initialize(const VectorParam & params, const MapFunction & mapFunctions);
