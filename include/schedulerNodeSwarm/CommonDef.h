#pragma once


constexpr std::uint32_t DEFAULT_SHARED_MEMORY_MAX_SIZE = 1024;

constexpr char const* NULL_STRING = "";

#define BOOST_PI boost::math::constants::pi<double>()
#define Degrees2Radians(degrees)    ((degrees) * (BOOST_PI / 180.0))
#define Radians2Degrees(radians)    ((radians) * (180.0 / BOOST_PI))
#define em2i(em)    static_cast<std::int32_t>(em)
#define getTuple(tp, em) std::get<em2i(em)>(tp)

#ifdef _WIN32
#pragma pack(push, 1)
#endif // _WIN32

typedef struct _EgoBus {
    std::int32_t time;
    std::int32_t id;
    std::uint8_t type;
    std::int32_t shape;
    float x;
    float y;
    float z;
    float yaw;
    float pitch;
    float roll;
    float speed;
    std::uint8_t side;
#ifdef _WIN32
}EgoBus;
#else
} __attribute__((packed)) EgoBus;
#endif // _WIN32

//typedef struct _ClockBlockBus
//{
//    std::uint32_t time;
//    std::int32_t interval;
//#ifdef _WIN32
//}ClockBlockBus;
//#else
//} __attribute__((packed)) ClockBlockBus;
//#endif // _WIN32


#pragma pack(pop)

// TODO: (没有实现) ...
// UNDONE: (没有完成) ...
// HACK: (需要修改) ...
