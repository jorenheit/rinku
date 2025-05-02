#include <iostream>
#include <fstream>

#define RINKU_ENABLE_DEBUGGER
#include "bfcpu.h"

int main(int argc, char **argv) try {
  if (argc < 2) {
    std::cerr << "Insufficient arguments: " << argv[0] << " <program.bin>\n";
    return 1;
  }
  
  BFComputer(argv[1]).debug();
  
} catch (Rinku::Error::Exception &err) {
  std::cerr << err.what() << '\n';
}
