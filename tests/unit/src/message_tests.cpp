#include "test_enums.h"
#include "test_types.h"
#include "wired.h"
#include <gtest/gtest.h>

class message_tests_fixture : public ::testing::Test {
  public:
    message_tests_fixture() : msg() {}

  protected:
    void SetUp() override {}

    void TearDown() override {}

  protected:
    wired::message<message_type> msg;
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
    std::vector<int> v = {40, 41, 42, 43, 43};
    msg.id(message_type::vector);
    msg << v;
    EXPECT_EQ(msg.id(), message_type::vector);
    EXPECT_EQ(msg.body().data().size(),
              v.size() * sizeof(int) + sizeof(size_t));
}

TEST_F(message_tests_fixture, input_vector_multi) {
    std::vector<int> v1 = {40, 41, 42, 43, 43};
    std::vector<int> v2 = {50, 51, 52, 53, 53};
    msg.id(message_type::vector);
    msg << v1;
    msg << v2;
    EXPECT_EQ(msg.id(), message_type::vector);
    EXPECT_EQ(msg.body().data().size(),
              v1.size() * sizeof(int) + sizeof(size_t) +
                  v2.size() * sizeof(int) + sizeof(size_t));
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
    int i;
    msg >> i;
    EXPECT_EQ(i, 42);
    EXPECT_EQ(msg.body().data().size(), 0);
    EXPECT_EQ(msg.head().size(), 0);
}

TEST_F(message_tests_fixture, output_single_multi) {
    msg << int(42) << int(43) << int(44);
    int i1, i2, i3;
    msg >> i3 >> i2 >> i1;
    EXPECT_EQ(i3, 44);
    EXPECT_EQ(i2, 43);
    EXPECT_EQ(i1, 42);
    EXPECT_EQ(msg.body().data().size(), 0);
    EXPECT_EQ(msg.head().size(), 0);
}

TEST_F(message_tests_fixture, output_vector) {
    std::vector<int> v = {40, 41, 42, 43, 43};
    msg << v;
    std::vector<int> v2;
    msg >> v2;
    EXPECT_EQ(v, v2);
    EXPECT_EQ(msg.body().data().size(), 0);
    EXPECT_EQ(msg.head().size(), 0);
}

TEST_F(message_tests_fixture, output_vector_multi) {
    std::vector<int> v1 = {40, 41, 42, 43, 43};
    std::vector<int> v2 = {50, 51, 52, 53, 53};
    msg << v1 << v2;
    std::vector<int> v3, v4;
    msg >> v3 >> v4;
    EXPECT_EQ(v1, v4);
    EXPECT_EQ(v2, v3);
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