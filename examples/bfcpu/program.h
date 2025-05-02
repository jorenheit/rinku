
INPUT(PROG_ADDR_IN, 16)
OUTPUT(PROG_CMD_OUT, 4)
OUTPUT(PROG_EXIT, 1);

SIGNAL_LIST(ProgramInputSignals,
	    PROG_ADDR_IN)

SIGNAL_LIST(ProgramOutputSignals,
	    PROG_CMD_OUT,
	    PROG_EXIT);

class Program: MODULE(ProgramInputSignals, ProgramOutputSignals) {

  std::vector<unsigned char> data;
  
public:
  Program(std::string const &filename){
    std::ifstream file(filename);
    assert(file && "Could not open file");

    std::copy(std::istreambuf_iterator<char>(file),
	      std::istreambuf_iterator<char>{},
	      std::back_inserter(data));
  }

  RESET() {
    SET_OUTPUT(PROG_EXIT, 0);
  }
  
  UPDATE() {
    assert(data.size() > 0 && "no program loaded");
    signal_t address = GET_INPUT(PROG_ADDR_IN);
    bool const endOfProgram =
      (address >> 1) >= data.size() ||
      ((address >> 1) == (data.size() - 1) && (address & 1));
    
    if (endOfProgram) {
      SET_OUTPUT(PROG_EXIT, 1);
      return;
    }
    signal_t byte = data[address >> 1];
    signal_t nibble = ((address & 1) ? (byte >> 4) : byte) & 0x0f;

    SET_OUTPUT(PROG_CMD_OUT, nibble);
  }
};

