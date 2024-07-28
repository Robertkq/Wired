#include "test_enums.h"
#include "test_types.h"
#include "wired.h"
#include <gtest/gtest.h>

void on_message_received(wired::connection<message_type>& conn,
                         wired::message<message_type>& msg) {
    WIRED_LOG_MESSAGE(wired::LOG_INFO,
                      "Message received with id: {} and size: {}",
                      msg.head().id(), msg.head().size());
    switch (msg.head().id()) {
    case message_type::single: {
        int value;
        msg >> value;
        WIRED_LOG_MESSAGE(wired::LOG_INFO, "Single message received: {}",
                          value);
        ASSERT_EQ(value, 6);
    } break;
    case message_type::vector: {
        std::vector<int> values;
        msg >> values;
        std::string values_str;
        for (auto& value : values) {
            values_str += std::to_string(value) + ", ";
        }
        WIRED_LOG_MESSAGE(wired::LOG_INFO,
                          "Vector message received with size {} and values: {}",
                          values.size(), values_str);
        ASSERT_EQ(values.size(), 3);
        ASSERT_EQ(values[0], 1);
        ASSERT_EQ(values[1], 2);
        ASSERT_EQ(values[2], 3);
    } break;
    }
}

class connection_tests_fixture : public ::testing::Test {
  public:
    using message_t = wired::message<message_type>;
    using connection_t = wired::connection<message_type>;
    using connection_ptr = std::shared_ptr<connection_t>;

  public:
    connection_tests_fixture()
        : io_context(), server_socket(io_context), client_socket(io_context),
          server_conn(nullptr), client_conn(nullptr), idle_work(io_context),
          io_thread([&]() { io_context.run(); }) {}

    ~connection_tests_fixture() {
        io_context.stop();
        io_thread.join();
    }

  protected:
    void SetUp() override {
        asio::ip::tcp::acceptor listener(
            io_context, asio::ip::tcp::endpoint(
                            asio::ip::address::from_string("127.0.0.1"), 0));
        asio::ip::tcp::endpoint server_endpoint = listener.local_endpoint();
        asio::error_code ec;

        WIRED_LOG_MESSAGE(wired::LOG_INFO, "Test server on {}:{}",
                          server_endpoint.address().to_string(),
                          server_endpoint.port());

        std::promise<void> server_promise;
        std::future<void> server_future = server_promise.get_future();

        listener.async_accept(server_socket, [&](asio::error_code ec) {
            if (!ec) {
                WIRED_LOG_MESSAGE(wired::LOG_INFO,
                                  "Server accepted connection");
                server_conn = std::make_shared<connection_t>(
                    io_context, std::move(server_socket));
            } else {
                WIRED_LOG_MESSAGE(wired::LOG_ERROR,
                                  "Server failed to accept connection");
            }
            server_promise.set_value();
        });

        std::promise<void> client_promise;
        std::future<void> client_future = client_promise.get_future();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        WIRED_LOG_MESSAGE(wired::LOG_INFO,
                          "Client trying to connect to 127.0.0.1:{}",
                          server_endpoint.port());
        client_socket.async_connect(
            asio::ip::tcp::endpoint(asio::ip::address::from_string("127.0.0.1"),
                                    server_endpoint.port()),
            [&](asio::error_code ec) {
                if (!ec) {
                    WIRED_LOG_MESSAGE(wired::LOG_INFO,
                                      "Client connected to server");
                    client_conn = std::make_shared<connection_t>(
                        io_context, std::move(client_socket));
                } else {
                    WIRED_LOG_MESSAGE(wired::LOG_ERROR,
                                      "Client failed to connect to server");
                }
                client_promise.set_value();
            });

        WIRED_LOG_MESSAGE(wired::LOG_INFO,
                          "Waiting for server and client to connect");
        server_future.wait();
        client_future.wait();

        ASSERT_NE(server_conn, nullptr);
        ASSERT_NE(client_conn, nullptr);

        server_conn->set_message_handler(on_message_received);
        client_conn->set_message_handler(on_message_received);
    }

    void TearDown() override {}

  protected:
    asio::io_context io_context;
    asio::ip::tcp::socket server_socket;
    asio::ip::tcp::socket client_socket;
    connection_ptr server_conn;
    connection_ptr client_conn;
    asio::io_context::work idle_work;
    std::thread io_thread;
};

TEST_F(connection_tests_fixture, connected) {
    EXPECT_EQ(server_conn->is_connected(), true);
    EXPECT_EQ(client_conn->is_connected(), true);
}

TEST_F(connection_tests_fixture, disconnect_server) {
    server_conn->disconnect();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(server_conn->is_connected(), false);
    EXPECT_EQ(client_conn->is_connected(), false);
}

TEST_F(connection_tests_fixture, disconnect_client) {
    client_conn->disconnect();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(server_conn->is_connected(), false);
    EXPECT_EQ(client_conn->is_connected(), false);
}

TEST_F(connection_tests_fixture, client_send) {
    message_t msg(message_type::single);
    msg << int(6);
    auto future = client_conn->send(msg);
    future.wait();
    EXPECT_GT(server_conn->incoming_messages_count(), 0);
}

TEST_F(connection_tests_fixture, server_send) {
    message_t msg(message_type::single);
    msg << int(6);
    auto future = server_conn->send(msg);
    future.wait();
    EXPECT_GT(client_conn->incoming_messages_count(), 0);
}