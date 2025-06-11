#include "wired.h"

#include "test_enums.h"
#include "test_types.h"

#include <gtest/gtest.h>

class message_tests_fixture : public ::testing::Test {
  public:
    message_tests_fixture() {}

  protected:
    void SetUp() override {}

    void TearDown() override {}

    wired::message<message_type> msg;
    std::vector<int> vector1 = {40, 41, 42, 43, 44};
    std::vector<int> vector2 = {45, 46, 47, 48, 49};
};

TEST_F(message_tests_fixture, default_constructor) {
    msg.id(message_type::single);
    EXPECT_EQ(msg.id(), message_type::single);
    EXPECT_EQ(msg.head().id(), message_type::single);
    EXPECT_EQ(msg.from(), nullptr);
    EXPECT_EQ(msg.head().size(), 0);
    EXPECT_EQ(msg.head().timestamp(), 0);
    EXPECT_EQ(msg.body().data().size(), 0);
}

TEST_F(message_tests_fixture, input_single) {
    msg.id(message_type::single);
    msg << int(42);
    EXPECT_EQ(msg.head().id(), message_type::single);
    EXPECT_EQ(msg.body().data().size(), sizeof(int));
    EXPECT_EQ(msg.body().data()[0], 42);
    EXPECT_EQ(msg.head().size(), msg.body().data().size());
}

TEST_F(message_tests_fixture, input_single_multi) {
    msg.id(message_type::single);
    msg << int(42) << int(43) << int(44);
    EXPECT_EQ(msg.body().data().size(), 3 * sizeof(int));
    EXPECT_EQ(msg.head().size(), msg.body().data().size());
}

TEST_F(message_tests_fixture, input_vector) {
    msg.id(message_type::vector);
    msg << vector1;
    EXPECT_EQ(msg.id(), message_type::vector);
    EXPECT_EQ(msg.body().data().size(),
              vector1.size() * sizeof(int) + sizeof(size_t));
}

TEST_F(message_tests_fixture, input_vector_multi) {
    msg.id(message_type::vector);
    msg << vector1;
    msg << vector2;
    EXPECT_EQ(msg.id(), message_type::vector);
    EXPECT_EQ(msg.body().data().size(),
              vector1.size() * sizeof(int) + sizeof(size_t) +
                  vector2.size() * sizeof(int) + sizeof(size_t));
}

TEST_F(message_tests_fixture, input_wired_serialize) {
    serializable_only_type sot{42, 43};
    auto reference = sot.wired_serialize();
    msg.id(message_type::wired_serialize);
    msg << sot;
    EXPECT_EQ(msg.id(), message_type::wired_serialize);
    EXPECT_EQ(msg.body().data().size(), reference.size() + sizeof(size_t));
}

TEST_F(message_tests_fixture, input_wired_serialize_multi) {
    serializable_only_type sot1{42, 43};
    serializable_only_type sot2{52, 53};
    auto reference1 = sot1.wired_serialize();
    auto reference2 = sot2.wired_serialize();
    msg.id(message_type::wired_serialize);
    msg << sot1 << sot2;
    EXPECT_EQ(msg.id(), message_type::wired_serialize);
    EXPECT_EQ(msg.body().data().size(),
              reference1.size() + reference2.size() + 2 * sizeof(size_t));
}

TEST_F(message_tests_fixture, output_single) {
    msg << int(42);
    int value;
    msg >> value;
    EXPECT_EQ(value, 42);
    EXPECT_EQ(msg.body().data().size(), 0);
    EXPECT_EQ(msg.head().size(), 0);
}

TEST_F(message_tests_fixture, output_single_multi) {
    msg << int(42) << int(43) << int(44);
    int value1;
    int value2;
    int value3;
    msg >> value3 >> value2 >> value1;
    EXPECT_EQ(value3, 44);
    EXPECT_EQ(value2, 43);
    EXPECT_EQ(value1, 42);
    EXPECT_EQ(msg.body().data().size(), 0);
    EXPECT_EQ(msg.head().size(), 0);
}

TEST_F(message_tests_fixture, output_vector) {
    msg << vector1;
    std::vector<int> vector2;
    msg >> vector2;
    EXPECT_EQ(vector1, vector2);
    EXPECT_EQ(msg.body().data().size(), 0);
    EXPECT_EQ(msg.head().size(), 0);
}

TEST_F(message_tests_fixture, output_vector_multi) {
    msg << vector1 << vector2;
    std::vector<int> vector3;
    std::vector<int> vector4;
    msg >> vector3 >> vector4;
    EXPECT_EQ(vector1, vector4);
    EXPECT_EQ(vector2, vector3);
    EXPECT_EQ(msg.body().data().size(), 0);
    EXPECT_EQ(msg.head().size(), 0);
}

TEST_F(message_tests_fixture, output_wired_serialize) {
    deserializable_only_type dot{42, 43};
    msg << dot.weight;
    msg << dot.factor;
    msg << sizeof(uint32_t) + sizeof(uint64_t);
    dot.weight = 0;
    dot.factor = 0;
    msg >> dot;
    EXPECT_EQ(dot.weight, 42);
    EXPECT_EQ(dot.factor, 43);
    EXPECT_EQ(msg.body().data().size(), 0);
    EXPECT_EQ(msg.head().size(), 0);
}

TEST_F(message_tests_fixture, output_wired_serialize_multi) {
    deserializable_only_type dot1{42, 43};
    deserializable_only_type dot2{52, 53};
    msg << dot1.weight;
    msg << dot1.factor;
    msg << sizeof(uint32_t) + sizeof(uint64_t);
    msg << dot2.weight;
    msg << dot2.factor;
    msg << sizeof(uint32_t) + sizeof(uint64_t);
    dot1.weight = 0;
    dot1.factor = 0;
    dot2.weight = 0;
    dot2.factor = 0;
    msg >> dot2 >> dot1;
    EXPECT_EQ(dot1.weight, 42);
    EXPECT_EQ(dot1.factor, 43);
    EXPECT_EQ(dot2.weight, 52);
    EXPECT_EQ(dot2.factor, 53);
    EXPECT_EQ(msg.body().data().size(), 0);
    EXPECT_EQ(msg.head().size(), 0);
}

TEST_F(message_tests_fixture, reset) {
    msg << int(42);
    msg.reset();
    EXPECT_EQ(msg.head().id(), message_type::single);
    EXPECT_EQ(msg.from(), nullptr);
    EXPECT_EQ(msg.head().size(), 0);
    EXPECT_EQ(msg.head().timestamp(), 0);
    EXPECT_EQ(msg.body().data().size(), 0);
}