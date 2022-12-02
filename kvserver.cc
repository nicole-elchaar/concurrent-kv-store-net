//
// kvserver.cc
// Key-value store that accepts requests to access a centralized data storage
// over the network
//

#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class kvserver {
private:
  int port_;
  boost::asio::io_context& io_context_;
  boost::asio::ip::tcp::acceptor acceptor_;
  std::vector<TCPConection::pointer> connections_;

  void accept() {
    auto connection = TCPConection::create(io_context_);
    acceptor_.async_accept(
          connection->socket(),
          [this, connection](boost::system::error_code ec) {
            if (!ec) {
              connections_.push_back(connection);
              connection->start();
              print("Accepted connection from " << connection->socket().remote_endpoint());
            }
            accept();
          });
  }

public:
  kvserver(int port, int num_threads) :
      port_(port),
      acceptor_(
          io_context_,
          boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) {}
  
    

    // // Create a socket and display the IP of the server
    // boost::asio::io_service io_service;
    // boost::asio::ip::tcp::socket socket(io_service);
    // boost::asio::ip::tcp::resolver resolver(io_service);
    // boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), 0);
    // boost::asio::ip::tcp::acceptor acceptor(io_service, endpoint);
    // boost::asio::ip::tcp::endpoint local_endpoint = acceptor.local_endpoint();
    // std::cout << "Server IP: " << local_endpoint.address() << std::endl;

    // // Sleep
    // std::this_thread::sleep_for(std::chrono::seconds(10));
  // }
};

// class kvserver {
//   // Store the server port number
//   int server_port;
//   kvserver(int port) : server_port(port) {
//     // Create a socket
//     int sockfd = socket(AF_INET, SOCK_STREAM, 0);
//     if (sockfd < 0) {
//       std::cerr << "Error opening socket";
//       exit(1);
//     }
//     // Create the server address
//     struct sockaddr_in server_addr;
//     bzero((char*) &server_addr, sizeof(server_addr));
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_addr.s_addr = INADDR_ANY;
//     server_addr.sin_port = htons(server_port);
//     // Bind the socket to the server address
//     if (bind(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
//       std::cerr << "Error on binding";
//       exit(1);
//     }
//     // Listen for connections
//     listen(sockfd, 5);
//     // Accept a connection
//     struct sockaddr_in client_addr;
//     socklen_t client_len = sizeof(client_addr);
//     int newsockfd = accept(sockfd, (struct sockaddr*) &client_addr, &client_len);
//     if (newsockfd < 0) {
//       std::cerr << "Error on accept";
//       exit(1);
//     }
//     // Read a message from the client
//     char buffer[256];
//     bzero(buffer, 256);
//     int n = read(newsockfd, buffer, 255);
//     if (n < 0) {
//       std::cerr << "Error reading from socket";
//       exit(1);
//     }
//     std::cout << buffer << std::endl;
//     // Send a message to the client
//     std::string msg = "Hello from server";
//     n = write(newsockfd, msg.c_str(), msg.length());
//     if (n < 0) {
//       std::cerr << "Error writing to socket";
//       exit(1);
//     }
//     // Close the socket
//     close(newsockfd);
//     close(sockfd);
//   }
// };

//----------------------------------------------------------------------
int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: kvserver <port>\n";
    return 1;
  }

  int port = atoi(argv[1]);
  // Check that port number is 1895
  if (port != 1895) {
    std::cerr << "Error: port number must be 1895";
    return 1;
  }
  kvserver server(port);

  return 0;
}
