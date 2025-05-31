#include "wired.h"
#include "test_client.h"
#include "test_server.h"

#include <gtest/gtest.h>
#include <thread>

class client_server_tests_fixture : public ::testing::Test {
  public:
    using message_t = wired::message<message_type>;
    using connection_t = wired::connection<message_type>;
    using connection_ptr = std::shared_ptr<connection_t>;
    using ts_deque = wired::ts_deque<message_t>;

  public:
    void SetUp() override {
        wired::tls_options options;
        options.set_certificate_file("../server.crt")
            .set_private_key_file("../server.key")
            .set_verify_mode(wired::tls_verify_mode::none);
        server_.set_tls_options(options);
        server_.start("60000");
        server_.run(wired::execution_policy::non_blocking);
        for (auto& client : clients_) {
            auto future = client.connect("localhost", "60000");
            ASSERT_TRUE(future.get());
            ASSERT_TRUE(client.is_connected());
            client.run(wired::execution_policy::non_blocking);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        std::cout << " ===== Server and clients are set up ===== " << std::endl;
    }

    void TearDown() override {
        std::cout << " ===== Server and clients are tearing down ===== "
                  << std::endl;
        for (auto& client : clients_) {
            if (client.is_connected()) {
                auto future = client.disconnect();
                ASSERT_TRUE(future.get());
                ASSERT_FALSE(client.is_connected());
            }
        }
        if (server_.is_listening()) {
            server_.shutdown();
        }
    }

  protected:
    server_t server_;
    std::array<client_t, 3> clients_;
};

TEST_F(client_server_tests_fixture, assert_connections) {
    ASSERT_TRUE(server_.is_listening());
    for (auto& client : clients_) {
        ASSERT_TRUE(client.is_connected());
    }
}

TEST_F(client_server_tests_fixture, server_disconnect) {
    server_.shutdown();
    ASSERT_FALSE(server_.is_listening());
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    for (auto& client : clients_) {
        ASSERT_FALSE(client.is_connected());
    }
}

TEST_F(client_server_tests_fixture, client_disconnect) {
    for (auto& client : clients_) {
        auto future = client.disconnect();
        ASSERT_TRUE(future.get());
        ASSERT_FALSE(client.is_connected());
    }
}

TEST_F(client_server_tests_fixture, server_send) {
    message_t msg(message_type::server_message);
    std::cout << server_.connections().size() << " connections" << std::endl;
    auto results = server_.send_all(nullptr, msg);

    for (auto& result : results) {
        ASSERT_TRUE(result.get());
    }
    int count = 0;
    while (count < 3) {
        count = 0;
        for (auto& client : clients_) {
            count += client.get_frequency(message_type::server_message);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

TEST_F(client_server_tests_fixture, client_send) {
    message_t msg(message_type::client_message);
    for (auto& client : clients_) {
        auto results = client.send(msg);
        ASSERT_TRUE(results.get());
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_EQ(server_.get_frequency(message_type::client_message), 3);
}