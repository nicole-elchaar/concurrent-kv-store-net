//
// kvclient.cc
//

#include <iostream>

class kvclient{
};

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
      std::cerr << "Usage: kvclient <host> <port>\n";
      return 1;
    }
  return 0;
}
