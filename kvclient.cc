// Create a boost asio client that sends GET, PUT, and DELETE requests to the 
// server and reads OK and ERROR responses

#ifndef KVCLIENT_H
#define KVCLIENT_H

#include <iostream>
#include <string>
#include <boost/bind/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/asio.hpp>

#include "message.cc"

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
    message msg(GET, key, "");
    send_request(msg);
    message response = read_response_msg();
    if (response.get_type() == OK) {
      value = response.get_value();
      return true;
    }
    return false;
    // std::cout << "GET: " << response.get_type() << std::endl;
  }

  bool put(const std::string& key, const std::string& value) {
    message msg(PUT, key, value);
    cout << "Sending PUT: " << msg.get_type() << endl;
    send_request(msg);
    cout << "Sent PUT: " << msg.get_type() << endl;
    message response = read_response_msg();
    cout << "Received Response: " << response.get_type() << endl;
    // cout << "PUT: " << response.get_type() << endl;

    if (response.get_type() == OK) {
      return true;
    }
    return false;
    // std::cout << "PUT: " << response.get_type() << std::endl;
  }

  bool del(const std::string& key) {
    message msg(DEL, key, "");
    send_request(msg);
    message response = read_response_msg();
    if (response.get_type() == OK) {
      return true;
    }
    return false;
    // std::cout << "DEL: " << response.get_type() << std::endl;
  }

private:
  boost::asio::io_service& io_service_;
  tcp::socket socket_;
};

// int main(int argc, char* argv[]) {
//   try {
//     if (argc != 3) {
//       std::cerr << "Usage: kvclient <host> <port>" << std::endl;
//       return 1;
//     }

//     boost::asio::io_service io_service;
//     kvclient client(io_service, argv[1], argv[2]);

//     std::string request;
//     std::string response;

//     message msg;

//     // PUT
//     msg.reset(PUT, "key1", "value1");
//     msg.encode(request);
//     client.send_request(request);
//     response = client.read_response();
//     if (!msg.decode(response)) {

//     }
//     std::cout << "PUT: " << msg.get_type() << std::endl;

//     // // PUT
//     // message = put_message(PUT, "key1", "value1");
//     // std::string request = put_message.to_string();
//     // client.send_request(request);

//     // message put_response(client.read_response());
//     // std::cout << "PUT: " << put_response.to_string() << std::endl;

//     // // GET
//     // message get_message(GET, "key1", "");
//     // request = get_message.to_string();
//     // client.send_request(request);

//     // message get_response(client.read_response());
//     // std::cout << "GET: " << get_response.to_string() << std::endl;

//     // // DELETE
//     // message del_message(DEL, "key1", "");
//     // request = del_message.to_string();
//     // client.send_request(request);

//     // message del_response(client.read_response());
//     // std::cout << "DEL: " << del_response.to_string() << std::endl;

//     // // GET
//     // message get_message2(GET, "key1", "");
//     // request = get_message2.to_string();
//     // client.send_request(request);

//     // message get_response2(client.read_response());
//     // std::cout << "GET: " << get_response2.to_string() << std::endl;

//     // // DELETE
//     // message del_message2(DEL, "key1", "");
//     // request = del_message2.to_string();
//     // client.send_request(request);

//     // message del_response2(client.read_response());
//     // std::cout << "DEL: " << del_response2.to_string() << std::endl;




//     // // PUT
//     // request = "PUT key1 value1\n";
//     // client.send_request(request);
//     // response = client.read_response();
//     // std::cout << "PUT: " << response << std::endl;

//     // // GET
//     // request = "GET key1\n";
//     // client.send_request(request);
//     // response = client.read_response();
//     // std::cout << "GET: " << response << std::endl;

//     // // DELETE
//     // request = "DEL key1\n";
//     // client.send_request(request);
//     // response = client.read_response();
//     // std::cout << "DEL: " << response << std::endl;

//     // // GET
//     // request = "GET key1\n";
//     // client.send_request(request);
//     // response = client.read_response();
//     // std::cout << "GET: " << response << std::endl;

//   } catch (std::exception& e) {
//     std::cerr << "Exception: " << e.what() << std::endl;
//   }

//   return 0;

// };

#endif