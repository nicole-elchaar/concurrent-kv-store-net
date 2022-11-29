
//
// kvserver.cc
//

#include <iostream>

class kvserver{
};

//----------------------------------------------------------------------
int main(int argc, char* argv[]){
    if (argc != 2){
      std::cerr << "Usage: kvserver <port>\n";
      return 1;
    }
  return 0;
}
