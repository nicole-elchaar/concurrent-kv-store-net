//
// kvserver.cc
// Key-value store that accepts requests to access a centralized data storage
// over the network
//
// Copyright (c) 2003-2021 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <algorithm>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <set>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include "message.hpp"

using boost::asio::ip::tcp;

typedef std::deque<chat_message> chat_message_queue;

class chat_session : public boost::enable_shared_from_this<chat_session> {
private:
  tcp::socket socket_;
  chat_message read_msg_;
  chat_message_queue write_msgs_;
public:
  chat_session(boost::asio::io_context& io_context) :
      socket_(io_context) {}

  tcp::socket& socket() {
    return socket_;
  }

  void start() {
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(read_msg_.data(), chat_message::header_length),
        boost::bind(
            &chat_session::handle_read_header,
            shared_from_this(),
            boost::asio::placeholders::error));
    std::cout << "Connected " << read_msg_.data() << std::endl;
  }

  void deliver(const chat_message& msg) {
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress) {
      boost::asio::async_write(
          socket_,
          boost::asio::buffer(
              write_msgs_.front().data(), write_msgs_.front().length()),
          boost::bind(
            &chat_session::handle_write,
            shared_from_this(),
            boost::asio::placeholders::error));
    }
    else {
      std::cout << "WRITE QUEUE??" << std::endl;
    }
  }

  void handle_read_header(const boost::system::error_code& error) {
    if (!error && read_msg_.decode_header()) {
      boost::asio::async_read(socket_,
      boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
      boost::bind(&chat_session::handle_read_body, shared_from_this(),
      boost::asio::placeholders::error));
      std::cout << "Read header: " << read_msg_.data() << std::endl;
    }
    else {
      std::cout << "Read header error: " << error.message() << std::endl;
    }
  }

  void handle_read_body(const boost::system::error_code& error) {
    if (!error) {
      deliver(read_msgs_);
      boost::asio::async_read(
          socket_,
          boost::asio::buffer(read_msg_.data(), chat_message::header_length),
          boost::bind(
              &chat_session::handle_read_header, 
              shared_from_this(),
              boost::asio::placeholders::error));
      std::cout << "Read body: " << read_msg_.data() << std::endl;
    }
    else {
      std::cout << "Read body error: " << error.message() << std::endl;
    }
  }

  void handle_write(const boost::system::error_code& error) {
    if (!error) {
      write_msgs_.pop_front();
      if (!write_msgs_.empty()) {
        boost::asio::async_write(
            socket_,
            boost::asio::buffer(
                write_msgs_.front().data(), write_msgs_.front().length()),
            boost::bind(
                &chat_session::handle_write,
                shared_from_this(),
                boost::asio::placeholders::error));
      }
    }
    else {
      std::cout << "Write error: " << error.message() << std::endl;
    }
  }
};
typedef boost::shared_ptr<chat_session> chat_session_ptr;

//----------------------------------------------------------------------
class chat_server {
private:
  boost::asio::io_context& io_context_;
  tcp::acceptor acceptor_;

public:
  chat_server(
      boost::asio::io_context& io_context,
      const tcp::endpoint& endpoint) :
        io_context_(io_context), acceptor_(io_context, endpoint) {
    start_accept();
  }
  void start_accept() {
    chat_session_ptr new_session(new chat_session(io_context_));
    acceptor_.async_accept(new_session->socket(), boost::bind(
        &chat_server::handle_accept, 
        this,
        new_session,
        boost::asio::placeholders::error));
  }
  void handle_accept(chat_session_ptr session,
        const boost::system::error_code& error) {
    if (!error) {
      session->start();
    }
    start_accept();
  }
};

typedef boost::shared_ptr<chat_server> chat_server_ptr;
typedef std::list<chat_server_ptr> chat_server_list;

//----------------------------------------------------------------------
int main(int argc, char* argv[]) {
  try {
    if (argc < 2) {
      std::cerr << "Usage: chat_server <port> [<port> ...]\n";
      return 1;
    }
    boost::asio::io_context io_context;
    chat_server_list servers;
    for (int i = 1; i < argc; ++i) {
      using namespace std; // For atoi.
      tcp::endpoint endpoint(tcp::v4(), atoi(argv[i]));
      chat_server_ptr server(new chat_server(io_context, endpoint));
      servers.push_back(server);
    }
    io_context.run();
  }
  catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }
  return 0;
}
