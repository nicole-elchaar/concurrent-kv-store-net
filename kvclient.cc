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
    std::cout << "Handle connect" << std::endl;
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
    std::cout << "Handle read header" << std::endl;
    if (!error && read_msg_.decode_header()) {
      boost::asio::async_read(socket_,
          boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
          boost::bind(
              &kvclient::handle_read_body,
              this,
              boost::asio::placeholders::error));
      // message_type type;
      // char* key;
      // char* value;
      // read_msg_.decode_body(type, key, value);
      // std::cout << "Handle read header type: " << type << " key: " << key << " value: " << value << std::endl;
    }
    else {
      do_close();
    }
  }

  void handle_read_body(const boost::system::error_code& error) {
    std::cout << "Handle read body" << std::endl;
    if (!error) {
      std::cout.write(read_msg_.body(), read_msg_.body_length()) << std::endl;
      boost::asio::async_read(socket_,
          boost::asio::buffer(read_msg_.data(), message::header_length),
          boost::bind(&kvclient::handle_read_header, this,
          boost::asio::placeholders::error));
      // message_type type;
      // char* key;
      // char* value;
      // read_msg_.decode_body(type, key, value);
      // std::cout << "Handle read body type: " << type << " key: " << key << " value: " << value << std::endl;
    }
    else {
      do_close();
    }
  }

  void do_write(message msg) {
    std::cout << "Do write" << std::endl;
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
    std::cout << "Handle write" << std::endl;
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
    std::cout << "Do close" << std::endl;
    socket_.close();
  }

public:
  kvclient(
        boost::asio::io_context& io_context,
        const tcp::resolver::results_type& endpoints) :
        io_context_(io_context), socket_(io_context) {
    std::cout << "kvclient" << std::endl;
    boost::asio::async_connect(
        socket_,
        endpoints,
        boost::bind(
            &kvclient::handle_connect,
            this,
            boost::asio::placeholders::error));
  }

  void write(const message& msg) {
    std::cout << "Write" << std::endl;
    boost::asio::post(
        io_context_, boost::bind(&kvclient::do_write, this, msg));
  }

  void close() {
    std::cout << "Close" << std::endl;
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
      msg.encode_body(PUT, line, line);
      msg.encode_header();
      c.write(msg);
    }


  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }
  
  return 0;




  // //   // memcpy(line, "Hello, world!", 13);

  // //   // message gmsg;
  // //   // gmsg.type(PUT);
  // //   // gmsg.first_length(strlen(line));
  // //   // memcpy(gmsg.first(), line, gmsg.first_length());
  // //   // gmsg.second_length(strlen(line));
  // //   // memcpy(gmsg.second(), line, gmsg.second_length());
  // //   // gmsg.encode_header();
  // //   // c.write(gmsg);

  // //   // // Parse first from generic message
  // //   // char first[gmsg.first_length() + 1];
  // //   // memcpy(first, gmsg.first(), gmsg.first_length());
  // //   // first[gmsg.first_length()] = '\0';

  // //   // // Parse second from generic message
  // //   // char second[gmsg.second_length() + 1];
  // //   // memcpy(second, gmsg.second(), gmsg.second_length());

  // //   // Sleep
  // //   std::this_thread::sleep_for(std::chrono::seconds(1));

  // //   // // Copy generic message to a new message and confirm decoding
  // //   // message gmsg2;
  // //   // memcpy(gmsg2.data(), gmsg.data(), gmsg.length());
  // //   // gmsg2.decode_header();

  // //   while (std::cin.getline(line, message::max_body_length + 1)) {
  // //     using namespace std; // For strlen and memcpy.
  // //     // message msg;
  // //     // msg.body_length(strlen(line));
  // //     // memcpy(msg.body(), line, msg.body_length());
  // //     // msg.encode_header();
  // //     // c.write(msg);

  // //     message gmsg;
  // //     gmsg.type(PUT);
  // //     gmsg.first_length(strlen(line));
  // //     memcpy(gmsg.first(), line, gmsg.first_length());
  // //     gmsg.second_length(strlen(line));
  // //     memcpy(gmsg.second(), line, gmsg.second_length());
  // //     gmsg.encode_header();
  // //     c.write(gmsg);

  // //     // // Parse first from generic message
  // //     // char first[gmsg.first_length() + 1];
  // //     // memcpy(first, gmsg.first(), gmsg.first_length());
  // //     // first[gmsg.first_length()] = '\0';

  // //     // // std::cout << "msg.length(): " << msg.length() << std::endl;
  // //     // // std::cout << "gmsg.length(): " << gmsg.length() << std::endl;
  // //     // // std::cout << "msg.body(): " << msg.body() << std::endl;
  // //     // // std::cout << "gmsg.first(): " << gmsg.first() << std::endl;
  // //     // // std::cout << "gmgs.second(): " << gmsg.second() << std::endl;
  // //     // // std::cout << "msg.data(): " << msg.data() << std::endl;
  // //     // // std::cout << "gmsg.data: " << gmsg.data() << std::endl;

  // //     // // Copy generic message to a new message and confirm decoding
  // //     // message gmsg2;
  // //     // memcpy(gmsg2.data(), gmsg.data(), gmsg.length());
  // //     // gmsg2.decode_header();
  // //     // c.write(gmsg2);
  // //     // std::cout << "gmsg.first(): " << gmsg.first() << std::endl;
  // //     // std::cout << "gmsg2.first(): " << gmsg2.first() << std::endl;
  // //     // std::cout << "gmsg.first_length(): " << gmsg.first_length() << std::endl;
  // //     // std::cout << "gmsg2.first_length(): " << gmsg2.first_length() << std::endl;
  // //     // std::cout << "gmsg.second(): " << gmsg.second() << std::endl;
  // //     // std::cout << "gmsg2.second(): " << gmsg2.second() << std::endl;
  // //     // std::cout << "gmsg.second_length(): " << gmsg.second_length() << std::endl;
  // //     // std::cout << "gmsg2.second_length(): " << gmsg2.second_length() << std::endl;

  // //     // // Copy message_text to data of a new generic message
  // //     // char message_text[] = "PUT\n3  \nkey\n  13\nabcdefghijklm\n";
  // //     // message gmsg2;
  // //     // memcpy(gmsg2.data(), message_text, strlen(message_text));
  // //     // gmsg2.decode_header();

  // //     // std::cout << "gmsg2.first(): " << gmsg2.first() << std::endl;
  // //     // std::cout << "gmsg.first_length(): " << gmsg2.first_length() << std::endl;
  // //     // std::cout << "gmsg2.second(): " << gmsg2.second() << std::endl;
  // //     // std::cout << "gmsg2.second_length(): " << gmsg2.second_length() << std::endl;
  // //   }

  // //   c.close();
  // //   t.join();
  // // }
  // catch (std::exception& e) {
  //   std::cerr << "Exception: " << e.what() << "\n";
  // }

  // return 0;
}
