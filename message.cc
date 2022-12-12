// Create message class to build string requests (GET, PUT, DEL) for the client
// and responses (OK, ERR) for the server.  The messages can be encoded and
// decoded on the client and server side.

#ifndef MESSAGE_H
#define MESSAGE_H

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

enum message_type {GET = 3, PUT = 4, DEL = 5, OK = 0, ERROR = 1, UNSET = -1};

class message {
private:
  message_type type;
  std::string first;
  std::string second;
public:
  message(message_type type, std::string first, std::string second);
  message(message_type type, std::string first);
  message(message_type type);
  message();
  message(std::string encoded_message);
  std::string to_string();
  bool encode(std::string& encoded_message);
  bool decode(std::string& encoded_message);
  message_type get_type();
  std::string get_key();
  std::string get_value();
  bool reset(message_type type, std::string first, std::string second);
  bool reset(message_type type, std::string first);
  bool reset(message_type type);
  bool reset();
};

message::message(message_type type, std::string first, std::string second) {
  this->type = type;
  this->first = first;
  this->second = second;
}

message::message(message_type type, std::string first) {
  this->type = type;
  this->first = first;
}

message::message(message_type type) {
  this->type = type;
}

message::message() {
  this->type = UNSET;
}

message::message(std::string encoded_message) {
  decode(encoded_message);
}

std::string message::to_string() {
  std::string message_string;
  if (encode(message_string)) {
    return message_string;
  } else {
    return "ERR\n";
  }
}

bool message::decode(std::string& encoded_message) {
  std::stringstream ss(encoded_message);
  std::string token;
  std::vector<std::string> tokens;
  while (std::getline(ss, token, ' ')) {
    tokens.push_back(token);
  }
  
  if (tokens[0] == "ERR") {
    this->type = ERROR;
    return true;
  }

  if (tokens[0] == "OK") {
    this->type = OK;
    if (tokens.size() > 1) {
      this->second = tokens[1];
    }
    return true;
  }
  
  // Check that at least one additional token is present
  if (tokens.size() < 2) {
    return false;
  }

  if (tokens[0] == "GET") {
    this->type = GET;
    this->first = tokens[1];
  } else if (tokens[0] == "PUT") {
    if (tokens.size() < 3) {
      return false;
    }
    this->type = PUT;
    this->first = tokens[1];
    this->second = tokens[2];
  } else if (tokens[0] == "DEL") {
    this->type = DEL;
    this->first = tokens[1];

  }

  // Replace carriage returns in values with newlines
  while (this->second.find("\r") != std::string::npos) {
    this->second.replace(this->second.find("\r"), 1, "\n");
  }

  return true;
}

bool message::encode(std::string& encoded_message) {
  // Forbid newlines in keys, replace newlines in values with carriage returns
  if (this->first.find("\n") != std::string::npos) {
    return false;
  }
  while (this->second.find("\n") != std::string::npos) {
    this->second.replace(this->second.find("\n"), 1, "\r");
  }

  if (this->type == GET) {
    // Check that first is set
    if (this->first == "") {
      return false;
    }
    encoded_message = "GET " + this->first;
  } else if (this->type == PUT) {
    // Check that first and second are set
    if (this->first == "" || this->second == "") {
      return false;
    }
    encoded_message = "PUT " + this->first + " " + this->second;
  } else if (this->type == DEL) {
    // Check that first is set
    if (this->first == "") {
      return false;
    }
    encoded_message = "DEL " + this->first;
  } else if (this->type == OK) {
    encoded_message = "OK";
    if (this->first != "") {
      encoded_message += " " + this->first;
    }
  } else if (this->type == ERROR) {
    encoded_message = "ERR";
  }
  encoded_message += "\n";
  return true;
}

message_type message::get_type() {
  return this->type;
}

std::string message::get_key() {
  return this->first;
}

std::string message::get_value() {
  return this->second;
}

bool message::reset(message_type type, std::string first, std::string second) {
  this->type = type;
  this->first = first;
  this->second = second;
  return true;
}

bool message::reset(message_type type, std::string first) {
  this->type = type;
  this->first = first;
  this->second = "";
  return true;
}

bool message::reset(message_type type) {
  this->type = type;
  this->first = "";
  this->second = "";
  return true;
}

bool message::reset() {
  this->type = UNSET;
  this->first = "";
  this->second = "";
  return true;
}

#endif