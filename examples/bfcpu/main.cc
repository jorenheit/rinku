#include <iostream>
#include <fstream>
#include "bfcpu.h"

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Insufficient arguments: " << argv[0] << " <program.bin>\n";
    return 1;
  }
  
  BFComputer cpu(argv[1]);
  Rinku::signal_t err = cpu.run(true);
  std::cout << "DONE! Exit code: " << err << '\n';
}

