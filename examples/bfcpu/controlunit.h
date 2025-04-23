
INPUT(CU_F_IN, 4);
INPUT(CU_I_IN, 4);
INPUT(CU_CC_IN, 3);

OUTPUT(CU_HLT, 1);
OUTPUT(CU_RS0, 1);
OUTPUT(CU_RS1, 1);
OUTPUT(CU_RS2, 1);
OUTPUT(CU_INC, 1);
OUTPUT(CU_DEC, 1);
OUTPUT(CU_DPR, 1);
OUTPUT(CU_EN_SP, 1);
OUTPUT(CU_OE_RAM, 1);
OUTPUT(CU_WE_RAM, 1);
OUTPUT(CU_EN_SCR, 1);
OUTPUT(CU_EN_KB, 1);
OUTPUT(CU_V, 1);
OUTPUT(CU_A, 1);
OUTPUT(CU_LD_FB, 1);
OUTPUT(CU_LD_FA, 1);
OUTPUT(CU_EN_IP, 1);
OUTPUT(CU_LD_IP, 1);
OUTPUT(CU_EN_D, 1);
OUTPUT(CU_LD_D, 1);
OUTPUT(CU_CR, 1);
OUTPUT(CU_ERR, 1);

SIGNAL_LIST(ControlUnitInputs,
	    CU_F_IN,
	    CU_I_IN,
	    CU_CC_IN);

SIGNAL_LIST(ControlUnitOutputs,
	    CU_HLT, 
	    CU_RS0, 
	    CU_RS1, 
	    CU_RS2, 
	    CU_INC, 
	    CU_DEC, 
	    CU_DPR, 
	    CU_EN_SP, 

	    CU_OE_RAM, 
	    CU_WE_RAM,
	    CU_EN_KB,
	    CU_EN_SCR, 
	    CU_V, 
	    CU_A, 
	    CU_LD_FB, 
	    CU_LD_FA, 

	    CU_EN_IP, 
	    CU_LD_IP, 
	    CU_EN_D, 
	    CU_LD_D, 
	    CU_CR, 
	    CU_ERR);

class ControlUnit: MODULE(ControlUnitInputs, ControlUnitOutputs)
{
  std::vector<size_t> microcodeRom;
  signal_t currentSignals = 0;
  bool needsUpdate = true;
  
public:
  ControlUnit() {
    microcodeRom.resize(Mugen::IMAGE_SIZE);
    for (size_t idx = 0; idx != Mugen::IMAGE_SIZE; ++idx) {
      size_t value = 0;
      for (size_t jdx = 0; jdx != Mugen::N_IMAGES; ++jdx) {
	size_t part = Mugen::images[jdx][idx];
	value |= (part << (8 * jdx));
      }
      microcodeRom[idx] = value;
    }
  }

  ON_CLOCK_RISING() {
    signal_t cycle = GET_INPUT(CU_CC_IN);
    signal_t cmd = GET_INPUT(CU_I_IN);
    signal_t flags = GET_INPUT(CU_F_IN);

    signal_t address =
      ((cycle << 0) & 0b00000000111) |
      ((cmd   << 3) & 0b00001111000) |
      ((flags << 7) & 0b11110000000);

    signal_t newSignals = microcodeRom[address];
    needsUpdate = (newSignals != currentSignals);
    currentSignals = newSignals;
  }

  UPDATE() {
    if (!needsUpdate) return;
    for (size_t idx = 0; idx != Outputs::N; ++idx) {
      SET_OUTPUT_INDEX(idx, (currentSignals >> idx) & 1);
    }
  }

  RESET() {
    currentSignals = 0;
    needsUpdate = true;
  }
};
