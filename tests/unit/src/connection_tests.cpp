#include "wired.h"

#include "test_enums.h"
#include "test_types.h"

#include <gtest/gtest.h>
#include <thread>
#include <mutex>

class connection_tests_fixture : public ::testing::Test {
  public:
    using message_t = wired::message<message_type>;
    using connection_t = wired::connection<message_type>;
    using connection_ptr = std::shared_ptr<connection_t>;
    using ts_deque = wired::ts_deque<message_t>;

  public:
    connection_tests_fixture()
        : io_context(), server_conn(nullptr), client_conn(nullptr),
          idle_work(io_context.get_executor()),
          io_thread([&]() { io_context.run(); }) {}

    ~connection_tests_fixture() {
        io_context.stop();
        io_thread.join();
    }

  protected:
    void SetUp() override {

        verify_certificates();

        ssl_context_client.set_verify_mode(asio::ssl::verify_none);
        ssl_context_server.set_verify_mode(asio::ssl::verify_none);

        ssl_context_server.use_certificate_chain_file("../server.crt");
        ssl_context_server.use_private_key_file("../server.key",
                                                asio::ssl::context::pem);

        asio::ip::tcp::acceptor listener(
            io_context,
            asio::ip::tcp::endpoint(asio::ip::make_address("127.0.0.1"), 0));
        asio::ip::tcp::endpoint server_endpoint = listener.local_endpoint();
        asio::error_code ec;

        WIRED_LOG_MESSAGE(wired::LOG_INFO, "Test server on {}:{}",
                          server_endpoint.address().to_string(),
                          server_endpoint.port());

        std::promise<bool> server_promise;
        std::future<bool> server_future = server_promise.get_future();

        // Server accepts connection and performs SSL handshake
        listener.async_accept([&](asio::error_code ec,
                                  asio::ip::tcp::socket socket) {
            if (!ec) {
                server_conn = std::make_shared<connection_t>(
                    io_context, ssl_context_server, std::move(socket),
                    server_incoming_messages, placeholder_cv);
                WIRED_LOG_MESSAGE(wired::LOG_INFO,
                                  "Server accepted connection with address: {}",
                                  (void*)server_conn.get());

                // Perform SSL handshake on the server side
                server_conn->ssl_stream().async_handshake(
                    asio::ssl::stream_base::server,
                    [&](const asio::error_code& handshake_error) {
                        if (!handshake_error) {
                            WIRED_LOG_MESSAGE(
                                wired::LOG_INFO,
                                "Server SSL handshake successful");
                            server_promise.set_value(true);
                            server_conn->start_listening();
                        } else {
                            WIRED_LOG_MESSAGE(
                                wired::LOG_ERROR,
                                "Server SSL handshake failed with error: {}",
                                handshake_error.message());
                            server_promise.set_value(false);
                        }
                    });
            } else {
                WIRED_LOG_MESSAGE(wired::LOG_ERROR,
                                  "Server failed to accept connection");
                server_promise.set_value(false);
            }
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        WIRED_LOG_MESSAGE(wired::LOG_INFO,
                          "Client trying to connect to 127.0.0.1:{}",
                          server_endpoint.port());

        // Client connects to the server and performs SSL handshake

        client_conn = std::make_shared<connection_t>(
            io_context, ssl_context_client, asio::ip::tcp::socket(io_context),
            client_incoming_messages, placeholder_cv);

        asio::ip::tcp::resolver resolver(io_context);
        asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(
            "127.0.0.1", std::to_string(server_endpoint.port()));

        auto client_future = client_conn->connect(endpoints);

        WIRED_LOG_MESSAGE(wired::LOG_INFO,
                          "Waiting for server and client to connect");
        ASSERT_EQ(server_future.get(), true);
        ASSERT_EQ(client_future.get(), true);

        ASSERT_NE(server_conn, nullptr);
        ASSERT_NE(client_conn, nullptr);
    }

    void TearDown() override {}

  protected:
    std::condition_variable placeholder_cv;
    asio::io_context io_context;
    asio::ssl::context ssl_context_client{asio::ssl::context::tls_client};
    asio::ssl::context ssl_context_server{asio::ssl::context::tls_server};
    connection_ptr server_conn;
    connection_ptr client_conn;
    ts_deque server_incoming_messages;
    ts_deque client_incoming_messages;
    asio::executor_work_guard<asio::io_context::executor_type> idle_work;
    std::thread io_thread;
};

TEST_F(connection_tests_fixture, connected) {
    EXPECT_EQ(server_conn->is_connected(), true);
    EXPECT_EQ(client_conn->is_connected(), true);
}

TEST_F(connection_tests_fixture, disconnect_server) {
    auto future = server_conn->disconnect();
    EXPECT_EQ(future.get(), true);
    EXPECT_EQ(server_conn->is_connected(), false);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(client_conn->is_connected(), false);
}

TEST_F(connection_tests_fixture, disconnect_client) {
    auto future = client_conn->disconnect();
    EXPECT_EQ(future.get(), true);
    EXPECT_EQ(client_conn->is_connected(), false);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(server_conn->is_connected(), false);
}

TEST_F(connection_tests_fixture, client_send) {
    message_t msg(message_type::single);
    msg << int(6);
    auto future = client_conn->send(msg, wired::message_strategy::immediate);
    future.wait();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_GT(server_conn->incoming_messages_count(), 0);
}

TEST_F(connection_tests_fixture, server_send) {
    message_t msg(message_type::single);
    msg << int(6);
    auto future = server_conn->send(msg, wired::message_strategy::immediate);
    future.wait();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_GT(client_conn->incoming_messages_count(), 0);
}