### Summary

This simple client-server application demonstrates the basic usage of the Wired library, including connecting, sending messages, and handling responses. It provides an easy-to-understand example of how to use the primary interface calls and possibilities of the Wired library.

## Overview

- **Client**: The client connects to the server, sends ping messages, and handles responses from the server.
- **Server**: The server listens for incoming connections, receives ping messages from the client, and responds with ping messages.

## Building and Running the Application

### Steps to Build

1. **Create a build directory and navigate to it**:

   ```sh
   mkdir build
   cd build
   ```

2. **Configure the project with CMake**:

   ```sh
   cmake ..
   ```

3. **Build the project**:

   ```sh
   cmake --build . 
   ```

### Steps to Run

#### Start the Server

1. **Navigate to the build directory**:

   ```sh
   cd build
   ```

2. **Run the server executable**:

   ```sh
   ./server
   ```

#### Start the Client

1. **Open a new terminal and navigate to the build directory**:

   ```sh
   cd build
   ```

2. **Run the client executable**:

   ```sh
   ./client
   ```

## Explanation of `server.cpp` and `client.cpp`

### `server.cpp`

The `server.cpp` file creates a server object that inherits from `wired::server_interface<common_messages>`. It implements the `on_message` method to handle incoming messages from the client. When the server receives a ping message from the client, it responds with a ping message back to the client.

### `client.cpp`

The `client.cpp` file creates a client object that inherits from `wired::client_interface<common_messages>`. It implements the `on_message` method to handle incoming messages from the server. The client sends ping messages to the server and handles the responses. After receiving three ping responses from the server, the client disconnects.

