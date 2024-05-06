#ifndef CLIENT_H
#define CLIENT_H

#include <asio.hpp>
#include <iostream>

class Client {
  public:
    int hello;
    asio::io_context io_context;

  private:
    // Private member variables and methods
};

#endif // CLIENT_H
