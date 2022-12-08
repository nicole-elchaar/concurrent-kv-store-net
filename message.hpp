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

typedef enum { GET = 1, PUT = 2, DEL = 3, OK = 4, ERROR = 5 } message_type;

class generic_message {
public:
  const static int header_length = 4;
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
    return
        header_length + 2 * offset_length + first_length_ + second_length_ + 2;
  }

  const char* body() const {
    return data_ + header_length;
  }

  char* body() {
    return data_ + header_length;
  }

  size_t body_length() const {
    return first_length_ + second_length_ + 2 * offset_length + 2;
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
    if (first_length_ > max_body_length) {
      first_length_ =
          first_length_ > max_body_length ? max_body_length : first_length_;
    }
  }

  const char* second() const {
    return
        data_ + header_length + offset_length + first_length_ + offset_length + 2;
  }

  char* second() {
    return
        data_ + header_length + offset_length + first_length_ + offset_length + 2;
  }

  size_t second_length() const {
    return second_length_;
  }

  void second_length(size_t length) {
    second_length_ = length;
    if (first_length_ + second_length_ > max_body_length) {
      second_length_ = max_body_length - first_length_;
    }
  }

  void body_length(size_t first_length, size_t second_length) {
    first_length_ = first_length;
    second_length_ = second_length;
    if (first_length_ + second_length_ > max_body_length) {
      first_length_ =
          first_length_ > max_body_length ? max_body_length : first_length_;
      second_length_ = max_body_length - first_length_;
    }
  }

  bool decode_header() {
    // Decode the header
    // Start with the message_type
    char header[header_length + 1] = "";
    memcpy(header, data_, header_length);
    if (strcmp(header, "GET\n") == 0) {
      type_ = GET;
    } else if (strcmp(header, "PUT\n") == 0) {
      type_ = PUT;
    } else if (strcmp(header, "DEL\n") == 0) {
      type_ = DEL;
    } else if (strcmp(header, "OK \n") == 0) {
      type_ = OK;
    } else if (strcmp(header, "ERR\n") == 0) {
      type_ = ERROR;
      return true;
    } else {
      std::cout << "Error: unrecognized message type" << std::endl;
      return false;
    }

    // Decode the first length
    char first_length[offset_length + 1] = "";
    memcpy(first_length, data_ + header_length, offset_length);
    first_length_ = atoi(first_length);

    if (first_length_ > max_body_length) {
      std::cout << "Error: first length is too long" << std::endl;
      return false;
    }

    if (type_ == PUT) {
      // Decode the second length
      char second_length[offset_length + 1] = "";
      memcpy(
          second_length,
          data_ + header_length + first_length_ + offset_length + 1,
          offset_length);
      second_length_ = atoi(second_length);

      if (second_length_ > max_body_length) {
        std::cout << "Error: second length is too long" << std::endl;
        return false;
      }
    }

    return true;

    // using namespace std; // For strncat and atoi.
    // // Decode an encoded header
    // char header[header_length + 1] = "";
    // memcpy(header, data_, header_length);
    // if (strcmp(header, "GET\n") == 0) {
    //   type_ = GET;
    // } else if (strcmp(header, "PUT\n") == 0) {
    //   type_ = PUT;
    // } else if (strcmp(header, "DEL\n") == 0) {
    //   type_ = DEL;
    // } else if (strcmp(header, "OK \n") == 0) {
    //   type_ = OK;
    // } else if (strcmp(header, "ERR\n") == 0) {
    //   type_ = ERROR;
    //   return true;
    // } else {
    //   cout << "Error: Unknown message type" << endl;
    //   return false;
    // }

    // cout << "Header: " << header << endl;
    // cout << "Data: " << data_ << endl;

    // // Decode the first length
    // char first_length[offset_length + 1] = "";
    // memcpy(first_length, data_ + header_length, offset_length);
    // first_length_ = atoi(first_length);
    // cout << "First length: " << first_length << endl;
    // cout << "First length: " << first_length_ << endl;

    // // Decode the second length
    // if (type_ == PUT) {
    //   char second_length[offset_length + 1] = "";
    //   memcpy(second_length, data_ + header_length + offset_length + first_length_,
    //          offset_length);
    //   second_length_ = atoi(second_length);
    //   cout << "Second length: " << second_length << endl;
    //   cout << "Second length: " << second_length_ << endl;
    // }
    // return true;


    // // using namespace std; // For strncat and atoi.
    // // char header[header_length + 1] = "";
    // // memcpy(header, data_, header_length);
    // // // strncat(header, data_, header_length);

    // // // Compare string to possible types
    // // if (strcmp(header, "GET\n") == 0) {
    // //   type_ = GET;
    // // } else if (strcmp(header, "PUT\n") == 0) {
    // //   type_ = PUT;
    // // } else if (strcmp(header, "DEL\n") == 0) {
    // //   type_ = DEL;
    // // } else if (strcmp(header, "OK \n") == 0) {
    // //   type_ = OK;
    // // } else if (strcmp(header, "ERR\n") == 0) {
    // //   type_ = ERROR;
    // // } else {
    // //   std::cout << "Unknown message type: " << header << std::endl;
    // //   return false;
    // // }

    // // if (type_ == ERROR) {
    // //   return true;
    // // }

    // // // Get the first length
    // // char first_length[offset_length + 1] = "";
    // // memcpy(first_length, data_ + header_length, offset_length);
    // // // strncat(first_length, data_ + header_length, offset_length);
    // // first_length_ = atoi(first_length);
    // // if (first_length_ > max_body_length) {
    // //   first_length_ = 0;
    // //   return false;
    // // }

    // // if (type_ == PUT) {
    // //   // Get the second length
    // //   char second_length[offset_length + 1] = "";
    // //   // strncat(
    // //   //     second_length, data_ + header_length + offset_length, offset_length);
    // //   memcpy(second_length, data_ + header_length + offset_length + first_length_ + 1, offset_length);
    // //   second_length_ = atoi(second_length);
    // //   if (second_length_ > max_body_length) {
    // //     second_length_ = 0;
    // //     return false;
    // //   }
    // // }

    // // return true;
  }

  void encode_header(message_type type) {
    // Encode the header
    // Start with type and newline
    type_ = type;
    switch (type) {
      case ERROR:
        memcpy(data_, "ERR\n", header_length);
        return;
      case OK:
        memcpy(data_, "OK \n", header_length);
        break;
      case GET:
        memcpy(data_, "GET\n", header_length);
        break;
      case PUT:
        memcpy(data_, "PUT\n", header_length);
        break;
      case DEL:
        memcpy(data_, "DEL\n", header_length);
        break;
      default:
        std::cout << "Error: Unknown message type" << std::endl;
        return;
    }
    // if (type == ERROR) {
    //   memcpy(data_, "ERR\n", header_length);
    //   return;
    // } else if (type == OK) {
    //   memcpy(data_, "OK \n", header_length);
    // } else if (type == GET) {
    //   memcpy(data_, "GET\n", header_length);
    // } else if (type == PUT) {
    //   memcpy(data_, "PUT\n", header_length);
    // } else if (type == DEL) {
    //   memcpy(data_, "DEL\n", header_length);
    // } else {
    //   std::cout << "Error: Unknown message type" << std::endl;
    // }

    // Encode the first length and newline
    char len[offset_length + 1] = "";
    sprintf(len, "%3d\n", first_length_);
    // std::cout << "Len: " << len << std::endl;
    memcpy(data_ + header_length, len, offset_length);
    // std::cout << "First length: " << first_length_ << std::endl;

    // Encode the second length and newline
    if (type_ == PUT) {
      // char second_length[offset_length + 1] = "";
      // std::cout << "4First length: " << len << std::endl;
      // std::cout << "Len: " << len << std::endl;
      sprintf(len, "%3d\n", second_length_);
      // std::cout << "Len: " << len << std::endl;
      // std::cout << "5First length: " << first_length << std::endl;
      memcpy(data_ + header_length + offset_length + first_length_ + 1, len, offset_length);
      // std::cout << "6First length: " << first_length << std::endl;
      // std::cout << "Second length: " << second_length_ << std::endl;
    }

    // if (type == ERROR) {
    //   memcpy(data_, "ERR\n", header_length);
    //   return;
    // } else if (type == OK) {
    //   memcpy(data_, "OK \n", header_length);
    // } else if (type == GET) {
    //   memcpy(data_, "GET\n", header_length);
    // } else if (type == PUT) {
    //   memcpy(data_, "PUT\n", header_length);
    // } else if (type == DEL) {
    //   memcpy(data_, "DEL\n", header_length);
    // }
    // // // Set the header bytes to type of message
    // // char header[header_length + 1] = "";
    // // if (type == GET) {
    // //   sprintf(header, "GET\n");
    // // } else if (type == PUT) {
    // //   sprintf(header, "PUT\n");
    // // } else if (type == DEL) {
    // //   sprintf(header, "DEL\n");
    // // } else if (type == OK) {
    // //   sprintf(header, "OK \n");
    // // } else if (type == ERROR) {
    // //   sprintf(header, "ERR\n");
    // // }
    // // memcpy(data_, header, header_length);

    // // Set the first length
    // char first_length[offset_length + 1] = "";
    // sprintf(first_length, "%4d", static_cast<int>(first_length_));
    // memcpy(data_ + header_length, first_length, offset_length);
    // memset(data_ + header_length + offset_length, '\n', sizeof(char));

    // if (type == PUT) {
    //   // Set the second length
    //   char second_length[offset_length + 1] = "";
    //   sprintf(second_length, "%4d", static_cast<int>(second_length_));
    //   memcpy(
    //       data_ + header_length + offset_length + first_length_,
    //       second_length,
    //       offset_length);
    //   memset(data_ + header_length + offset_length + first_length_ + offset_length, '\n', sizeof(char));
    // }

    // return;
  }
};

#endif // MESSAGE_HPP