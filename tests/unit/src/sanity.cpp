#include "wired/tools/log.h"
#include <gtest/gtest.h>

TEST(SanityTest, CheckEquality) {
    WIRED_LOG_MESSAGE(wired::LOG_DEBUG, "This is a debug message");
    WIRED_LOG_MESSAGE(wired::LOG_INFO, "This is an info message");
    WIRED_LOG_MESSAGE(wired::LOG_WARNING, "This is a warning message");
    WIRED_LOG_MESSAGE(wired::LOG_ERROR, "This is an error message");
    WIRED_LOG_MESSAGE(wired::LOG_CRITICAL, "This is a critical message");
    EXPECT_EQ(1, 1);
}