//
// kvclient.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2021 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <cstdlib>
#include <deque>
#include <iostream>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind/bind.hpp>
#include "message.hpp"

using boost::asio::ip::tcp;

typedef std::deque<message> message_queue;

class kvclient {
private:
  boost::asio::io_context& io_context_;
  tcp::socket socket_;
  message read_msg_;
  message_queue write_msgs_;
  void handle_connect(const boost::system::error_code& error) {
    if (!error) {
      boost::asio::async_read(
          socket_,
          boost::asio::buffer(
              read_msg_.data(),
              message::header_length),
          boost::bind(
              &kvclient::handle_read_header,
              this,
              boost::asio::placeholders::error));
    }
  }

void handle_read_header(const boost::system::error_code& error) {
    if (!error && read_msg_.decode_header()) {
      boost::asio::async_read(socket_,
          boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
          boost::bind(
              &kvclient::handle_read_body,
              this,
              boost::asio::placeholders::error));
    }
    else {
      do_close();
    }
}

void handle_read_body(const boost::system::error_code& error) {
    if (!error) {
      // std::cout.write(read_msg_.body(), read_msg_.body_length());
      // std::cout << "\n";
      boost::asio::async_read(socket_,
          boost::asio::buffer(read_msg_.data(), message::header_length),
          boost::bind(&kvclient::handle_read_header, this,
          boost::asio::placeholders::error));
    }
    else {
      do_close();
    }
}

void do_write(message msg) {
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress) {
        boost::asio::async_write(socket_,
            boost::asio::buffer(write_msgs_.front().data(),
            write_msgs_.front().length()),
            boost::bind(&kvclient::handle_write, this,
            boost::asio::placeholders::error));
    }
}

  void handle_write(const boost::system::error_code& error) {
    if (!error) {
      write_msgs_.pop_front();
      if (!write_msgs_.empty()) {
        boost::asio::async_write(
            socket_,
            boost::asio::buffer(
                write_msgs_.front().data(),
                write_msgs_.front().length()),
            boost::bind(
                &kvclient::handle_write,
                this,
                boost::asio::placeholders::error));
      }
    }
    else {
      do_close();
    }
  }

  void do_close() {
    socket_.close();
  }

public:
  kvclient(
        boost::asio::io_context& io_context,
        const tcp::resolver::results_type& endpoints) :
        io_context_(io_context), socket_(io_context) {
    boost::asio::async_connect(
        socket_,
        endpoints,
        boost::bind(
            &kvclient::handle_connect,
            this,
            boost::asio::placeholders::error));
  }

  void write(const message& msg) {
    boost::asio::post(
        io_context_, boost::bind(&kvclient::do_write, this, msg));
  }

  void close() {
    boost::asio::post(io_context_, boost::bind(&kvclient::do_close, this));
  }
};

int main(int argc, char* argv[]) {
  try {
    if (argc != 3) {
      std::cerr << "Usage: kvclient <host> <port>\n";
      return 1;
    }

    boost::asio::io_context io_context;
    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints = resolver.resolve(argv[1], argv[2]);

    kvclient c(io_context, endpoints);

    boost::thread t(boost::bind(&boost::asio::io_context::run, &io_context));

    char line[message::max_body_length + 1];
    while (std::cin.getline(line, message::max_body_length + 1)) {
      using namespace std; // For strlen and memcpy.
      message msg;
      msg.body_length(strlen(line));
      memcpy(msg.body(), line, msg.body_length());
      msg.encode_header();
      c.write(msg);

      generic_message gmsg;
      gmsg.first_length(strlen(line));
      memcpy(gmsg.first(), line, gmsg.first_length());
      gmsg.second_length(strlen(line));
      memcpy(gmsg.second(), line, gmsg.second_length());
      gmsg.encode_header(PUT);
      std::cout << "first" << gmsg.first() << std::endl;

      // Parse first from generic message
      char first[gmsg.first_length() + 1];
      memcpy(first, gmsg.first(), gmsg.first_length());
      first[gmsg.first_length()] = '\0';
      std::cout << "first: " << first << std::endl;

      std::cout << "msg.length(): " << msg.length() << std::endl;
      std::cout << "gmsg.length(): " << gmsg.length() << std::endl;
      std::cout << "msg.body(): " << msg.body() << std::endl;
      std::cout << "gmsg.first(): " << gmsg.first() << std::endl;
      std::cout << "gmgs.second(): " << gmsg.second() << std::endl;
      std::cout << "msg.data(): " << msg.data() << std::endl;
      std::cout << "gmsg.data: " << gmsg.data() << std::endl;
    }

    c.close();
    t.join();
  }
  catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
