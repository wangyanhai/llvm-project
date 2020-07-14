#ifndef LLVM_LIB_TARGET_WANGARM_H
#define LLVM_LIB_TARGET_WANGARM_H

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/CodeGen.h"
#include <functional>
#include <vector>


namespace llvm {
class WangARMTargetMachine;
class Target;

FunctionPass *createWangARMISelDag(WangARMTargetMachine &TM,
                                   CodeGenOpt::Level OptLevel);
Target& getTheWangARMTarget();

} // namespace llvm


#define GET_REGINFO_ENUM
#include "WangARMGenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#include "WangARMGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "WangARMGenSubtargetInfo.inc"

#endif /*LLVM_LIB_TARGET_WANGARM_H*/