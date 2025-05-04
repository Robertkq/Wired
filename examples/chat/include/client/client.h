#include <cstdint>
#include <thread>
#include <string>

#include "common.h"
#include "wired/client.h"

class client : public wired::client_interface<message_types> {
  public:
    client();

    client(const client&) = delete;
    client& operator=(const client&) = delete;

    ~client();

    void on_message(message_t& msg, connection_ptr conn) override;

  public:
    std::string name;
    uint32_t id;
};