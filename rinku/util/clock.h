#ifndef RINKU_CLOCK_H
#define RINKU_CLOCK_H

#include "../rinku.h"

namespace Rinku {
  namespace Util {
    RINKU_OUTPUT(CLK_OUT, 1);
    RINKU_SIGNAL_LIST(ClockOutput, CLK_OUT);

    class Clock: RINKU_MODULE(ClockOutput) {
      bool state = false;
    public:
      RINKU_ON_CLOCK_RISING() {
	state = true;
      }
      RINKU_ON_CLOCK_FALLING() {
	state = false;
      }
      RINKU_UPDATE() {
	RINKU_GUARANTEE_NO_GET_INPUT();
	RINKU_SET_OUTPUT(CLK_OUT, state);
      }
      RINKU_RESET() {
	state = false;
      }
    };
  }
}

#endif // RINKU_CLOCK_H
