#ifndef __LLVM_TARGET_WANGARM_SUBTARGET__
#define __LLVM_TARGET_WANGARM_SUBTARGET__

#include "llvm/ADT/Triple.h"
#include "llvm/CodeGen/GlobalISel/CallLowering.h"
#include "llvm/CodeGen/GlobalISel/InstructionSelector.h"
#include "llvm/CodeGen/GlobalISel/LegalizerInfo.h"
#include "llvm/CodeGen/GlobalISel/RegisterBankInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/MC/MCInstrItineraries.h"
#include "llvm/MC/MCSchedule.h"
#include "llvm/Target/TargetOptions.h"
#include <memory>
#include <string>

#define GET_SUBTARGETINFO_HEADER
#include "WangARMGenSubtargetInfo.inc"
#include "WangARMFrameLowering.h"
#include "WangARMISelLowering.h"
#include "WangARMInstrInfo.h"
#include "WangARMRegisterInfo.h"

namespace llvm {
class GlobalValue;
class StringRef;
class WangARMTargetMachine;

class WangARMSubtarget : public WangARMGenSubtargetInfo {
public:
  WangARMSubtarget(const Triple &TT, StringRef CPU, StringRef FS,const WangARMTargetMachine& TM);

  const TargetFrameLowering *getFrameLowering() const override { return &FrameLowering; }
  const TargetLowering *getTargetLowering() const override { return &TLInfo; }
  const TargetInstrInfo *getInstrInfo() const override { return &InstrInfo; }
  const TargetRegisterInfo *getRegisterInfo() const override { return &RegisterInfo; }

protected:
  WangARMRegisterInfo RegisterInfo;
  WangARMFrameLowering FrameLowering;
  WangARMTargetLowering TLInfo;
  WangARMInstrInfo InstrInfo;
};

} // namespace llvm

#endif