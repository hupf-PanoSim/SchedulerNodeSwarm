#pragma once

#include <string>
#include <vector>

/// @brief 目标类型
enum class object_type : unsigned char
{
    equipment = 0,/*< 车辆(Car/Van/Bus/OtherVehicle) */
    pedestrian,/*< 行人 */
    other,/*< 其他目标 */
    obstacle,/*< 障碍物 */

    unknown/*< 未知目标类型 */
};

/// @brief 目标子类型
enum class object_subtype : unsigned char
{
    Car = 0,
    Van,
    Bus,
    OtherVehicle,
    Pedestrian,
    NonMotorVehicle,
    Others,

    unknown/*< 未知目标子类型 */
};

/// @brief 角点坐标
using VctVertex = std::vector<std::tuple<double, double, double>>;

/// @brief 返回目标尺寸
std::tuple<double, double, double> GetObjectSize(object_type type, int shape);

/// @brief 返回目标子类型
object_subtype GetObjectSubtype(object_type type, int shape);

/// @brief 停止当前仿真实验
void StopSimulation();
