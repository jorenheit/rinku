
INPUT(SCR_EN, 1);
INPUT(SCR_DATA_IN, 8);

SIGNAL_LIST(ScreenInputs,
	    SCR_EN,
	    SCR_DATA_IN);
	    
class Screen: MODULE(ScreenInputs) {
public:
  ON_CLOCK_FALLING() {
    if (GET_INPUT(SCR_EN)) {
      std::cout << (char)GET_INPUT(SCR_DATA_IN);
    }
  }
};
