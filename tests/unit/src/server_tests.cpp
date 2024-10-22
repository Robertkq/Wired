#include "test_enums.h"
#include "test_types.h"
#include "wired.h"
#include <gtest/gtest.h>

class test_server : public wired::server_interface<message_type> {
  public:
    test_server(uint16_t port);
};

test_server::test_server(uint16_t port)
    : wired::server_interface<message_type>(port) {}
class server_tests_fixture : public ::testing::Test {
  public:
    server_tests_fixture() : server(60000) {}

  protected:
    void SetUp() override {}

    void TearDown() override {}

  protected:
    test_server server;
};

TEST_F(server_tests_fixture, is_open) {
    server.start();
    ASSERT_EQ(server.is_open(), true);
    server.stop();
}

TEST_F(server_tests_fixture, update) {
    server.start();
    server.update(10);
    server.stop();
}