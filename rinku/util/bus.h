#ifndef RINKU_BUS_H
#define RINKU_BUS_H

#include "../rinku.h"

namespace Rinku {
  namespace Util {
    RINKU_INPUT(BUS_DATA_IN, 64);
    RINKU_OUTPUT(BUS_DATA_OUT, 64);

    RINKU_SIGNAL_LIST(BusInputs,
		      BUS_DATA_IN);

    RINKU_SIGNAL_LIST(BusOutputs,
		      BUS_DATA_OUT);
	    
    class Bus: RINKU_MODULE(BusInputs, BusOutputs) {
    public:
      virtual void update() override {
	size_t const data = getInput<BUS_DATA_IN>();
	setOutput<BUS_DATA_OUT>(data);
      }
    };
    
  } // namespace Util
} // namespace Rinku

#endif // BUS_H
