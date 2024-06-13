#pragma once

#include <cstdint>
#include <cstring>
#include <vector>


class serializable_only_type {
  public:
    serializable_only_type(uint32_t w, uint64_t f) : weight(w), factor(f) {}
    std::vector<uint8_t> wired_serialize() const {
        std::vector<uint8_t> v;
        v.resize(sizeof(weight) + sizeof(factor));
        std::memcpy(v.data(), &weight, sizeof(weight));
        std::memcpy(v.data() + sizeof(weight), &factor, sizeof(factor));
        return v;
    }

  public:
    uint32_t weight;
    uint64_t factor;
};

class deserializable_only_type {
  public:
    deserializable_only_type(uint32_t w, uint64_t f) : weight(w), factor(f) {}
    void wired_deserialize(const std::vector<uint8_t>& v) {
        std::memcpy(&weight, v.data(), sizeof(weight));
        std::memcpy(&factor, v.data() + sizeof(weight), sizeof(factor));
    }

  public:
    uint32_t weight;
    uint64_t factor;
};

class wired_type {
  public:
    wired_type(uint32_t w, uint64_t f) : weight(w), factor(f) {}
    std::vector<uint8_t> wired_serialize() const {
        std::vector<uint8_t> v;
        v.resize(sizeof(weight) + sizeof(factor));
        std::memcpy(v.data(), &weight, sizeof(weight));
        std::memcpy(v.data() + sizeof(weight), &factor, sizeof(factor));
        return v;
    }
    void wired_deserialize(const std::vector<uint8_t>& v) {
        std::memcpy(&weight, v.data(), sizeof(weight));
        std::memcpy(&factor, v.data() + sizeof(weight), sizeof(factor));
    }

  public:
    uint32_t weight;
    uint64_t factor;
};