
//
// kvserver.cpp
// ~~~~~~~~~~~~~~~
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

//----------------------------------------------------------------------

typedef std::deque<message> message_queue;

//----------------------------------------------------------------------
class participant {
public:
  virtual ~participant() {}
  virtual void deliver(const message& msg) = 0;
};
typedef boost::shared_ptr<participant> participant_ptr;
//----------------------------------------------------------------------

class room {
private:
  std::set<participant_ptr> participants_;
  enum { max_recent_msgs = 100 };
  message_queue recent_msgs_;
public:
  void join(participant_ptr participant) {
    participants_.insert(participant);
    std::for_each(
        recent_msgs_.begin(),
        recent_msgs_.end(),
        boost::bind(
            &participant::deliver, participant,  boost::placeholders::_1));
  }

  void leave(participant_ptr participant) {
    participants_.erase(participant);
  }

  void deliver(const message& msg) {
    recent_msgs_.push_back(msg);
    while (recent_msgs_.size() > max_recent_msgs) 
      recent_msgs_.pop_front();

    std::for_each(participants_.begin(), participants_.end(), boost::bind(&participant::deliver, boost::placeholders::_1, boost::ref(msg)));
  }
};

//----------------------------------------------------------------------
class session : public participant,
    public boost::enable_shared_from_this<session> {
private:
  tcp::socket socket_;
  // room& room_;
  message read_msg_;
  message_queue write_msgs_;
public:
  // session(boost::asio::io_context& io_context, room& room) :
  //     socket_(io_context), room_(room) {}
  session(boost::asio::io_context& io_context) : socket_(io_context) {}

  tcp::socket& socket() {
    return socket_;
  }

  void start() {
    // room_.join(shared_from_this());
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(read_msg_.data(), message::header_length),
        boost::bind(
            &session::handle_read_header,
            shared_from_this(),
            boost::asio::placeholders::error));
    std::cout<< "Message received:" << read_msg_.data() << std::endl;
  }

  void deliver(const message& msg) {
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress) {
      boost::asio::async_write(
          socket_,
          boost::asio::buffer(
              write_msgs_.front().data(), write_msgs_.front().length()),
          boost::bind(
            &session::handle_write,
            shared_from_this(),
            boost::asio::placeholders::error));
    }
  }

  void handle_read_header(const boost::system::error_code& error) {
    if (!error && read_msg_.decode_header()) {
      boost::asio::async_read(socket_,
      boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
      boost::bind(&session::handle_read_body, shared_from_this(),
      boost::asio::placeholders::error));
    }
    else {
      // room_.leave(shared_from_this());
    }
  }

  void handle_read_body(const boost::system::error_code& error) {
    if (!error) {
      // room_.deliver(read_msg_);
      deliver(read_msg_);
      boost::asio::async_read(
          socket_,
          boost::asio::buffer(read_msg_.data(), message::header_length),
          boost::bind(
              &session::handle_read_header, 
              shared_from_this(),
              boost::asio::placeholders::error));
    }
    else {
      // room_.leave(shared_from_this());
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
                &session::handle_write,
                shared_from_this(),
                boost::asio::placeholders::error));
      }
    }
    else {
      // room_.leave(shared_from_this());
    }
  }
};
typedef boost::shared_ptr<session> session_ptr;

//----------------------------------------------------------------------
class kvserver {
private:
  boost::asio::io_context& io_context_;
  tcp::acceptor acceptor_;
  // room room_;

public:
  kvserver(
      boost::asio::io_context& io_context,
      const tcp::endpoint& endpoint) :
        io_context_(io_context), acceptor_(io_context, endpoint) {
    start_accept();
  }
  void start_accept() {
    // session_ptr new_session(new session(io_context_, room_));
    session_ptr new_session(new session(io_context_));
    acceptor_.async_accept(new_session->socket(), boost::bind(
        &kvserver::handle_accept, 
        this,
        new_session,
        boost::asio::placeholders::error));
  }
  void handle_accept(session_ptr session,
      const boost::system::error_code& error) {
    if (!error) {
      session->start();
    }
    start_accept();
  }
};

typedef boost::shared_ptr<kvserver> kvserver_ptr;
typedef std::list<kvserver_ptr> kvserver_list;

//----------------------------------------------------------------------
int main(int argc, char* argv[]) {
  try {
    if (argc < 2) {
      std::cerr << "Usage: kvserver <port> [<port> ...]\n";
      return 1;
    }
    boost::asio::io_context io_context;
    kvserver_list servers;
    for (int i = 1; i < argc; ++i) {
      using namespace std; // For atoi.
      tcp::endpoint endpoint(tcp::v4(), atoi(argv[i]));
      kvserver_ptr server(new kvserver(io_context, endpoint));
      servers.push_back(server);
    }
    io_context.run();
  }
  catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }
  return 0;
}
