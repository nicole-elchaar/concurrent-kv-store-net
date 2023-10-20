/*
 * Nicole ElChaar, CSE 411, Fall 2022
 *
 * The kvserver class handles GET, PUT, and DELETE requests from any number of
 * clients, using a thread pool to handle multiple requests at once. It returns
 * OK and ERROR responses depending on the success of the operation. The
 * kvserver uses the kvstore class to store the key-value pairs.
 */

#ifndef KVSERVER_H
#define KVSERVER_H

#include "kvstore.hpp"
#include "message.hpp"
#include "test.cc"

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread.hpp>

using boost::asio::ip::tcp;

class kvserver {
private:
  friend class Test;
  boost::asio::io_service &io_service;
  tcp::acceptor acceptor;
  kvstore store;
  std::vector<message> message_queue;

  void start_accept() {
    tcp::socket *socket = new tcp::socket(io_service);
    acceptor.async_accept(*socket,
                          boost::bind(&kvserver::handle_accept,
                                      this,
                                      socket,
                                      boost::asio::placeholders::error));
  }

  void handle_accept(tcp::socket *socket,
                     const boost::system::error_code &error) {
    if (!error) {
      boost::thread t(boost::bind(&kvserver::handle_session, this, socket));
    } else {
      delete socket;
    }
    start_accept();
  }

  void handle_session(tcp::socket *socket) {
    try {
      for (;;) {
        boost::asio::streambuf request;
        boost::asio::read_until(*socket, request, "\n");

        // Copy into string
        std::istream request_stream(&request);
        std::string request_string;
        std::getline(request_stream, request_string);

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
          if (store.put(msg.get_key(), msg.get_value())) {
            message resp(OK);
            boost::asio::write(*socket, boost::asio::buffer(resp.to_string()));
          } else {
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
    } catch (std::exception &e) {
      // std::cout << "Client disconnected" << std::endl;
    }
    delete socket;
  }

public:
  kvserver(boost::asio::io_service &io_service, short port)
      : io_service(io_service),
        acceptor(io_service, tcp::endpoint(tcp::v4(), port)) {
    start_accept();
  }
};

#endif