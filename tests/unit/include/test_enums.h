#include <cstdint>

enum class vehicle_type : std::uint8_t {
    car,
    truck,
    motorcycle,
    bicycle
};

enum class color : std::uint8_t {
    red,
    green,
    blue,
    yellow,
    black,
    white
};

enum class day_of_week : std::uint8_t {
    monday,
    tuesday,
    wednesday,
    thursday,
    friday,
    saturday,
    sunday
};