//
// message.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2021 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>

class message {
public:
  const static int header_length = 4;
  const static int max_body_length = 512;

private:
  char data_[header_length + max_body_length];
  size_t body_length_;

public:
  message() : body_length_(0) {}

  const char* data() const {
    return data_;
  }

  char* data() {
    return data_;
  }

  size_t length() const {
    return header_length + body_length_;
  }

  const char* body() const {
    return data_ + header_length;
  }

  char* body(){
    return data_ + header_length;
  }

  size_t body_length() const {
    return body_length_;
  }

  void body_length(size_t new_length) {
    body_length_ = new_length;
    if (body_length_ > max_body_length)
      body_length_ = max_body_length;
  }

  bool decode_header() {
    using namespace std; // For strncat and atoi.
    char header[header_length + 1] = "";
    strncat(header, data_, header_length);
    body_length_ = atoi(header);
    if (body_length_ > max_body_length) {
      body_length_ = 0;
      return false;
    }
    return true;
  }

  void encode_header() {
    using namespace std; // For sprintf and memcpy.
    char header[header_length + 1] = "";
    sprintf(header, "%4d", static_cast<int>(body_length_));
    memcpy(data_, header, header_length);
  }
};

enum message_type { GET, PUT, DEL, OK, ERROR };

class generic_message {
public:
  const static int header_length = 5;
  const static int max_body_length = 1024;
  const static int offset_length = 4;

private:
  char data_[header_length + max_body_length];
  message_type type_;
  size_t first_length_;
  size_t second_length_;

public:
  generic_message() : first_length_(0), second_length_(0) {}

  const char* data() const {
    return data_;
  }

  char* data() {
    return data_;
  }

  size_t length() const {
    return header_length + offset_length ;
  }

  const char* body() const {
    return data_ + header_length;
  }

  char* body() {
    return data_ + header_length;
  }

  size_t body_length() const {
    return first_length_ + second_length_ + 2 * offset_length;
  }

  const char* first() const {
    return data_ + header_length + offset_length;
  }

  char* first() {
    return data_ + header_length + offset_length;
  }

  size_t first_length() const {
    return first_length_;
  }

  void first_length(size_t length) {
    first_length_ = length;
    if (first_length_ + second_length_ > max_body_length) {
      first_length_ = first_length_ > max_body_length ? max_body_length : first_length_;
      second_length_ = max_body_length - first_length_;
    }
  }

  const char* second() const {
    return data_ + header_length + offset_length + first_length_ + offset_length;
  }

  char* second() {
    return data_ + header_length + offset_length + first_length_ + offset_length;
  }

  size_t second_length() const {
    return second_length_;
  }

  void second_length(size_t length) {
    second_length_ = length;
    if (second_length_ > max_body_length) {
      first_length_ = first_length_ > max_body_length ? max_body_length : first_length_;
      second_length_ = max_body_length - first_length_;
    }
  }

  void body_length(size_t first_length, size_t second_length) {
    first_length_ = first_length;
    second_length_ = second_length;
    if (first_length_ + second_length_ > max_body_length) {
      first_length_ = first_length_ > max_body_length ? max_body_length : first_length_;
      second_length_ = max_body_length - first_length_;
    }
  }

  bool decode_header() {
    using namespace std; // For strncat and atoi.
    char header[header_length + 1] = "";
    strncat(header, data_, header_length);

    // Get the type of the message
    if (header == "GET") {
      type_ = message_type::GET;
    } else if (header == "PUT\n") {
      type_ = message_type::PUT;
    } else if (header == "DEL\n") {
      type_ = message_type::DEL;
    } else if (header == "OK \n") {
      type_ = message_type::OK;
    } else if (header == "ERR\n") {
      type_ = message_type::ERROR;
    } else {
      return false;
    }

    if (type_ == message_type::ERROR) {
      return true;
    }

    // Get the first length
    char first_length[offset_length + 1] = "";
    strncat(first_length, data_ + header_length, offset_length);
    first_length_ = atoi(first_length);
    if (first_length_ > max_body_length) {
      first_length_ = 0;
      return false;
    }

    if (type_ == message_type::PUT) {
      // Get the second length
      char second_length[offset_length + 1] = "";
      strncat(second_length, data_ + header_length + offset_length, offset_length);
      second_length_ = atoi(second_length);
      if (second_length_ > max_body_length) {
        second_length_ = 0;
        return false;
      }
    }

    return true;
    // body_length_ = atoi(header);
    // if (body_length_ > max_body_length) {
    //   body_length_ = 0;
    //   return false;
    // }
    // return true;
  }

  void encode_header(message_type type) {
    // Set the header bytes to type of message
    char header[header_length + 1] = "";
    if (type == message_type::GET) {
      sprintf(header, "GET\n");
    } else if (type == message_type::PUT) {
      sprintf(header, "PUT\n");
    } else if (type == message_type::DEL) {
      sprintf(header, "DEL\n");
    } else if (type == message_type::OK) {
      sprintf(header, "OK \n");
    } else if (type == message_type::ERROR) {
      sprintf(header, "ERR\n");
    }
    memcpy(data_, header, header_length);

    if (type == message_type::ERROR) {
      return;
    }

    // Set the first length
    char first_length[offset_length + 1] = "";
    sprintf(first_length, "%4d", static_cast<int>(first_length_));
    memcpy(data_ + header_length, first_length, offset_length);

    if (type == message_type::PUT) {
      // Set the second length
      char second_length[offset_length + 1] = "";
      sprintf(second_length, "%4d", static_cast<int>(second_length_));
      memcpy(data_ + header_length + offset_length + first_length_, second_length, offset_length);
    }

    return;

    // using namespace std; // For sprintf and memcpy.
    // char header[header_length + 1] = "";
    // sprintf(header, "%4d", static_cast<int>(body_length_));
    // memcpy(data_, header, header_length);
  }
};

#endif // MESSAGE_HPP