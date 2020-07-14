#ifndef __LLVM_TARGET_WANGARM_INSTRINFO_HEADER__
#define __LLVM_TARGET_WANGARM_INSTRINFO_HEADER__

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include <array>
#include <cstdint>

#define GET_INSTRINFO_HEADER
#include "WangARMGenInstrInfo.inc"


namespace llvm {

class WangARMSubtarget;

class WangARMInstrInfo : public WangARMGenInstrInfo {
  const WangARMSubtarget &Subtarget;

public:
  explicit WangARMInstrInfo(const WangARMSubtarget &STI);

};

}

#endif