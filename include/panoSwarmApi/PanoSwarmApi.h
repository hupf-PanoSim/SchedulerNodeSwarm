#pragma once

#include <windows.h>
#include "Export.h"

bool Initialize(const VectorParam& params, const MapFunction& functions)
{
    return SetFunctions(functions);
}
