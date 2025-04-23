
INPUT(CR_INC, 1);
INPUT(CR_DEC, 1);
INPUT(CR_LD, 1);
INPUT(CR_EN, 1);
INPUT(CR_DATA_IN, 16);

OUTPUT(CR_Z, 1);
OUTPUT(CR_DATA_OUT, 16);
OUTPUT(CR_DATA_OUT_ALWAYS, 16);

SIGNAL_LIST(CountingRegisterInputs,
	    CR_INC,
	    CR_DEC,
	    CR_LD,
	    CR_EN,
	    CR_DATA_IN);


SIGNAL_LIST(CountingRegisterOutputs,
	    CR_Z,
	    CR_DATA_OUT,
	    CR_DATA_OUT_ALWAYS);


template <size_t N>
class CountingRegister: MODULE(CountingRegisterInputs, CountingRegisterOutputs) {

  static constexpr size_t MAX_VALUE = (1 << N);
  size_t const resetValue;
  size_t value;
  bool needsUpdate = true;
  
public:
  explicit CountingRegister(size_t val = 0):
    resetValue(val),
    value(val)
  {}
  
  ON_CLOCK_FALLING() {
    if (GET_INPUT(CR_LD)) {
      value = GET_INPUT(CR_DATA_IN);
      needsUpdate = true;
      return;
    }

    bool const INC = GET_INPUT(CR_INC); 
    bool const DEC = GET_INPUT(CR_DEC); 
    assert(!(INC && DEC) && "INC/DEC active at the same time");
    
    if (INC) {
      value = (value + 1) % MAX_VALUE;
      needsUpdate = true;
    }
    else if (DEC) {
      value = (value - 1 + MAX_VALUE) % MAX_VALUE;
      needsUpdate = true;
    }
  }

  UPDATE() {
    if (!needsUpdate) return;
    
    SET_OUTPUT(CR_Z, (value == 0));
    SET_OUTPUT(CR_DATA_OUT, GET_INPUT(CR_EN) ? value : 0);
    SET_OUTPUT(CR_DATA_OUT_ALWAYS, value);
    needsUpdate = false;
  }

  RESET() {
    value = resetValue;
    needsUpdate = true;
  }
};
