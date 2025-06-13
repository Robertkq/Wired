#ifndef WIRED_TYPES_H
#define WIRED_TYPES_H

#include <cstdint>
#include <string>
#include <asio/ssl.hpp>

namespace wired {

enum class tls_verify_mode {
    none,
    peer,
    fail_if_no_peer_cert
};

class tls_options {
  public:
    tls_options()
        : certificate_file_(""), private_key_file_(""),
          verify_mode_(tls_verify_mode::none) {}

    tls_options& set_certificate_file(const std::string& file) {
        certificate_file_ = file;
        return *this;
    }

    tls_options& set_private_key_file(const std::string& file) {
        private_key_file_ = file;
        return *this;
    }

    tls_options& set_verify_mode(tls_verify_mode mode) {
        verify_mode_ = mode;
        return *this;
    }

    // Getters for configuration options
    const std::string& certificate_file() const { return certificate_file_; }
    const std::string& private_key_file() const { return private_key_file_; }
    tls_verify_mode verify_mode() const { return verify_mode_; }

    asio::ssl::verify_mode to_asio_verify_mode() const {
        switch (verify_mode_) {
        case tls_verify_mode::none:
            return asio::ssl::verify_none;
        case tls_verify_mode::peer:
            return asio::ssl::verify_peer;
        case tls_verify_mode::fail_if_no_peer_cert:
            return asio::ssl::verify_fail_if_no_peer_cert;
        default:
            return asio::ssl::verify_none; // Fallback to none
        }
    }

    static void set_context_options(asio::ssl::context& ctx,
                                    const tls_options& options) {
        if (!options.certificate_file().empty()) {
            ctx.use_certificate_chain_file(options.certificate_file());
        }
        if (!options.private_key_file().empty()) {
            ctx.use_private_key_file(options.private_key_file(),
                                     asio::ssl::context::pem);
        }
        ctx.set_verify_mode(options.to_asio_verify_mode());
    }

  private:
    std::string certificate_file_; // Path to the certificate file
    std::string private_key_file_; // Path to the private key file
    tls_verify_mode verify_mode_;  // Custom verification mode
};

enum class message_strategy : uint8_t {
    normal = 1 << 0,
    immediate = 1 << 1
}; // enum class message_strategy

enum class execution_policy {
    blocking,
    non_blocking
}; // enum class execution_policy

struct selection_tag_0 {};
struct selection_tag_1 : selection_tag_0 {};
struct selection_tag_2 : selection_tag_1 {};

} // namespace wired

#endif // WIRED_TYPES_H