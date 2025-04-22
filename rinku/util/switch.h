#ifndef RINKU_SWITCH_H
#define RINKU_SWITCH_H

#include "../rinku.h"

namespace Rinku {
  namespace Util {

    // Binary Switch
    RINKU_OUTPUT(SWITCH_OUT, 1);
    RINKU_SIGNAL_LIST(SwitchOutputs, SWITCH_OUT);

    struct Switch: RINKU_MODULE(SwitchOutputs) {
      bool state = false;
    public:
      void set(bool val) {
	state = val;
      }
  
      void toggle() {
	state = !state;
      }
  
      RINKU_UPDATE() {
	setOutput<SWITCH_OUT>(state);
      }
    };
    
  } // namespace Util
} // namespace Rinku


#endif // RINKU_SWITCH_H
