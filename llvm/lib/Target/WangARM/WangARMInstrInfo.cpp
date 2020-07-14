#include "WangARMInstrInfo.h"
#include "WangARM.h"

#define GET_INSTRINFO_CTOR_DTOR
#define GET_INSTRINFO_MC_DESC
#include "WangARMGenInstrInfo.inc"

namespace llvm {

WangARMInstrInfo::WangARMInstrInfo(const WangARMSubtarget &STI):
    WangARMGenInstrInfo(),
    Subtarget(STI) {
		
	}

}