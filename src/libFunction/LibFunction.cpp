// LibFunction.cpp : 定义静态库的函数。
//

#include "pch.h"
#include "framework.h"
#include <Import.h>

#define DefineFunction(Upper, Lower) void SetFunc##Upper(fp##Upper fp) { Lower = fp; }

#define FindAndSetFunction(f) {                               \
    auto findFunction = functions.find(FunctionName::##f);    \
    if (findFunction == functions.end()) { return false; }    \
    SetFunc##f(fp##f(findFunction->second));                  \
}

//////////////////////////////////////////////////////////////////////////
fpGetObjectSize getObjectSize;
fpGetObjectSubtype getObjectSubtype;
fpStopSimulation stopSimulation;


//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
DefineFunction(GetObjectSize, getObjectSize)
DefineFunction(GetObjectSubtype, getObjectSubtype)
DefineFunction(StopSimulation, stopSimulation)
//////////////////////////////////////////////////////////////////////////

bool SetFunctions(const MapFunction& functions)
{
    FindAndSetFunction(GetObjectSize);
    FindAndSetFunction(GetObjectSubtype);
    FindAndSetFunction(StopSimulation);

    return true;
}
