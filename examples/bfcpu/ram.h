
INPUT(OE_RAM, 1);
INPUT(WE_RAM, 1);
INPUT(RAM_DATA_IN, 16);
INPUT(RAM_ADDR_IN, 16);

OUTPUT(RAM_DATA_OUT, 16);

SIGNAL_LIST(RamInputs,
	    OE_RAM,
	    WE_RAM,
	    RAM_DATA_IN,
	    RAM_ADDR_IN);

SIGNAL_LIST(RamOutputs,
	    RAM_DATA_OUT);

template <size_t N>
class RAM: MODULE(RamInputs, RamOutputs) {
  uint16_t data[N];
  size_t address;

public:
  ON_CLOCK_FALLING() {
    if (GET_INPUT(WE_RAM)) {
      data[address] = GET_INPUT(RAM_DATA_IN);
    }
  }

  UPDATE() {
    address = GET_INPUT(RAM_ADDR_IN);
    SET_OUTPUT(RAM_DATA_OUT, GET_INPUT(OE_RAM) ? data[address] : 0);
  }
};
