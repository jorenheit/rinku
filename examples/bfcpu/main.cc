#include <iostream>
#include <fstream>
#include "bfcpu.h"

int main(int argc, char **argv) try {
  if (argc < 2) {
    std::cerr << "Insufficient arguments: " << argv[0] << " <program.bin> [VCD file]\n";
    return 1;
  }
  
  BFComputer cpu(argv[1], 1e5);
  Rinku::signal_t err = cpu.run(true);

  if (argc > 2) {
    std::ofstream vcdFile(argv[2]);
    vcdFile << cpu.vcd() << '\n';
  }
  
  std::cout << "DONE! Exit code: " << err << '\n';
 } catch (Rinku::Error::Exception &err) {
  std::cerr << err.what() << '\n';
 }
