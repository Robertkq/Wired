#include "test_enums.h"
#include "test_types.h"
#include "wired.h"
#include <gtest/gtest.h>

class connection_tests_fixture : public ::testing::Test {
  public:
    using message_t = wired::message<message_type>;
    using connection_t = wired::connection<message_type>;
    using connection_ptr = std::shared_ptr<connection_t>;

  public:
    connection_tests_fixture()
        : io_context(), server_socket(io_context), client_socket(io_context),
          server_messages(), client_messages(), server_conn(nullptr),
          client_conn(nullptr), idle_work(io_context),
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

        LOG_MESSAGE(LOG_INFO, "Test server on {}:{}",
                    server_endpoint.address().to_string(),
                    server_endpoint.port());

        std::promise<void> server_promise;
        std::future<void> server_future = server_promise.get_future();

        listener.async_accept(server_socket, [&](asio::error_code ec) {
            if (!ec) {
                LOG_MESSAGE(LOG_INFO, "Server accepted connection");
                server_conn = std::make_shared<connection_t>(
                    io_context, std::move(server_socket), server_messages);
            } else {
                LOG_MESSAGE(LOG_ERROR, "Server failed to accept connection");
            }
            server_promise.set_value();
        });

        std::promise<void> client_promise;
        std::future<void> client_future = client_promise.get_future();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        LOG_MESSAGE(LOG_INFO, "Client trying to connect to 127.0.0.1:{}",
                    server_endpoint.port());
        client_socket.async_connect(
            asio::ip::tcp::endpoint(asio::ip::address::from_string("127.0.0.1"),
                                    server_endpoint.port()),
            [&](asio::error_code ec) {
                if (!ec) {
                    LOG_MESSAGE(LOG_INFO, "Client connected to server");
                    client_conn = std::make_shared<connection_t>(
                        io_context, std::move(client_socket), client_messages);
                } else {
                    LOG_MESSAGE(LOG_ERROR,
                                "Client failed to connect to server");
                }
                client_promise.set_value();
            });

        LOG_MESSAGE(LOG_INFO, "Waiting for server and client to connect");
        server_future.wait();
        client_future.wait();

        ASSERT_NE(server_conn, nullptr);
        ASSERT_NE(client_conn, nullptr);
    }

    void TearDown() override {}

  protected:
    asio::io_context io_context;
    asio::ip::tcp::socket server_socket;
    asio::ip::tcp::socket client_socket;
    wired::ts_deque<message_t> server_messages;
    wired::ts_deque<message_t> client_messages;
    connection_ptr server_conn;
    connection_ptr client_conn;
    asio::io_context::work idle_work;
    std::thread io_thread;
};

TEST_F(connection_tests_fixture, connected) {
    EXPECT_EQ(server_conn->is_connected(), true);
    EXPECT_EQ(client_conn->is_connected(), true);
}

TEST_F(connection_tests_fixture, send) {}