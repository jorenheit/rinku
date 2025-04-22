#include <iostream>

#define RINKU_REMOVE_MACRO_PREFIX
#include "../../rinku/rinku.h"

using namespace Rinku;


// ------------ COUNTER --------------
OUTPUT(CNT_OUT, 8);
SIGNAL_LIST(CounterOutputs, CNT_OUT);

class Counter: MODULE(CounterOutputs) {
  int value = 0;
public:
  ON_CLOCK_FALLING() {
    ++value;
  }

  UPDATE() {
    SET_OUTPUT(CNT_OUT, value);
  }
};


// ------------ PRINTER --------------
INPUT(PRN_INDEX_IN, 8);
OUTPUT(PRN_DONE, 1);
OUTPUT(PRN_EXIT_CODE, 8);

SIGNAL_LIST(PrinterInputs, PRN_INDEX_IN);
SIGNAL_LIST(PrinterOutputs, PRN_DONE, PRN_EXIT_CODE);


class Printer: MODULE(PrinterInputs, PrinterOutputs) {
  std::string str;
  signal_t currentIndex;
public:
  Printer(std::string const &s):
    str(s)
  {
    SET_OUTPUT(PRN_EXIT_CODE, 69);
  }

  ON_CLOCK_RISING() {
    currentIndex = GET_INPUT(PRN_INDEX_IN);
    std::cout << str[currentIndex];
  }

  UPDATE() {
    if (currentIndex >= str.length() - 1) {
      SET_OUTPUT(PRN_DONE, 1);
      SET_OUTPUT(PRN_EXIT_CODE, 42);
    }
  }
};

// ------------ SYSTEM --------------

class Hello: public System {
public:
  Hello(std::string const &who) {
    auto& counter = System::addModule<Counter>();
    auto& printer = System::addModule<Printer>("Hello, " + who + "!");
    printer.connect<PRN_INDEX_IN, CNT_OUT>(counter);

    System::connectExit<PRN_DONE>(printer);
    System::connectExitCode<PRN_EXIT_CODE>(printer);
    System::init();
  }

};

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Insufficient arguments: " << argv[0] << " [name]\n";
    return 1;
  }
  
  Hello sys(argv[1]);
  signal_t result = sys.run();
  std::cout << "\nExited with exit code " << result << '\n';
}
