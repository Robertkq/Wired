#include "test_enums.h"
#include <gtest/gtest.h>
#include <wired.h>

class messageTextFixture : public ::testing::Test {
  protected:
    void SetUp() override {
        // Setup code
    }

    void TearDown() override {
        // TearDown code
    }

  protected:
    wired::message<vehicle_type> veh_msg;
    wired::message<color> col_msg;
    wired::message<day_of_week> dow_msg;
};