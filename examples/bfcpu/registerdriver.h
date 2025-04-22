INPUT(RD_RS0, 1);
INPUT(RD_RS1, 1);
INPUT(RD_RS2, 1);
INPUT(RD_INC, 1);
INPUT(RD_DEC, 1);

OUTPUT(RD_INC_D, 1);
OUTPUT(RD_DEC_D, 1);
OUTPUT(RD_INC_DP, 1);
OUTPUT(RD_DEC_DP, 1);
OUTPUT(RD_INC_IP, 1);
OUTPUT(RD_DEC_IP, 1);
OUTPUT(RD_INC_SP, 1);
OUTPUT(RD_DEC_SP, 1);
OUTPUT(RD_INC_LS, 1);
OUTPUT(RD_DEC_LS, 1);

SIGNAL_LIST(RegisterDriverInputs,
	    RD_RS0,
	    RD_RS1,
	    RD_RS2,
	    RD_INC,
	    RD_DEC);

SIGNAL_LIST(RegisterDriverOutputs,
	    RD_INC_D,
	    RD_DEC_D,
	    RD_INC_DP,
	    RD_DEC_DP,
	    RD_INC_IP,
	    RD_DEC_IP,
	    RD_INC_SP,
	    RD_DEC_SP,
	    RD_INC_LS,
	    RD_DEC_LS);
	    

class RegisterDriver: MODULE(RegisterDriverInputs, RegisterDriverOutputs) {

public:
  UPDATE() {
    bool inc = GET_INPUT(RD_INC);
    bool dec = GET_INPUT(RD_DEC);;
    assert(!(inc && dec) && "INC/DEC active at the same time");

    size_t registerIndex = 0;
    registerIndex |= (GET_INPUT(RD_RS0) << 0);
    registerIndex |= (GET_INPUT(RD_RS1) << 1);
    registerIndex |= (GET_INPUT(RD_RS2) << 2);

    SET_OUTPUT(RD_INC_D, inc && registerIndex == 1);
    SET_OUTPUT(RD_DEC_D, dec && registerIndex == 1);

    SET_OUTPUT(RD_INC_DP, inc && registerIndex == 2);
    SET_OUTPUT(RD_DEC_DP, dec && registerIndex == 2);

    SET_OUTPUT(RD_INC_SP, inc && registerIndex == 3);
    SET_OUTPUT(RD_DEC_SP, dec && registerIndex == 3);

    SET_OUTPUT(RD_INC_IP, inc && registerIndex == 4);
    SET_OUTPUT(RD_DEC_IP, dec && registerIndex == 4);

    SET_OUTPUT(RD_INC_LS, inc && registerIndex == 5);
    SET_OUTPUT(RD_DEC_LS, dec && registerIndex == 5);
  }
};
