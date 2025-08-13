#include <PanoSwarmApi.h>
#include <string>
#include <string_view>
#include <vector>
#include <iostream>

using std::string;
using std::string_view;
using std::vector;
using std::cout;
using std::endl;
using std::move;

using PanoSimSensorBus::LIDAR_OBJLIST_G_FORMAT;
using PanoSimSensorBus::Lidar_ObjList_G;
using PanoSimBasicsBus::EGO_CONTROL_FORMAT;
using PanoSimBasicsBus::EgoControl;

struct GlobalData {
    BusAccessor* lidar;
    BusAccessor* ego_control;
};

void PrintParameters(UserData* userData);
void SplitString(const string_view strSrc, const string_view strSeparator, vector<string>& vctSplitResult);

void ModelStart(UserData* userData) {
    PrintParameters(userData);

    auto pGlobal = new GlobalData;
    pGlobal->lidar = new BusAccessor(userData->busId, "Lidar_ObjList_G.0", LIDAR_OBJLIST_G_FORMAT);
    pGlobal->ego_control = new BusAccessor(userData->busId, "ego_control", EGO_CONTROL_FORMAT);
    userData->state = pGlobal;
}

void ModelOutput(UserData* userData) {
    if (userData != nullptr) {
        auto pGlobal = static_cast<GlobalData*>(userData->state);
        if (pGlobal != nullptr) {
            Lidar_ObjList_G* pLidar = nullptr;
            if (pGlobal->lidar != nullptr) {
                pLidar = static_cast<Lidar_ObjList_G*>(pGlobal->lidar->GetHeader());
            }
            EgoControl* pEgoCtrl = nullptr;
            if (pGlobal->ego_control != nullptr) {
                pEgoCtrl = static_cast<EgoControl*>(pGlobal->ego_control->GetHeader());
            }
            if (pLidar != nullptr && pEgoCtrl != nullptr)
            {
                if (pLidar->header.width > 0) {
                    cout << "==============================" << endl;
                    pEgoCtrl->time = userData->time;
                    pEgoCtrl->valid = 1;
                    pEgoCtrl->throttle = 0;
                    pEgoCtrl->brake = 10000000;
                    pEgoCtrl->steer = 0;
                    pEgoCtrl->mode = 0;
                    pEgoCtrl->gear = 0;
                }
            }
        }
    }
}

void ModelTerminate(UserData* userData)
{
    if (userData->state != nullptr) {
        auto pGlobal = static_cast<GlobalData*>(userData->state);
        if (pGlobal != nullptr) {
            if (pGlobal->lidar != nullptr) {
                delete pGlobal->lidar;
                pGlobal->lidar = nullptr;
            }
            if (pGlobal->ego_control != nullptr) {
                delete pGlobal->ego_control;
                pGlobal->ego_control = nullptr;
            }
            delete pGlobal;
            userData->state = nullptr;
        }
    }
}

void SplitString(const string_view strSrc, const string_view strSeparator, vector<string>& vctSplitResult)
{
    vctSplitResult.clear();
    string::size_type nBegin = 0;
    string::size_type nEnd = strSrc.find(strSeparator);
    while (string::npos != nEnd) {
        vctSplitResult.emplace_back(move(strSrc.substr(nBegin, nEnd - nBegin)));
        nBegin = nEnd + strSeparator.size();
        nEnd = strSrc.find(strSeparator, nBegin);
    }
    if (nBegin != strSrc.length()) {
        vctSplitResult.emplace_back(move(strSrc.substr(nBegin)));
    }
}

void PrintParameters(UserData* userData)
{
    for (const auto& pairItem : userData->parameters) {
        cout << pairItem.first << ":" << pairItem.second << endl;
    }

    cout << userData->busId << endl;
    cout << userData->name << endl;

    const char* key_parameter = "Parameters";
    auto findParam = userData->parameters.find(key_parameter);
    if (findParam != userData->parameters.end()) {
        vector<string> vctParameter;
        constexpr string_view parameter_separator = ",";
        SplitString(findParam->second, parameter_separator, vctParameter);
        for (const auto& strParameter : vctParameter) {
            cout << strParameter << endl;
        }
    }
}
