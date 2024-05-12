#include "test_enums.h"
#include <gtest/gtest.h>
#include <wired.h>

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
    EXPECT_EQ(msg.from(), nullptr);
    EXPECT_EQ(msg.head().id(), message_type::single);
    EXPECT_EQ(msg.head().size(), 0);
    EXPECT_EQ(msg.head().timestamp(), 0);
    EXPECT_EQ(msg.body().data().size(), 0);
}

TEST_F(message_tests_fixture, input_single) {
    msg << int(42);
    EXPECT_EQ(msg.body().data().size(), 1);
    EXPECT_EQ(msg.body().data()[0], 42);
}

TEST_F(message_tests_fixture, input_single_multi) {
    msg << int(42) << int(43) << int(44);
    EXPECT_EQ(msg.body().data().size(), 3);
    EXPECT_EQ(msg.body().data()[0], 42);
    EXPECT_EQ(msg.body().data()[1], 43);
    EXPECT_EQ(msg.body().data()[2], 44);
}

TEST_F(message_tests_fixture, input_vector) {
    std::vector<int> v = {40, 41, 42, 43, 43};
    msg << v;
    EXPECT_EQ(msg.body().data().size(), v.size() + 1);
    for (size_t i = 0; i < v.size(); ++i) {
        EXPECT_EQ(msg.body().data()[i], v[i]);
    }
    EXPECT_EQ(msg.body().data()[5], 5);
}

TEST_F(message_tests_fixture, input_vector_multi) {
    std::vector<int> v1 = {40, 41, 42, 43, 43};
    std::vector<int> v2 = {50, 51, 52, 53, 53};
    msg << v1 << v2;
    EXPECT_EQ(msg.body().data().size(), v1.size() + v2.size() + 2);
    for (size_t i = 0; i < v1.size(); ++i) {
        EXPECT_EQ(msg.body().data()[i], v1[i]);
    }
    EXPECT_EQ(msg.body().data()[5], v1.size());
    for (size_t i = 0; i < v2.size(); ++i) {
        EXPECT_EQ(msg.body().data()[i + 6], v2[i]);
    }
    EXPECT_EQ(msg.body().data()[11], v2.size());
}

// TEST_F(message_tests_fixture, input_wired_serialize) { EXPECT_EQ(1, 1); }

// TEST_F(message_tests_fixture, output_single) { EXPECT_EQ(1, 1); }

// TEST_F(message_tests_fixture, output_vector) { EXPECT_EQ(1, 1); }

// TEST_F(message_tests_fixture, output_wired_serialize) { EXPECT_EQ(1, 1); }