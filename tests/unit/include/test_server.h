#include "wired.h"
#include "test_enums.h"
#include "test_types.h"
#include <thread>
#include <array>

class server_t : public wired::server_interface<message_type> {
  public:
    using message_t = wired::message<message_type>;
    using connection_t = wired::connection<message_type>;
    using connection_ptr = std::shared_ptr<connection_t>;
    using ts_deque = wired::ts_deque<message_t>;

  public:
    server_t() : wired::server_interface<message_type>(), frequency({0}) {}
    ~server_t() override {}

    void on_message(message_t& msg, connection_ptr conn) override {
        std::cout << "Server received message with id: "
                  << static_cast<uint32_t>(msg.id()) << std::endl;
        ++frequency[static_cast<uint32_t>(msg.id())];
    }

    uint32_t get_frequency(message_type type) const {
        return frequency[static_cast<uint32_t>(type)];
    }

    std::array<uint32_t,
               static_cast<uint32_t>(message_type::message_type_count)>
        frequency;
};