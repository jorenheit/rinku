#ifndef RINKU_LOGIC
#define RINKU_LOGIC

#include "../rinku.h"

#define RINKU_BINARY_GATE(NAME, PREFIX, OP)				\
  RINKU_INPUT(PREFIX##_IN_A, 1);					\
  RINKU_INPUT(PREFIX##_IN_B, 1);					\
  RINKU_OUTPUT(PREFIX##_OUT, 1);					\
  RINKU_SIGNAL_LIST(NAME##Inputs, PREFIX##_IN_A, PREFIX##_IN_B);	\
  RINKU_SIGNAL_LIST(NAME##Outputs, PREFIX##_OUT);			\
									\
  struct NAME : RINKU_MODULE(NAME##Inputs, NAME##Outputs) {		\
  public:								\
    RINKU_UPDATE() {							\
      bool a = getInput<PREFIX##_IN_A>();				\
      bool b = getInput<PREFIX##_IN_B>();				\
      setOutput<PREFIX##_OUT>(a OP b);					\
    }									\
  };

namespace Rinku {
  namespace Logic {
    
    // Logic gates
    RINKU_BINARY_GATE(And, AND, &&);
    RINKU_BINARY_GATE(Or, OR, ||);
    RINKU_BINARY_GATE(Xor, XOR, ^);

  } // namespace Logic
} // namespace Rinku

#endif // RINKU_LOGIC

 
