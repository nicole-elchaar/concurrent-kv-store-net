// Create a boost asio server that handles GET, PUT, and DELETE requests from
// the client and sends OK and ERROR responses

// Modify the code to use multithreading to handle multiple clients at once

#ifndef KVSERVER_H
#define KVSERVER_H

#include <iostream>
#include <string>
#include <cstdlib>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "kvstore.cc"
#include "message.cc"
#include "test.cc"

using boost::asio::ip::tcp;

class kvserver {
private:
  friend class Test;
  boost::asio::io_service& io_service;
  tcp::acceptor acceptor;
  kvstore store;
  vector<message> message_queue;

  void start_accept() {
    tcp::socket* socket = new tcp::socket(io_service);
    acceptor.async_accept(
        *socket,
        boost::bind(&kvserver::handle_accept,
            this, socket, boost::asio::placeholders::error));
  }

  void handle_accept(tcp::socket* socket, const boost::system::error_code& error) {
    if (!error) {
      boost::thread t(boost::bind(&kvserver::handle_session, this, socket));
    } else {
      delete socket;
    }
    start_accept();
  }

  // void handle_request(std::string& request_string) {
  //   // Parse message
  //   message msg(request_string);

  //   // Handle message
  //   if (msg.get_type() == GET) {
  //     std::string value;
  //     if (store.get(msg.get_key(), value)) {
  //       message resp(OK, value);
  //       boost::asio::write(*socket, boost::asio::buffer(resp.to_string()));
  //     } else {
  //       message resp(ERROR);
  //       boost::asio::write(*socket, boost::asio::buffer(resp.to_string()));
  //     }
  //   } else if (msg.get_type() == PUT) {
  //     if (store.put(msg.get_key(), msg.get_value())) {
  //       message resp(OK);
  //       boost::asio::write(*socket, boost::asio::buffer(resp.to_string()));
  //     } else {
  //       message resp(ERROR);
  //       boost::asio::write(*socket, boost::asio::buffer(resp.to_string()));
  //     }
  //   } else if (msg.get_type() == DEL) {
  //     if (store.del(msg.get_key())) {
  //       message resp(OK);
  //       boost::asio::write(*socket, boost::asio::buffer(resp.to_string()));
  //     } else {
  //       message resp(ERROR);
  //       boost::asio::write(*socket, boost::asio::buffer(resp.to_string()));
  //     }
  //   } else {
  //     message resp(ERROR);
  //     boost::asio::write(*socket, boost::asio::buffer(resp.to_string()));
  //   }
  // }

  void handle_session(tcp::socket* socket) {
    try {
      for (;;) {
        // cout << "Waiting for message" << endl;
        // Read in message from client
        boost::asio::streambuf request;
        boost::asio::read_until(*socket, request, "\n");
        // cout << "Message received" << endl;

        // Copy into string
        std::istream request_stream(&request);
        std::string request_string;
        std::getline(request_stream, request_string);

        // handle_request(request_string);

        // Parse message
        message msg(request_string);

        // Handle message
        if (msg.get_type() == GET) {
          std::string value;
          if (store.get(msg.get_key(), value)) {
            message resp(OK, value);
            boost::asio::write(*socket, boost::asio::buffer(resp.to_string()));
          } else {
            message resp(ERROR);
            boost::asio::write(*socket, boost::asio::buffer(resp.to_string()));
          }
        } else if (msg.get_type() == PUT) {
          cout << "PUT" << endl;
          if (store.put(msg.get_key(), msg.get_value())) {
            cout << "PUT OK" << endl;
            message resp(OK);
            boost::asio::write(*socket, boost::asio::buffer(resp.to_string()));
          } else {
            cout << "PUT ERROR" << endl;
            message resp(ERROR);
            boost::asio::write(*socket, boost::asio::buffer(resp.to_string()));
          }
        } else if (msg.get_type() == DEL) {
          if (store.del(msg.get_key())) {
            message resp(OK);
            boost::asio::write(*socket, boost::asio::buffer(resp.to_string()));
          } else {
            message resp(ERROR);
            boost::asio::write(*socket, boost::asio::buffer(resp.to_string()));
          }
        } else {
          message resp(ERROR);
          boost::asio::write(*socket, boost::asio::buffer(resp.to_string()));
        }
      }
    } catch (std::exception& e) {
      // std::cout << "Client disconnected" << std::endl;
      // std::cerr << e.what() << std::endl;
      // std::cerr << "Client disconnected" << std::endl;
    }
    delete socket;
  }

public:
  kvserver(boost::asio::io_service& io_service, short port) :
      io_service(io_service),
      acceptor(io_service, tcp::endpoint(tcp::v4(), port)) {
    start_accept();
  }
};

// int main(int argc, char* argv[]) {
//   try {
//     if (argc != 2) {
//       std::cerr << "Usage: kvserver <port>" << std::endl;
//       return 1;
//     }

//     boost::asio::io_service io_service;
//     kvserver server(io_service, std::atoi(argv[1]));

//     boost::thread_group threads;
//     // Use up to hardware concurrency threads
//     for (int i = 0; i < thread::hardware_concurrency(); ++i) {
//       threads.create_thread(boost::bind(&boost::asio::io_service::run, &io_service));
//     }
//     threads.join_all();
//     // io_service.run();
//   } catch (std::exception& e) {
//     std::cerr << e.what() << std::endl;
//   }

//   return 0;
// }

#endif