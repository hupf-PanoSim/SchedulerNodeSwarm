#pragma once

#include <Windows.h>

#include <map>

#include "Import.hpp"
#include "BusAccessor.hpp"


struct UserData
{
    std::uint32_t time;
    std::int32_t busId;
    void* state;
    std::string name;
    std::string outputPath;
    std::map<std::string, std::string> parameters;
};


extern "C" void PANOSIM_API ModelStart(UserData*);

extern "C" void PANOSIM_API ModelOutput(UserData*);

extern "C" void PANOSIM_API ModelTerminate(UserData*);


#define ReadBusHeader(bus_fmt, bus_accessor)            \
panosim_bus::read_header(BusFormat(panosim_bus::BusFormatTrait<bus_fmt>::str()), (reinterpret_cast<char*>(bus_accessor->GetHeader())))

#define WriteBusHeader(bus_fmt, bus_accessor, ...)      \
panosim_bus::write_header(BusFormat(panosim_bus::BusFormatTrait<bus_fmt>::str()), (reinterpret_cast<char*>(bus_accessor->GetHeader())), ##__VA_ARGS__)

#define ReadBusBody(bus_fmt, bus_accessor, index)       \
panosim_bus::read_body(BusFormat(panosim_bus::BusFormatTrait<bus_fmt>::str()), (reinterpret_cast<char*>(bus_accessor->GetBody(index))))

#define WriteBusBody(bus_fmt, bus_accessor, index, ...) \
panosim_bus::write_body(BusFormat(panosim_bus::BusFormatTrait<bus_fmt>::str()), (reinterpret_cast<char*>(bus_accessor->GetBody(index))), ##__VA_ARGS__)


#ifndef InterfaceInstantiation
#define InterfaceInstantiation  \
fpGetObjectSize getObjectSize;                                  \
fpGetObjectSubtype getObjectSubtype;                            \
fpGetObjectVertex getObjectVertex;                              \
fpGetEgoVertex getEgoVertex;                                    \
fpGetLaneLines getLaneLines;                                    \
fpGetWeatherInfo getWeatherInfo;                                \
fpGetTrafficLight getTrafficLight;                              \
fpGetTrafficSign _getTrafficSign;                               \
fpGetObstacle _getObstacle;                                     \
fpGetObstacleV2 _getObstacleV2;                                 \
fpGetParkingSpots getParkingSpots;                              \
fpGetParkingSpotsV2 getParkingSpotsV2;                          \
fpGetRSU _getRSU;                                               \
fpIsInsideJunction isInsideJunction;                            \
fpGetMapProjection getMapProjection;                            \
fpGetUtcOrigin getUtcOrigin;                                    \
fpGetTaskRoute getTaskRoute;                                    \
fpGetCurrentEdge getCurrentEdge;                                \
fpGetEdgeLanes getEdgeLanes;                                    \
fpGetLaneDirection getLaneDirection;                            \
fpGetLaneWidth getLaneWidth;                                    \
fpGetLaneShape getLaneShape;                                    \
fpGetJunctionShape getJunctionShape;                            \
fpStopSimulation stopSimulation;                                \
fpGetLaneSpeedLimit getLaneSpeedLimit;                          \
fpGetKeyPoints getKeyPoints;                                    \
fpGetTargetParkingSpot getTargetParkingSpot;                    \
fpGetEdgeByLane getEdgeByLane;                                  \
fpGetWayPoints getWayPoints;                                    \
fpGetSpeedTable getSpeedTable;                                  \
fpGetStartPoint getStartPoint;                                  \
bool Initialize(                                                \
    const VectorParam& params, const MapFunction& functions) {  \
    return SetFunctions(functions); }                           \
std::vector<TrafficSignData> getTrafficSign(                    \
    double distance = 200) {                                    \
    return _getTrafficSign(distance); }                         \
std::vector<ObstacleData> getObstacle(double distance = 200) {  \
    return _getObstacle(distance); }                            \
std::vector<ObstacleDataV2> getObstacleV2(                      \
    double distance = 200) {                                    \
    return _getObstacleV2(distance); }                          \
std::vector<RsuData> getRSU(double distance = 200) {            \
    return _getRSU(distance); }
#endif // !InterfaceInstantiation
