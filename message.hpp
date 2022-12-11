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

enum message_type {
    OK = 0, ERROR = 1, GET = 3, PUT = 4, DEL = 5 };

// Convert message type to string
const char* message_type_to_string(message_type type) {
  switch (type) {
    case OK:
      return "OK ";
    case ERROR:
      return "ERR";
    case GET:
      return "GET";
    case PUT:
      return "PUT";
    case DEL:
      return "DEL";
    default:
      return "\0\0\0";
  }
}

// Convert string to message type
message_type string_to_message_type(const char* str) {
  if (strncmp(str, "OK ", 3) == 0) {
    return OK;
  } else if (strncmp(str, "ERR", 3) == 0) {
    return ERROR;
  } else if (strncmp(str, "GET", 3) == 0) {
    return GET;
  } else if (strncmp(str, "PUT", 3) == 0) {
    return PUT;
  } else if (strncmp(str, "DEL", 3) == 0) {
    return DEL;
  } else {
    return ERROR;
  }
}

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

  char* body() {
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
    char header[header_length + 1] = "";
    std::strncat(header, data_, header_length);
    body_length_ = std::atoi(header);
    if (body_length_ > max_body_length) {
      body_length_ = 0;
      return false;
    }
    return true;
  }

  void encode_header() {
    char header[header_length + 1] = "";
    std::sprintf(header, "%4d", static_cast<int>(body_length_));
    std::memcpy(data_, header, header_length);
  }

  bool encode_body(message_type type, const char* key, const char* value) {
    // Fail if newlines are present in key or value
    if (std::strchr(key, '\n') != NULL || std::strchr(value, '\n') != NULL) {
      return false;
    }

    // Flush the body
    std::memset(data_ + header_length, '\0', max_body_length);

    char body[max_body_length] = "";
    std::sprintf(
        body,
        "%s\n%d\n%s\n%d\n%s\n",
        message_type_to_string(type),
        strlen(key),
        key,
        strlen(value),
        value);
    this->body_length(std::strlen(body));
    std::memcpy(data_ + header_length, body, body_length_);
    encode_header();

    std::cout << "encode type: " << message_type_to_string(type) << std::endl;
    std::cout << "encode key: " << key << std::endl;
    std::cout << "encode value: " << value << std::endl;
    return true;
  }

  bool decode_body(message_type& type, char* key, char* value) {
    // Parse body
    decode_header();
    char body[body_length_] = "";
    std::strncat(body, data_ + header_length, body_length_);

    // Get type
    char* token = std::strtok(body, "\n");
    type = string_to_message_type(token);
    std::cout << "decode type: " << message_type_to_string(type) << std::endl;

    if (type == ERROR) {
      return true;
    }

    // Get key and value
    token = std::strtok(NULL, "\n");
    int key_length = std::atoi(token);
    if (key_length == 0) {
      return false;
    }
    token = std::strtok(NULL, "\n");
    std::strncpy(key, token, key_length);
    std::cout << "decode key: " << key << std::endl;
    token = std::strtok(NULL, "\n");

    if (type == PUT) {
      int value_length = std::atoi(token);
      token = std::strtok(NULL, "\n");
      std::strncpy(value, token, value_length);
      std::cout << "decode value: " << value << std::endl;
    }

    return true;
  }
};

// class message {
// public:
//   const static int header_length = 4;
//   const static int max_body_length = 512;

// private:
//   char data_[header_length + max_body_length];
//   size_t body_length_;

// public:
//   message() : body_length_(0) {}

//   const char* data() const {
//     return data_;
//   }

//   char* data() {
//     return data_;
//   }

//   size_t length() const {
//     return header_length + body_length_;
//   }

//   const char* body() const {
//     return data_ + header_length;
//   }

//   char* body() {
//     return data_ + header_length;
//   }

//   size_t body_length() const {
//     return body_length_;
//   }

//   void body_length(size_t new_length) {
//     body_length_ = new_length;
//     if (body_length_ > max_body_length)
//       body_length_ = max_body_length;
//   }

//   bool decode_header() {
//     using namespace std; // For strncat and atoi.
//     char header[header_length + 1] = "";
//     strncat(header, data_, header_length);
//     body_length_ = atoi(header);
//     if (body_length_ > max_body_length) {
//       body_length_ = 0;
//       return false;
//     }
//     return true;
//   }

//   void encode_header() {
//     using namespace std; // For sprintf and memcpy.
//     char header[header_length + 1] = "";
//     sprintf(header, "%4d", static_cast<int>(body_length_));
//     memcpy(data_, header, header_length);
//   }
// };

// typedef enum { GET = 3, PUT = 4, DEL = 5, OK = 0, ERROR = 1 } message_type;

// class message {
// public:
//   const static int header_length = 3;
//   const static int max_body_length = 512;
//   // const static int offset_length = 3;

// private:
//   char header_[header_length];
//   internal_message first_;
//   internal_message second_;
//   message_type type_;

// public:
//   message() : type_(ERROR) {}

//   const char* data() const {
//     return header_;
//   }

//   char* data() {
//     return header_;
//   }

//   size_t length() const {
//     return header_length + first_.length() + second_.length();
//   }

//   const char* body() const {
//     return first_.body();
//   }

//   char* body() {
//     return first_.body();
//   }

//   size_t body_length() const {
//     return first_.body_length() + second_.body_length();
//   }

//   const char* first() const {
//     return first_.body();
//   }

//   char* first() {
//     return first_.body();
//   }

//   size_t first_length() const {
//     return first_.body_length();
//   }

//   void first_length(size_t length) {
//     first_.body_length(length);
//   }

//   const char* second() const {
//     return second_.body();
//   }

//   char* second() {
//     return second_.body();
//   }

//   size_t second_length() const {
//     return second_.body_length();
//   }

//   void second_length(size_t length) {
//     second_.body_length(length);
//   }

//   message_type type() const {
//     return type_;
//   }

//   void type(message_type type) {
//     type_ = type;
//   }

//   bool decode_header() {
//     using namespace std; // For strncat and atoi.
//     char header[header_length + 1] = "";
//     strncat(header, header_, header_length);
//     type_ = static_cast<message_type>(atoi(header));
//     if (type_ < GET || type_ > ERROR) {
//       type_ = ERROR;
//       return false;
//     }
//     if (!first_.decode_header() || !second_.decode_header()) {
//       return false;
//     }
//     return true;
//   }

//   void encode_header() {
//     using namespace std; // For sprintf and memcpy.
//     char header[header_length + 1] = "";
//     sprintf(header, "%3d", static_cast<int>(type_));
//     memcpy(header_, header, header_length);

//     // Write whatever data we have to output.
//     std::cout << "encode_header: " << header_ << std::endl;

//     first_.encode_header();
//     std::cout << "encode_header_first: " << first_.data() << std::endl;
//     second_.encode_header();
//     std::cout << "encode_header_second: " << second_.data() << std::endl;
//   }
// };

#endif // MESSAGE_HPP