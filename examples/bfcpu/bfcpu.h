#ifndef BFCPU_H
#define BFCPU_H
#include <functional>

#define RINKU_REMOVE_MACRO_PREFIX
#include "../../rinku/rinku.h"

struct BFComputer: Rinku::System {
  BFComputer(std::string const &filename, double frequency = 1);
};


#endif // BFCPU_H
