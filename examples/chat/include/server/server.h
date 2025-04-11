#include <unordered_map>

#include "common.h"
#include "wired/server.h"

class server : public wired::server_interface<message_types> {
  public:
    server();

    server(const server&) = delete;
    server& operator=(const server&) = delete;

    ~server();

    void on_message(message_t& msg, connection_ptr conn) override;

  private:
    void on_client_message(message_t& msg, connection_ptr conn);

  private:
    std::unordered_map<uint32_t, std::string> clients;
};