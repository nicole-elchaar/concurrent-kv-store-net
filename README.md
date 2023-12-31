# Concurrent KV Store Net 🗝️🗄️🌐

This project implements a **highly-concurrent**, **highly-available** **key-value storage system** that accepts requests over the network.  As key-value stores play a significant role in modern distributed systems, this simplified version serves as a useful exercise in concurrent programming and a starting point for more sophisticated systems.

Concurrent KV Store Net consists of two main components: a server and a client. The server listens for incoming requests from clients and handles them accordingly. The client makes requests to the server to access the key-value store.  

## Features 🌟

- Concurrent operation
  - Uses a thread pool to serve as many requests as there are threads available
  - Lock-free read operations allow simultaneous reads on the same data
  - Locking write operations ensure correctness when modifying data
- Generic storage
  - Value types are arbitrary as they are stored in byte-array format 
- Custom communication protocol
  - Requests and responses are sent over the network using a custom protocol similar to HTTP
  - Variable-length keys and content are supported
- Correctness
  - Concurrent operations are stress tested with a custom test suite

## Usage 🧭

To build the project, run `make`.  Optionally set parameter `BUILD` to `release` to build with optimizations:

```shell
make BUILD=release
```

To test the key-value store, run the following command after building, replacing `$(BUILD)` with either `debug` or `release`:

```shell
./bin/$(BUILD)/test <host> <port>
```

For example:

```shell
./bin/debug/test localhost 1895
```

By default, the program will run with four clients and one server.  We initialize global hash map with 10,000 key-value pairs from the *users.txt* file.

## Dependencies 🧩

- Make
- boost

## Contributions 🤝

Contributions and comments are welcome!  Please fork the repository and submit a pull request or add an issue.
