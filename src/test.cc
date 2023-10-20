/*
 * Nicole ElChaar, CSE 411, Fall 2022
 *
 * The Test class tests the functionality and correctness of the kvserver and
 * kvclient by sending messages to the server and verifying that the server's
 * kvstore is consistent after each test.  A mock database verifies the result.
 */

#ifndef TEST_H
#define TEST_H

#include "kvclient.cc"
#include "kvserver.cc"
#include "message.hpp"

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

using boost::asio::ip::tcp;

// Define custom NASSERT macro to print the file and line number of the error
#define NASSERT_1(condition) \
  { PRINT_NASSERT(condition, "ASSERT FAILED") }
#define NASSERT_2(condition, message) \
  { PRINT_NASSERT(condition, message) }
#define GET_MACRO(_1, _2, NAME, ...) NAME
#define NASSERT(...) GET_MACRO(__VA_ARGS__, NASSERT_2, NASSERT_1)(__VA_ARGS__)
#define PRINT_NASSERT(condition, message)                                     \
  {                                                                           \
    if (!(condition)) {                                                       \
      std::cerr << "\033[1;31m[FAIL]\033[0m\t" << message << "\n"             \
                << #condition << " @ " << __FILE__ << " (" << __LINE__ << ")" \
                << std::endl;                                                 \
      exit(128 + SIGABRT);                                                    \
    }                                                                         \
  }

class Test {
private:
  kvserver server;
  std::vector<kvclient> clients;
  std::vector<boost::shared_ptr<boost::thread>> server_threads;
  std::unordered_map<std::string, std::string> store; // Avoid duplicates
  const static int NUM_ITERS = 1000; // Number of iterations for each test

  void SetUp() {
    // Import all data from users.txt into the server's kvstore and verify size
    std::ifstream users_file("src/users.txt");

    // If the users file is empty, does not exist, or is not open, end the test
    NASSERT(users_file.good(), "SETUP: Unable to read users.txt");
    NASSERT(users_file.peek() != std::ifstream::traits_type::eof(),
            "SETUP: users.txt is empty");
 
    std::string line;
    while (getline(users_file, line)) {
      // Split the line into a key and value
      std::string key = line.substr(0, line.find(" "));
      std::string value = line.substr(line.find(" ") + 1);
      value[value.length() - 1] = '\0'; // Null terminate the value

      // Insert the key and value into the server's kvstore and local store
      server.store.store[key] = value;
      store[key] = value;
    }

    // Verify that the server's kvstore and local store have the same size
    NASSERT(server.store.size() == store.size(),
            "SETUP: Unexpected server size");
    NASSERT(server.store.size() == 99728,
            "SETUP: Unexpected length of users.txt");
  }

  void TearDown() {
    // Clear the server's kvstore
    server.store.clear();
    NASSERT(server.store.size() == 0);

    // Clear the keys and values accessible in Test
    store.clear();
    NASSERT(store.size() == 0);
  }

  bool test_message(int num_iterations = NUM_ITERS) {
    // Test that the message format is correct
    message m(GET, "key");
    NASSERT(m.get_type() == GET);
    NASSERT(m.get_key() == "key");
    NASSERT(m.get_value() == "");

    m = message(PUT, "key", "value");
    NASSERT(m.get_type() == PUT);
    NASSERT(m.get_key() == "key");
    NASSERT(m.get_value() == "value");

    m = message(DEL, "key");
    NASSERT(m.get_type() == DEL);
    NASSERT(m.get_key() == "key");
    NASSERT(m.get_value() == "");

    m = message();
    NASSERT(m.get_type() == UNSET);
    NASSERT(m.get_key() == "");
    NASSERT(m.get_value() == "");

    // Test that the message is correctly parsed
    std::string msg = "GET key";
    m = message(msg);
    NASSERT(m.get_type() == GET);
    NASSERT(m.get_key() == "key");
    NASSERT(m.get_value() == "");

    msg = "PUT key value";
    m = message(msg);
    NASSERT(m.get_type() == PUT);
    NASSERT(m.get_key() == "key");
    NASSERT(m.get_value() == "value");

    msg = "DEL key";
    m = message(msg);
    NASSERT(m.get_type() == DEL);
    NASSERT(m.get_key() == "key");
    NASSERT(m.get_value() == "");

    msg = "UNPARSABLE";
    m = message(msg);
    NASSERT(m.get_type() == UNSET);
    NASSERT(m.get_key() == "");
    NASSERT(m.get_value() == "");
    return true;
  }

  // Test GET for a num_iterations keys in the database on all clients
  bool test_get(int num_iterations = NUM_ITERS) {
    try {
      for (int i = 0; i < num_iterations; i++) {
        // Test get functionality on existing key in the database
        auto it = next(begin(store), rand() % store.size());
        const std::string key = it->first;
        const std::string value = it->second;

        // Get the value from the server
        std::string server_value;
        server.store.get(key, server_value);
        NASSERT(strcmp(server_value.c_str(), value.c_str()) == 0,
                "TEST GET: Server value does not match value in database");

        for (size_t j = 0; j < clients.size(); j++) {
          // Get the value from the client
          std::string client_value;
          NASSERT(clients[j].get(key, client_value));

          // Ensure that the client's value is the same as the server's value
          NASSERT(strcmp(client_value.c_str(), server_value.c_str()) == 0,
                  "TEST GET: Client value does not match server value");
        }
      }
    } catch (std::exception &e) {
      std::cerr << e.what() << std::endl;
      return false;
    }
    return true;
  }

  // Test PUT for a num_iterations keys in the database on all clients
  bool test_put(int num_iterations = NUM_ITERS) {
    try {
      for (int i = 0; i < num_iterations; i++) {
        // Test put functionality on existing key in the database
        auto it = next(begin(store), rand() % store.size());
        const std::string key = it->first;
        const std::string value = it->second;

        // Put the value into the server
        server.store.put(key, value);
        NASSERT(server.store.size() == store.size(),
                "TEST PUT: Server size does not match database size");

        // Get the value from the server
        std::string server_value;
        // Verify with iterator
        it = server.store.store.find(key);
        NASSERT(it != server.store.store.end(),
                "TEST PUT: Key not found in server store");
        server_value = it->second;
        // server.store.get(key, server_value);
        NASSERT(strcmp(server_value.c_str(), value.c_str()) == 0,
                "TEST PUT: Server value does not match value in database");

        // Set the value to a different random string of the same length
        std::string new_value = value;
        for (size_t j = 0; j < new_value.length(); j++) {
          new_value[j] = rand() % 26 + 'a';
        }

        for (size_t j = 0; j < clients.size(); j++) {
          // Put the value into the client
          NASSERT(clients[j].put(key, new_value),
                  "TEST PUT: Client put failed");

          // Verify the value was changed on the server
          it = server.store.store.find(key);
          NASSERT(it != server.store.store.end(),
                  "TEST PUT: Key not found in server store");
          server_value = it->second;
          NASSERT(strcmp(server_value.c_str(), new_value.c_str()) == 0,
                  "TEST PUT: Server value does not match updated value");
          NASSERT(server.store.size() == store.size(),
                  "TEST PUT: Server size does not match database size");
        }
      }
    } catch (std::exception &e) {
      std::cerr << e.what() << std::endl;
      return false;
    }
    return true;
  }

  // Test DEL for a num_iterations keys in the database on all clients
  bool test_del(int num_iterations = NUM_ITERS) {
    std::mutex del_mutex;
    try {
      for (int i = 0; i < num_iterations; i++) {
        // Test del functionality on existing key in the database
        auto it = next(begin(store), rand() % store.size());
        const std::string key = it->first;
        const std::string value = it->second;

        // Verify that the key is in the server's kvstore
        it = server.store.store.find(key);
        NASSERT(it != server.store.store.end(),
                "TEST DEL: Key does not exist on server");
        std::string server_value = it->second;
        NASSERT(strcmp(server_value.c_str(), value.c_str()) == 0,
                "TEST DEL: Server value does not match value in database");

        // Select a random client to send the DEL request
        int client_index = rand() % clients.size();

        // Delete the key from the server
        clients[client_index].del(key);

        // Ensure that the key is no longer in the server's kvstore
        NASSERT(server.store.store.find(key) == server.store.store.end(),
                "TEST DEL: Server value still exists after delete");

        // Remove the key from the local database
        del_mutex.lock();
        store.erase(key);
        del_mutex.unlock();
      }
    } catch (std::exception &e) {
      std::cerr << e.what() << std::endl;
      return false;
    }
    return true;
  }

  // Stress test GET operations with many concurrent clients
  bool test_stress_get(int num_iterations = NUM_ITERS) {
    // Create a lambda to randomly add get requests to the passed in client
    auto add_get_requests = [&](kvclient &client) {
      for (int i = 0; i < num_iterations; i++) {
        // Randomly select a key from the database or generate a random key
        if (rand() % 2 == 0) {
          // Get a random key already in the database and verify OK and value
          auto it = next(begin(store), rand() % store.size());
          std::string key = it->first;
          std::string value = it->second;
          std::string client_value = "";
          NASSERT(client.get(key, client_value),
                  "TEST STRESS GET: Client get existing key failed");
          NASSERT(
              strcmp(client_value.c_str(), value.c_str()) == 0,
              "TEST STRESS GET: Client value does not match value in database");
        } else {
          // Generate a random key that is not in the database and verify ERROR
          std::string key = "";
          do {
            key = "";
            for (int j = 0; j < rand() % 10 + 1; j++) {
              key += rand() % 26 + 'a';
            }
          } while (store.find(key) != store.end());
          std::string value = "";

          // Verify that the key is not in the server's kvstore
          NASSERT(!client.get(key, value),
                  "TEST STRESS GET: Client found non-existent key");
          NASSERT(strcmp(value.c_str(), "") == 0,
                  "TEST STRESS GET: Client value is not empty");
        }
      }
    };

    // Create a list of threads to run the lambda on each client
    std::vector<std::thread> threads;
    for (size_t i = 0; i < clients.size(); i++) {
      threads.push_back(std::thread(add_get_requests, std::ref(clients[i])));
    }

    // Join all the threads
    for (size_t i = 0; i < threads.size(); i++) {
      threads[i].join();
    }

    return true;
  }

  // Stress test PUT with many concurrent clients
  bool test_stress_put(int num_iterations = NUM_ITERS) {
    // Create a lambda to randomly add get requests to the passed in client
    auto add_put_requests = [&](kvclient &client) {
      for (int i = 0; i < num_iterations; i++) {
        // Randomly select a key from the database or generate a random key
        if (rand() % 2 == 0) {
          // Get a random key in the database
          auto it = next(begin(store), rand() % store.size());
          std::string key = it->first;
          std::string value = it->second;
          std::string client_value = "";
          for (int j = 0; j < rand() % 10 + 1; j++) {
            client_value += rand() % 26 + 'a';
          }

          // Verify OK and new value
          NASSERT(client.put(key, client_value),
                  "TEST STRESS PUT: Client put existing key failed");
          it = server.store.store.find(key);
          NASSERT(it != server.store.store.end(),
                  "TEST STRESS PUT: Server value does not exist");
          NASSERT(strcmp(client_value.c_str(), it->second.c_str()) == 0,
                  "TEST STRESS PUT: Inserted value does not match value in "
                  "database");
        } else {
          // Generate a random key that is not in the database
          std::string key = "";
          do {
            key = "";
            for (int j = 0; j < rand() % 10 + 1; j++) {
              key += rand() % 26 + 'a';
            }
          } while (store.find(key) != store.end());
          // input j jb n
          std::string value = "";
          for (int j = 0; j < rand() % 10 + 1; j++) {
            value += rand() % 26 + 'a';
          }

          // Verify OK and new value
          NASSERT(client.put(key, value),
                  "TEST STRESS PUT: Client received error on put");
          auto it = server.store.store.find(key);
          NASSERT(it != server.store.store.end(),
                  "TEST STRESS PUT: Server failed to put new key");
        }
      }
    };

    // Create a list of threads to run the lambda on each client
    std::vector<std::thread> threads;
    for (size_t i = 0; i < clients.size(); i++) {
      threads.push_back(std::thread(add_put_requests, std::ref(clients[i])));
    }

    // Join all the threads
    for (size_t i = 0; i < threads.size(); i++) {
      threads[i].join();
    }

    return true;
  }

  // Stress test DEL with many concurrent clients
  bool test_stress_del(int num_iterations = NUM_ITERS) {
    size_t starting_size = store.size();
    std::atomic<size_t> num_del(0);
    std::mutex del_lock;
    // Create a lambda to randomly delete keys from the database
    auto add_del_requests = [&](kvclient &client) {
      for (int i = 0; i < num_iterations; i++) {
        // Randomly select a key from the database or generate a random key
        std::cout << "Iteration " << i << std::endl;
        if (rand() % 2 == 0) {
          // Get a random key already in the database and verify OK and value
          std::cout << "Deleting existing key" << std::endl;
          del_lock.lock();
          std::cout << "Lock acquired" << std::endl;
          auto it = next(begin(store), rand() % store.size());
          const std::string key = it->first;
          const std::string value = it->second;

          // Remove the key from the database
          store.erase(it);
          std::cout << "Key erased" << std::endl;
          del_lock.unlock();
          std::cout << "Lock released" << std::endl;
          num_del++;
          std::cout << "Num del incremented" << std::endl;

          std::string client_value = "";
          NASSERT(client.del(key),
                  "TEST STRESS DEL: Client del existing key failed");
        } else {
          // Generate a random key that is not in the database and verify ERROR
          std::cout << "Deleting missing key" << std::endl;
          std::string key = "";
          do {
            key = "";
            std::cout << "Generating key" << std::endl;
            for (int j = 0; j < rand() % 10 + 1; j++) {
              key += rand() % 26 + 'a';
            }
          } while (store.find(key) != store.end());
          std::string value = "";

          // Verify that the key is not in the server's kvstore
          NASSERT(!client.del(key),
                  "TEST STRESS DEL: Client del missing key failed");
          NASSERT(strcmp(value.c_str(), "") == 0,
                  "TEST STRESS DEL: Client value is not empty");
        }
      }
    };

    // Create a list of threads to run the lambda on each client
    std::vector<std::thread> threads;
    std::cout << "Creating threads" << std::endl;
    for (size_t i = 0; i < clients.size(); i++) {
      threads.push_back(std::thread(add_del_requests, std::ref(clients[i])));
    }

    // Join all the threads
    std::cout << "Joining threads" << std::endl;
    for (size_t i = 0; i < threads.size(); i++) {
      threads[i].join();
    }

    // Check the number of keys deleted compared to the size of the database
    std::cout << "Checking number of keys deleted" << std::endl;
    NASSERT(
        server.store.size() == starting_size - num_del,
        "TEST STRESS DEL: Number of keys deleted does not match database size");

    return true;
  }

  // Wrap a supplied function
  bool test_wrapper(const std::string name, bool (Test::*func)(int)) {
    SetUp();
    auto result = (this->*func)(NUM_ITERS);
    TearDown();

    if (result) {
      std::cout << "\033[1;32m[PASS]\033[0m"
                << "\t" << name << std::endl;
      return true;
    }

    std::cout << "\033[1;31m[FAIL]\033[0m"
              << "\t" << name << std::endl;
    return false;
  }

public:
  Test(int num_clients,
       boost::asio::io_service &server_io_service,
       boost::asio::io_service &client_io_service,
       const std::string &host,
       int server_port,
       const std::string &client_port)
      : server(server_io_service, server_port) {
    // Create a single server and num_clients clients
    for (int i = 0; i < num_clients; i++) {
      clients.push_back(kvclient(client_io_service, host, client_port));
    }

    // Add threads to the server_io_service
    for (size_t i = 0; i < boost::thread::hardware_concurrency(); i++) {
      boost::shared_ptr<boost::thread> thread(new boost::thread(
          boost::bind(&boost::asio::io_service::run, &server_io_service)));
      server_threads.push_back(thread);
    }
  }

  ~Test() {
    TearDown();

    for (size_t i = 0; i < server_threads.size(); i++) {
      server_threads[i]->join();
    }
  }

  void run() {
    std::cout << "Running tests..." << std::endl;

    // Run all tests
    test_wrapper(std::move("TEST_MESSAGE"), &Test::test_message);
    test_wrapper(std::move("TEST_GET"), &Test::test_get);
    test_wrapper(std::move("TEST_PUT"), &Test::test_put);
    test_wrapper(std::move("TEST_DELETE"), &Test::test_del);
    test_wrapper(std::move("TEST_STRESS_GET"), &Test::test_stress_get);
    test_wrapper(std::move("TEST_STRESS_PUT"), &Test::test_stress_put);
    test_wrapper(std::move("TEST_STRESS_DEL"), &Test::test_stress_del);

    std::cout << "All tests passed!" << std::endl;
  }
};

int main(int argc, char *argv[]) {
  // Check for correct number of arguments
  if (argc != 3) {
    std::cerr << "Usage: test <host> <port>" << std::endl;
    return 1;
  }

  boost::asio::io_service server_io_service;
  boost::asio::io_service client_io_service;

  // Create a test with 4 clients
  Test test(
      4, server_io_service, client_io_service, argv[1], atoi(argv[2]), argv[2]);

  test.run();

  client_io_service.stop();
  server_io_service.stop();

  return 0;
}

#endif
