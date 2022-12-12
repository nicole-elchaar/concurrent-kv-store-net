// Create a testing class that tests kvserver and kvclient funcionality
// Ensure that the database is consistent after each test
// Override the main method in kvserver and kvclient
// Use simple

#ifndef TEST_H
#define TEST_H

#include "message.cc"
#include "kvserver.cc"
#include "kvclient.cc"
#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <fstream>
#include <random>
#include <cstdlib>


// #include <boost/asio.hpp>
// #include <boost/thread.hpp>
// #include <boost/bind.hpp>

using namespace std;
using boost::asio::ip::tcp;

#define NASSERT_1(condition) {          PRINT_NASSERT(condition, "ASSERT FAILED") }
#define NASSERT_2(condition, message) { PRINT_NASSERT(condition, message) }
#define GET_MACRO(_1,_2,NAME,...) NAME
#define NASSERT(...) GET_MACRO(__VA_ARGS__, NASSERT_2, NASSERT_1)(__VA_ARGS__)
#define PRINT_NASSERT(condition, message) { if (!(condition)) { std::cerr << "\033[1;31m[FAIL]\033[0m\t" << message << "\n" << #condition << " @ " << __FILE__ << " (" << __LINE__ << ")" << std::endl; exit(128+SIGABRT); } }
// #define NASSERT(condition) { if(!(condition)){ std::cerr << "ASSERT FAILED: " << #condition << " @ " << __FILE__ << " (" << __LINE__ << ")" << std::endl; } }

class Test {
private:
  kvserver server;
  vector<kvclient> clients;
  std::vector<boost::shared_ptr<boost::thread>> server_threads;
  unordered_map<string, string> store;  // Map because some keys are duplicated
  const int NUM_ITERS = 1000;

  void SetUp() {
    // Import all data from users.txt into the server's kvstore and verify size
    ifstream users_file("users.txt");
    string line;
    while (getline(users_file, line)) {
      // Split the line into a key and value
      string key = line.substr(0, line.find(" "));
      string value = line.substr(line.find(" ") + 1);
      value[value.length() - 1] = '\0';   // Null terminate the value

      // Insert the key and value into the server's kvstore and local store
      server.store.store[key] = value;
      store[key] = value;
    }
    NASSERT(server.store.size() == 99728);
    NASSERT(store.size() == 99728);
  }

  void TearDown() {
    // Clear the server's kvstore
    server.store.clear();
    NASSERT(server.store.size() == 0);

    // Clear the keys and values accessible in Test
    store.clear();
  }

  void test_message_format(int num_iterations = 1000) {
    // Test that the message format is correct
    // Test that the message is correctly parsed

  }

  void test_concurrency(int num_iterations = 1000) {
    // Test that the server can handle multiple clients at once
    // Test that the server can handle multiple requests at once
    // Test that the server can handle multiple clients and requests at once
  }

  // Test GET for a num_iterations keys in the database on all clients
  bool test_get(int num_iterations = 1000) {
    try {
      for (int i = 0; i < num_iterations; i++) {
        // Test get functionality on existing key in the database
        auto it = next(begin(store), rand() % store.size());
        const string key = it->first;
        const string value = it->second;

        // Get the value from the server
        string server_value;
        server.store.get(key, server_value);
        NASSERT(
            strcmp(server_value.c_str(), value.c_str()) == 0,
            "TEST GET: Server value does not match value in database");

        for (int j = 0; j < clients.size(); j++) {
          // Get the value from the client
          string client_value;
          NASSERT(clients[j].get(key, client_value));
          
          // Ensure that the client's value is the same as the server's value
          NASSERT(
              strcmp(client_value.c_str(), server_value.c_str()) == 0,
              "TEST GET: Client value does not match server value");
        }
      }
    } catch (exception& e) {
      cerr << e.what() << endl;
      return false;
    }
    return true;
  }

  // Test PUT for a num_iterations keys in the database on all clients
  bool test_put(int num_iterations = 1000) {
    try {
      for (int i = 0; i < num_iterations; i++) {
        // Test put functionality on existing key in the database
        auto it = next(begin(store), rand() % store.size());
        const string key = it->first;
        const string value = it->second;

        // Put the value into the server
        server.store.put(key, value);
        NASSERT(
            server.store.size() == store.size(),
            "TEST PUT: Server size does not match size in database");

        // Get the value from the server
        string server_value;
        // Verify with iterator
        it = server.store.store.find(key);
        NASSERT(
            it != server.store.store.end(),
            "TEST PUT: Key not found in server store");
        server_value = it->second;        
        // server.store.get(key, server_value);
        NASSERT(
            strcmp(server_value.c_str(), value.c_str()) == 0,
            "TEST PUT: Server value does not match value in database");
        
        // Set the value to a different random string of the same length
        string new_value = value;
        for (int j = 0; j < new_value.length(); j++) {
          new_value[j] = rand() % 26 + 'a';
        }

        for (int j = 0; j < clients.size(); j++) {
          // Put the value into the client
          NASSERT(clients[j].put(key, new_value), "TEST PUT: Client put failed");

          // Verify the value was changed on the server
          it = server.store.store.find(key);
          NASSERT(
              it != server.store.store.end(),
              "TEST PUT: Key not found in server store");
          server_value = it->second;
          NASSERT(
              strcmp(server_value.c_str(), new_value.c_str()) == 0,
              "TEST PUT: Server value does not match new value");
          NASSERT(
              server.store.size() == store.size(),
              "TEST PUT: Server size does not match size in database");
          NASSERT(
              strcmp(server_value.c_str(), value.c_str()) != 0,
              "TEST PUT: Server value matches old value");
        }
      }
    } catch (exception& e) {
      cerr << e.what() << endl;
      return false;
    }
    return true;
  }

  // Test DEL for a num_iterations keys in the database on all clients
  bool test_del(int num_iterations = 1000) {
    try {
      for (int i = 0; i < num_iterations; i++) {
        // Test del functionality on existing key in the database
        auto it = next(begin(store), rand() % store.size());
        const string key = it->first;
        const string value = it->second;

        // Verify that the key is in the server's kvstore
        it = server.store.store.find(key);
        NASSERT(
            it != server.store.store.end(),
            "TEST DEL: Server value does not exist");
        string server_value = it->second;
        NASSERT(
            strcmp(server_value.c_str(), value.c_str()) == 0,
            "TEST DEL: Server value does not match value in database");

        // Select a random client to send the DEL request
        int client_index = rand() % clients.size();

        // Delete the key from the server
        clients[client_index].del(key);

        // Ensure that the key is no longer in the server's kvstore
        NASSERT(
            server.store.store.find(key) == server.store.store.end(),
            "TEST DEL: Server value still exists");
        
        // Remove the key from the local database
        store.erase(key);
      }
    } catch (exception& e) {
      cerr << e.what() << endl;
      return false;
    }
    return true;
  }

  // Stress test GET operations with many concurrent clients
  bool test_stress_get(int num_iterations = 1000) {
    // Create a lambda to randomly add get requests to the passed in client
    auto add_get_requests = [&](kvclient& client) {
      for (int i = 0; i < num_iterations; i++) {
        // Randomly select a key from the database or generate a random key
        if (rand() % 2 == 0) {
          // Get a random key already in the database and verify OK and value
          auto it = next(begin(store), rand() % store.size());
          string key = it->first;
          string value = it->second;
          string client_value = "";
          NASSERT(
              client.get(key, client_value),
              "TEST STRESS GET: Client get existing key failed");
          NASSERT(
              strcmp(client_value.c_str(), value.c_str()) == 0,
              "TEST STRESS GET: Client value does not match value in database");
        } else {
          // Generate a random key that is not in the database and verify ERROR
          string key = "";
          do {
            key = "";
            for (int j = 0; j < rand() % 10 + 1; j++) {
              key += rand() % 26 + 'a';
            }
          } while (store.find(key) != store.end());
          string value = "";

          // Verify that the key is not in the server's kvstore
          NASSERT(
              !client.get(key, value),
              "TEST STRESS GET: Client get missing key failed");
          NASSERT(
              strcmp(value.c_str(), "") == 0,
              "TEST STRESS GET: Client value is not empty");
        }
      }
    };

    // Create a list of threads to run the lambda on each client
    vector<thread> threads;
    for (int i = 0; i < clients.size(); i++) {
      threads.push_back(thread(add_get_requests, ref(clients[i])));
    }

    // Join all the threads
    for (int i = 0; i < threads.size(); i++) {
      threads[i].join();
    }

    return true;
  }

  // Stress test PUT with many concurrent clients
  bool test_stress_put(int num_iterations = 1000) {
    // Create a lambda to randomly add get requests to the passed in client
    auto add_put_requests = [&](kvclient& client) {
      cout << "Starting client" << endl;
      for (int i = 0; i < num_iterations; i++) {
        cout << "Iteration " << i << endl;
        // Randomly select a key from the database or generate a random key
        if (rand() % 2 == 0) {
          // Get a random key already in the database and verify OK and value
          auto it = next(begin(store), rand() % store.size());
          string key = it->first;
          string value = "";
          NASSERT(
              client.put(key, value),
              "TEST STRESS PUT: Client put existing key failed");
          store[key] = value;
        } else {
          // Generate a random key that is not in the database and verify ERROR
          string key = "";
          do {
            key = "";
            for (int j = 0; j < rand() % 10 + 1; j++) {
              key += rand() % 26 + 'a';
            }
          } while (store.find(key) != store.end());
          string value = "";

          // Verify that the key is not in the server's kvstore
          NASSERT(
              client.put(key, value),
              "TEST STRESS PUT: Client put missing key failed");
          store[key] = value;
        }
      }
      cout << "Finished client" << endl;
    };

    // Create a list of threads to run the lambda on each client
    vector<thread> threads;
    for (int i = 0; i < clients.size(); i++) {
      threads.push_back(thread(add_put_requests, ref(clients[i])));
    }

    // Join all the threads
    for (int i = 0; i < threads.size(); i++) {
      threads[i].join();
    }

    // Verify that the server's kvstore matches the local database
    for (auto it = store.begin(); it != store.end(); it++) {
      string key = it->first;
      string value = it->second;
      auto server_it = server.store.store.find(key);
      NASSERT(
          server_it != server.store.store.end(),
          "TEST STRESS PUT: Server value does not exist");
      string server_value = server_it->second;
      NASSERT(
          strcmp(server_value.c_str(), value.c_str()) == 0,
          "TEST STRESS PUT: Server value does not match value in database");
    }

    return true;
    // // Create a lambda to randomly add put requests to the passed in client
    // auto add_put_requests = [&](kvclient& client) {
    //   string value = "";
    //   for (int i = 0; i < num_iterations; i++) {
    //     // Randomly generate a key or use an existing key
    //     if (rand() % 2 == 0) {
    //       // Get a random key from the database
    //       auto it = next(begin(store), rand() % store.size());
    //       string key = it->first;
    //       NASSERT(
    //           client.put(key, value),
    //           "TEST STRESS PUT: Client put existing key failed");
    //       store[key] = "";
    //     } else {
    //       // Generate a random key
    //       string key = "";
    //       do {
    //         key = "";
    //         for (int j = 0; j < rand() % 10 + 1; j++) {
    //           key += rand() % 26 + 'a';
    //         }
    //       } while (store.find(key) != store.end());

    //       // Verify that the key is not in the server's kvstore
    //       NASSERT(
    //           client.put(key, value),
    //           "TEST STRESS PUT: Client put missing key failed");
    //       store[key] = "";
    //     }
    //   }
    // };

    // // Create a list of threads to run the lambda on each client
    // vector<thread> threads;
    // for (int i = 0; i < clients.size(); i++) {
    //   threads.push_back(thread(add_put_requests, ref(clients[i])));
    // }

    // // Join all the threads
    // for (int i = 0; i < threads.size(); i++) {
    //   threads[i].join();
    // }

    // // Verify that each key is in the server's kvstore
    // for (auto it = begin(store); it != end(store); it++) {
    //   string key = it->first;
    //   NASSERT(
    //       server.store.store.find(key) != server.store.store.end(),
    //       "TEST STRESS PUT: Server value does not match value in database");
    // }
    
    // // Verify that the size of the kvstore matches local
    // NASSERT(
    //     server.store.store.size() == store.size(),
    //     "TEST STRESS PUT: Server size does not match size in database");
    
    // return true;



    // Create a lambda to randomly add put requests to the passed in client

    // // Create a lambda to run random operations from the passed in client
    // auto run_client = [&](Client& client) {
    //   for (int i = 0; i < num_iterations; i++) {
    //     // Select a random operation
    //     int op = rand() % 3;
    //     switch (op) {
    //       case 0: {
    //         // Test GET
    //         auto it = next(begin(store), rand() % store.size());
    //         const string key = it->first;
    //         const string value = it->second;
    //         string server_value;
    //         client.get(key, server_value);
    //         NASSERT(
    //             strcmp(server_value.c_str(), value.c_str()) == 0,
    //             "TEST STRESS: Server value does not match value in database");
    //         break;
    //       }
    //       case 1: {
    //         // Test PUT
    //         auto it = next(begin(store), rand() % store.size());
    //         const string key = it->first;
    //         const string value = it->second;
    //         string new_value = value;
    //         for (int j = 0; j < new_value.length(); j++) {
    //           new_value[j] = rand() % 26 + 'a';
    //         }
    //         client.put(key, new_value);
    //         break;
    //       }
    //       case 2: {
    //         // Test DEL
    //         auto it = next(begin(store), rand() % store.size());
    //         const string key = it->first;
    //         client.del(key);
    //         break;
    //       }
    //     }
    //   }
    // }; 


    // try {
    //   // Create a vector of threads to run the clients
    //   vector<thread> threads;
    //   for (int i = 0; i < clients.size(); i++) {
    //     threads.push_back(thread(&Client::run, &clients[i], num_iterations));
    //   }

    //   // Wait for all threads to finish
    //   for (int i = 0; i < threads.size(); i++) {
    //     threads[i].join();
    //   }
    // } catch (exception& e) {
    //   cerr << e.what() << endl;
    //   return false;
    // }
    // return true;
  }


  // Wrap a supplied function
  bool test_wrapper(string name, bool (Test::*func)(int)) {
    SetUp();
    auto result = (this->*func)(NUM_ITERS);
    TearDown();

    if (result) {
      cout << "\033[1;32m[PASS]\033[0m" << "\t" << name << endl;
      return true;
    }

    cout << "\033[1;31m[FAIL]\033[0m" << "\t" << name << endl;
    return false;
  }

  // bool test_wrapper(string& name, bool (Test::*func)(int)) {
  //   // bool test_wrapper(string& name, bool (Test::*func)(int)) {
  //   SetUp();
  //   auto result = this->(*func)(NUM_ITERS);
  //   TearDown();

  //   if (result) {
  //     cout << "\033[1;32m[PASS]\033[0m" << "\t" << name << endl;
  //     return true;
  //   }
  //   // Print [FAIL] name in red and return false
  //   cout << "\033[1;31m[FAIL]\033[0m" << "\t" << name << endl;
  //   return false;
  // }

public:
  Test(
      int num_clients,
      boost::asio::io_service& server_io_service,
      boost::asio::io_service& client_io_service,
      const std::string& host,
      int server_port,
      const std::string& client_port) : server(server_io_service, server_port) {
    // Create a single server and num_clients clients
    for (int i = 0; i < num_clients; i++) {
      clients.push_back(kvclient(client_io_service, host, client_port));
    }

    // Add threads to the server_io_service
    for (int i = 0; i < thread::hardware_concurrency(); i++) {
      boost::shared_ptr<boost::thread> thread(new boost::thread(
          boost::bind(&boost::asio::io_service::run, &server_io_service)));
      server_threads.push_back(thread);
      // server_threads.create_thread(
      //     boost::bind(&boost::asio::io_service::run, &server_io_service));
    }

    SetUp();
  }

  ~Test() {
    TearDown();

    for (int i = 0; i < server_threads.size(); i++) {
      server_threads[i]->join();
    }
  }

  void run() {
    cout << "Running tests..." << endl;

    // Run all tests
    test_wrapper(std::move("TEST_GET"), &Test::test_get);
    test_wrapper(std::move("TEST_PUT"), &Test::test_put);
    test_wrapper(std::move("TEST_DELETE"), &Test::test_del);
    test_wrapper(std::move("TEST_STRESS_GET"), &Test::test_stress_get);
    test_wrapper(std::move("TEST_STRESS_PUT"), &Test::test_stress_put);
    // test_put();
    // test_delete();
    // test_random();
    // test_stress();

    cout << "All tests passed!" << endl;
  }

};


int main(int argc, char* argv[]) {
  // Check for correct number of arguments
  if (argc != 3) {
    std::cerr << "Usage: test <host> <port>\n";
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