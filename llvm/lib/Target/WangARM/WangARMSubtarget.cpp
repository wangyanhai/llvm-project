#include "WangARMSubtarget.h"
#include "llvm/Support/TargetRegistry.h"

#define DEBUG_TYPE "wangarm-subtarget"

#define GET_SUBTARGETINFO_ENUM
#include "WangARMGenSubtargetInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#define GET_SUBTARGETINFO_CTOR
#include "WangARMGenSubtargetInfo.inc"
#include "WangARMTargetMachine.h"


namespace llvm {

	WangARMSubtarget::WangARMSubtarget(const Triple &TT, StringRef CPU,
                                   StringRef FS, const WangARMTargetMachine &TM)
    : RegisterInfo() ,WangARMGenSubtargetInfo(TT, CPU, FS), FrameLowering(*this),
      TLInfo(TM, *this), InstrInfo(*this){

	  }

}