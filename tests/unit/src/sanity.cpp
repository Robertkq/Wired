#include "wired/tools/log.h"
#include <gtest/gtest.h>

TEST(SanityTest, CheckEquality) {
    LOG_MESSAGE(LOG_DEBUG, "This is a debug message");
    LOG_MESSAGE(LOG_INFO, "This is an info message");
    LOG_MESSAGE(LOG_WARNING, "This is a warning message");
    LOG_MESSAGE(LOG_ERROR, "This is an error message");
    LOG_MESSAGE(LOG_CRITICAL, "This is a critical message");
    EXPECT_EQ(1, 1);
}