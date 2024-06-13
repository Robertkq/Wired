#pragma once

#include <cstdint>
#include <vector>

enum some_types : uint8_t {
    type1,
    type2,
    type3
};

class a_type {
  public:
    a_type() = default;
    a_type(int i) : i(i) {}
    int i;

    std::vector<uint8_t> wired_serialize() {
        return std::vector<uint8_t>{uint8_t(i)};
    }
};