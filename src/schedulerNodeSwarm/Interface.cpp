#include <Interface.h>

#include <algorithm>

#pragma warning(disable : 4819)
#include <boost/geometry.hpp>

#include <CommonDef.h>
#include <CsvFiles.h>
#include <RuntimeSettings.h>
#include <SchedulerNode.h>

std::tuple<double, double, double> GetObjectSize(object_type type, int shape)
{
    std::tuple<double, double, double> tupleResult;
    CsvFileCtrl.GetObjectSize(type, shape, tupleResult);
    return tupleResult;
}

object_subtype GetObjectSubtype(object_type type, int shape)
{
    object_subtype emResult = object_subtype::unknown;

    if (CsvFileCtrl.GetObjectSubtype(type, shape, emResult)) {
        return emResult;
    }

    return object_subtype::unknown;
}

void StopSimulation()
{
    CSchedulerNode::Instance()->RequestStopSimulation();
}

