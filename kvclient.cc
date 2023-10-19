/* 
 * Nicole ElChaar, CSE 411, Fall 2022
 *
 * This file contains the kvclient class which sends requests to the server
 * and handles responses.
 */

#ifndef KVCLIENT_H
#define KVCLIENT_H

#include "message.hpp"

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class kvclient {
public:
  kvclient(
      boost::asio::io_service& io_service,
      const std::string& host,
      const std::string& port)
        : io_service_(io_service), socket_(io_service) {
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(host, port);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    tcp::resolver::iterator end;
    boost::system::error_code error = boost::asio::error::host_not_found;
    while (error && endpoint_iterator != end) {
      socket_.close();
      socket_.connect(*endpoint_iterator++, error);
    }
    if (error) {
      throw boost::system::system_error(error);
    }
  }

  void send_request(const std::string& request) {
    boost::asio::write(socket_, boost::asio::buffer(request));
  }

  void send_request(message& msg) {
    std::string request;
    msg.encode(request);
    boost::asio::write(socket_, boost::asio::buffer(request));
  }

  std::string read_response() {
    boost::asio::streambuf response;
    boost::asio::read_until(socket_, response, "\n");
    std::istream response_stream(&response);
    std::string line;
    std::getline(response_stream, line);
    return line;
  }

  message read_response_msg() {
    boost::asio::streambuf response;
    boost::asio::read_until(socket_, response, "\n");
    std::istream response_stream(&response);
    std::string line;
    std::getline(response_stream, line);
    message msg;
    msg.decode(line);

    return msg;
  }

  bool get(const std::string& key, std::string& value) {
    message msg(GET, key);
    if (msg.get_type() == UNSET || msg.get_type() == ERROR) {
      return false;
    }

    send_request(msg);
    message response = read_response_msg();

    if (response.get_type() == OK) {
      value = response.get_value();
      return true;
    }
    return false;
  }

  bool put(const std::string& key, const std::string& value) {
    message msg(PUT, key, value);
    if (msg.get_type() == UNSET || msg.get_type() == ERROR) {
      return false;
    }

    send_request(msg);
    message response = read_response_msg();

    if (response.get_type() == OK) {
      return true;
    }
    return false;
  }

  bool del(const std::string& key) {
    std::cout << "Deleting " << key << std::endl;
    message msg(DEL, key);
    std::cout << "Message created" << std::endl;
    if (msg.get_type() == UNSET || msg.get_type() == ERROR) {
      std::cout << "Message type is unset or error" << std::endl;
      return false;
    }

    std::cout << "Sending request" << std::endl;
    send_request(msg);
    std::cout << "Request sent" << std::endl;
    message response = read_response_msg();
    std::cout << "Response received" << std::endl;

    if (response.get_type() == OK) {
      std::cout << "Key found" << std::endl;
      return true;
    }
    std::cout << "Key not found" << std::endl;
    return false;
  }

private:
  boost::asio::io_service& io_service_;
  tcp::socket socket_;
};

#endif
