#include <fstream>
#include "bfcpu.h"

// Include Mugen-generated microcode 
#include "microcode/microcode.h"

#include "../../rinku/util/splitter.h"
#include "../../rinku/util/joiner.h"
#include "../../rinku/util/bus.h"

using namespace Rinku;
using namespace Rinku::Util; // for splitter, joiner and bus

// Custom modules
#include "countingregister.h"
#include "registerdriver.h"
#include "ram.h"
#include "screen.h"
#include "program.h"
#include "controlunit.h" 

// Wire up the system
BFComputer::BFComputer(std::string const &filename, double frequency):
  System(frequency)
{

  auto& dataBus	   = System::addModule<Bus>("databus");
  auto& addressBus = System::addModule<Bus>("addressbus");
  auto& faReg	   = System::addModule<CountingRegister<4>>("fa");
  auto& fbReg	   = System::addModule<CountingRegister<4>>("fb");
  auto& ccReg	   = System::addModule<CountingRegister<4>>("cc");
  auto& dReg	   = System::addModule<CountingRegister<8>>("d");
  auto& iReg	   = System::addModule<CountingRegister<8>>("i");
  auto& spReg	   = System::addModule<CountingRegister<8>>("sp");
  auto& lsReg	   = System::addModule<CountingRegister<8>>("ls");
  auto& dpReg	   = System::addModule<CountingRegister<16>>("dp");
  auto& ipReg	   = System::addModule<CountingRegister<16>>("ip");
  auto& rd	   = System::addModule<RegisterDriver>("rd");
  auto& ram	   = System::addModule<RAM<8*1024>>("ram");
  auto& cu	   = System::addModule<ControlUnit>("cu");
  auto& prog	   = System::addModule<Program>("prog", filename);
  auto& scr	   = System::addModule<Screen>("scr");
  auto& faJoin	   = System::addModule<Joiner<4>>();
  auto& fbJoin	   = System::addModule<Joiner<4>>();
  auto& faSplit	   = System::addModule<Splitter<4>>(); 

  // Connect databus inputs
  CONNECT_MOD(dataBus, BUS_DATA_IN, ram, RAM_DATA_OUT);
  CONNECT_MOD(dataBus, BUS_DATA_IN, dReg, CR_DATA_OUT);
  CONNECT_MOD(dataBus, BUS_DATA_IN, ipReg, CR_DATA_OUT);  

  // Connect addressBus inputs
  CONNECT_MOD(addressBus, BUS_DATA_IN, dpReg, CR_DATA_OUT);
  CONNECT_MOD(addressBus, BUS_DATA_IN, spReg, CR_DATA_OUT);

  // Connect RAM
  CONNECT_MOD(ram, OE_RAM, cu, CU_OE_RAM);
  CONNECT_MOD(ram, WE_RAM, cu, CU_WE_RAM);
  CONNECT_MOD(ram, RAM_DATA_IN, dataBus, BUS_DATA_OUT);
  CONNECT_MOD(ram, RAM_ADDR_IN, addressBus, BUS_DATA_OUT);  

  // Connect Program Module
  CONNECT_MOD(prog, PROG_ADDR_IN, ipReg, CR_DATA_OUT_ALWAYS);  
  
  // Connect D Register
  CONNECT_MOD(dReg, CR_INC, rd, RD_INC_D);
  CONNECT_MOD(dReg, CR_DEC, rd, RD_DEC_D);
  CONNECT_MOD(dReg, CR_EN, cu, CU_EN_D);
  CONNECT_MOD(dReg, CR_LD, cu, CU_LD_D);
  CONNECT_MOD(dReg, CR_DATA_IN, dataBus, BUS_DATA_OUT);  

  // Connect I Register
  CONNECT_CONST(iReg, CR_INC, 0);
  CONNECT_CONST(iReg, CR_DEC, 0);
  CONNECT_CONST(iReg, CR_EN, 1);
  CONNECT_MOD(iReg, CR_LD, cu, CU_LD_FB);
  CONNECT_MOD(iReg, CR_DATA_IN, prog, PROG_CMD_OUT);  

  // Connect DP Register
  CONNECT_MOD(dpReg, CR_INC, rd, RD_INC_DP);
  CONNECT_MOD(dpReg, CR_DEC, rd, RD_DEC_DP);
  CONNECT_MOD(dpReg, CR_EN, cu, NOT(CU_EN_SP));
  CONNECT_MOD(dpReg, CR_LD, cu, CU_DPR);
  CONNECT_CONST(dpReg, CR_DATA_IN, 0x0100);

  // Connect SP Register
  CONNECT_MOD(spReg, CR_INC, rd, RD_INC_SP);
  CONNECT_MOD(spReg, CR_DEC, rd, RD_DEC_SP);
  CONNECT_MOD(spReg, CR_EN, cu, CU_EN_SP);
  CONNECT_CONST(spReg, CR_LD, 0);

  // Connect IP Register
  CONNECT_MOD(ipReg, CR_INC, rd, RD_INC_IP);
  CONNECT_CONST(ipReg, CR_DEC, 0);
  CONNECT_MOD(ipReg, CR_EN, cu, CU_EN_IP);
  CONNECT_MOD(ipReg, CR_LD, cu, CU_LD_IP);
  CONNECT_MOD(ipReg, CR_DATA_IN, dataBus, BUS_DATA_OUT);

  // Connect LS Register
  CONNECT_MOD(lsReg, CR_INC, rd, RD_INC_LS);
  CONNECT_MOD(lsReg, CR_DEC, rd, RD_DEC_LS);
  CONNECT_CONST(lsReg, CR_EN, 0);
  CONNECT_CONST(lsReg, CR_LD, 0);

  // Connect Register Driver
  CONNECT_MOD(rd, RD_INC, cu, CU_INC);
  CONNECT_MOD(rd, RD_DEC, cu, CU_DEC);
  CONNECT_MOD(rd, RD_RS0, cu, CU_RS0);
  CONNECT_MOD(rd, RD_RS1, cu, CU_RS1);
  CONNECT_MOD(rd, RD_RS2, cu, CU_RS2);

  // Connect FA Register (use joiner on inputs and splitter on output)
  CONNECT_CONST(faReg, CR_INC, 0);
  CONNECT_CONST(faReg, CR_DEC, 0);
  CONNECT_CONST(faReg, CR_EN, 1);
  CONNECT_MOD(faReg, CR_LD, cu, CU_LD_FA);

  CONNECT_CONST(faJoin, JOINER_IN_0, 0);
  CONNECT_CONST(faJoin, JOINER_IN_1, 0);
  CONNECT_MOD(faJoin, JOINER_IN_2, cu, CU_A);
  CONNECT_MOD(faJoin, JOINER_IN_3, cu, CU_V);
  CONNECT_MOD(faReg, CR_DATA_IN, faJoin, JOINER_OUT);
  CONNECT_MOD(faSplit, SPLITTER_IN, faReg, CR_DATA_OUT);

  // Connect FB Register (use joiner)
  CONNECT_MOD(fbReg, CR_LD, cu, CU_LD_FB);
  CONNECT_CONST(fbReg, CR_INC, 0);
  CONNECT_CONST(fbReg, CR_DEC, 0);
  CONNECT_CONST(fbReg, CR_EN, 1);

  CONNECT_MOD(fbJoin, JOINER_IN_0, dReg, CR_Z);
  CONNECT_MOD(fbJoin, JOINER_IN_1, lsReg, NOT(CR_Z));
  CONNECT_MOD(fbJoin, JOINER_IN_2, faSplit, SPLITTER_OUT_2);
  CONNECT_MOD(fbJoin, JOINER_IN_3, faSplit, SPLITTER_OUT_3);
  CONNECT_MOD(fbReg, CR_DATA_IN, fbJoin, JOINER_OUT);

  // Connect CC Register
  CONNECT_MOD(ccReg, CR_LD, cu, CU_CR);
  CONNECT_CONST(ccReg, CR_INC, 1);
  CONNECT_CONST(ccReg, CR_DEC, 0);
  CONNECT_CONST(ccReg, CR_EN, 1);
  CONNECT_CONST(ccReg, CR_DATA_IN, 0);

  // Connect Screen
  CONNECT_MOD(scr, SCR_EN, cu, CU_EN_SCR);
  CONNECT_MOD(scr, SCR_DATA_IN, dataBus, BUS_DATA_OUT);

  // Connect CU
  CONNECT_MOD(cu, CU_F_IN, fbReg, CR_DATA_OUT);
  CONNECT_MOD(cu, CU_I_IN, iReg, CR_DATA_OUT);
  CONNECT_MOD(cu, CU_CC_IN, ccReg, CR_DATA_OUT);
  
  // Connect HLT, ERR and EXIT
  connectHalt<CU_HLT>(cu);
  connectError<CU_ERR>(cu);
  connectExit<PROG_EXIT>(prog);

  // Connect scope
  auto& cuScope = addScope("CUScope");
  cuScope.monitor(cu);
  
  // Done -> initialize system
  init();
}
