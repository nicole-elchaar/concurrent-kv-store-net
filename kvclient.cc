//
// kvclient.cc
// Key-value store client that makes requests to access key-value store over the
// network
//

#include <iostream>
#include <boost/asio.hpp>

class kvclient {
private:
  // Store the server port number and address
  int server_port;
  std::string server_addr;


public:
  kvclient(std::string hostname, int port) :
      server_port(port), server_addr(hostname) {
    // Create a socket and connect to the server
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::socket socket(io_service);
    boost::asio::ip::tcp::resolver resolver(io_service);
    
    try {
      auto endpoints = resolver.resolve({server_addr, std::to_string(server_port)});
      auto connected_to = boost::asio::connect(socket, endpoints);
      std::cout << "Connected to " << connected_to << std::endl;
    } catch (boost::system::system_error& se) {
      std::cerr << "Error " << se.what() << std::endl;
      exit(1);
    }
    // // Create hostname resolver
    // boost::asio::io_service io_service;
    // boost::asio::ip::tcp::resolver resolver(io_service);
    // boost::system::error_code ec;
    // for (auto&& result : resolver.resolve("www.google.com", "http", ec)) {
    //   std::cout << result.service_name() << " " << result.host_name() << " "
    //       << result.endpoint() << std::endl;
    // }
    // if (ec) {
    //   std::cout << "Error code: " << ec << std::endl;
    // } 
  }
};

// class kvclient {
//   // Store the server address and port number
//   std::string server_address;
//   int server_port;
// public:
//   kvclient(const std::string& server, int port) :
//       server_address(server), server_port(port) {
//     // Create a socket
//     int sockfd = socket(AF_INET, SOCK_STREAM, 0);
//     if (sockfd < 0) {
//       std::cerr << "Error opening socket\n";
//       exit(1);
//     }
//     // Get the server address
//     struct hostent* server = gethostbyname(server_address.c_str());
//     if (server == NULL) {
//       std::cerr << "Error, no such host\n";
//       exit(1);
//     }
//     // Create the server address
//     struct sockaddr_in server_addr;
//     bzero((char*) &server_addr, sizeof(server_addr));
//     server_addr.sin_family = AF_INET;
//     bcopy((char*) server->h_addr, (char*) &server_addr.sin_addr.s_addr,
//           server->h_length);
//     server_addr.sin_port = htons(server_port);

//     // Connect to the server
//     if (connect(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr))
//         < 0) {
//       std::cerr << "Error connecting\n";
//       exit(1);
//     }

//     // Send a message to the server
//     std::string msg = "Hello from client";
//     int n = write(sockfd, msg.c_str(), msg.length());
//     if (n < 0) {
//       std::cerr << "Error writing to socket\n";
//       exit(1);
//     }

//     // Read a message from the server
//     char buffer[256];
//     bzero(buffer, 256);
//     n = read(sockfd, buffer, 255);
//     if (n < 0) {
//       std::cerr << "Error reading from socket\n";
//       exit(1);
//     }
//     std::cout << buffer << std::endl;

//     // Close the socket
//     close(sockfd);
//   }

//   // RequestType enum
//   enum RequestType { GET, PUT, DEL };
//   struct Request {
//     RequestType type;
//     std::string key;
//     std::string value;    // Only used for PUT requests
//   };

//   struct Response {
//     bool success;
//     std::string value;    // Only used for GET requests
//   };

//   Response sendRequest(Request request) {
//     // Create request string
//     std::string request_string;
//     switch (request.type) {
//       case GET:
//         request_string =
//             "GET\nKEY-LEN: " + request.key.size() + "\n" + request.key + "\n";
//         break;
//       case PUT:
//         request_string =
//             "PUT\nKEY-LEN: " + request.key.size() + "\n" + request.key +
//             "\nVAL-LEN: " + request.value.size() + "\n" + request.value + "\n";
//         break;
//       case DEL:
//         request_string =
//             "DEL\nKEY-LEN: " + request.key.size() + "\n" + request.key + "\n";
//         break;
//       default:
//         std::cerr << "Invalid request type\n";
//         exit(1);
//     }

//     // Send request string


//     // Receive response string
//   }


// };


int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: kvclient <host> <port>\n";
    return 1;
  }

  // Create a kvclient object
  kvclient client(argv[1], atoi(argv[2]));

  return 0;
}
