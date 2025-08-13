#include <PanoSwarmApi.hpp>

InterfaceInstantiation;

DEFINE_BUS_FORMAT(TrafficAllBusFormat, "time@i,100@[,id@i,type@b,shape@i,x@f,y@f,z@f,yaw@f,pitch@f,roll@f,speed@f")
DEFINE_BUS_FORMAT(GlobalVarBusFormat, "time@i,variable@d")

struct GlobalData {
    DoubleBusReader<TrafficAllBusFormat>* ptr_traffic_bus;
    BusAccessor<GlobalVarBusFormat>* ptr_var_bus;
};

void ModelStart(UserData* userData) {
    auto pGlobal = new GlobalData;
    pGlobal->ptr_traffic_bus = new DoubleBusReader<TrafficAllBusFormat>(userData->busId, "traffic");
    pGlobal->ptr_var_bus = new BusAccessor<GlobalVarBusFormat>(userData->busId, "global.1");
    userData->state = pGlobal;
}

void ModelOutput(UserData* userData) {
    auto pGlobal = static_cast<GlobalData*>(userData->state);
    auto pReader = pGlobal->ptr_traffic_bus->GetReader(userData->time);
    const auto& [timestamp, width] = pReader->ReadHeader();
    printf("TrafficBus: %d, %d\n", timestamp, width);
    for (auto i = 0; i < width; ++i) {
        const auto& [id, type, shape, x, y, z, yaw, pitch, roll, speed] = pReader->ReadBody(i);
        printf("%d,%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
            id, type, shape, x, y, z, yaw, pitch, roll, speed);
    }
    pGlobal->ptr_var_bus->WriteHeader(userData->time, 2 * std::asin(1.0));
}

void ModelTerminate(UserData* userData) {
}
