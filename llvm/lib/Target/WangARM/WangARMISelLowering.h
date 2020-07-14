#ifndef __WANGARM_ISELLOWERING_HEADER__
#define __WANGARM_ISELLOWERING_HEADER__

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/ISDOpcodes.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/CodeGen/TargetLowering.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/MachineValueType.h"
#include <utility>

namespace llvm {
class WangARMSubtarget;
class DataLayout;
class FastISel;
class FunctionLoweringInfo;
class GlobalValue;
class InstrItineraryData;
class Instruction;
class MachineBasicBlock;
class MachineInstr;
class SelectionDAG;
class TargetLibraryInfo;
class TargetMachine;
class TargetRegisterInfo;
class VectorType;

class WangARMTargetLowering : public TargetLowering {
public:
  explicit WangARMTargetLowering(const TargetMachine &TM,
                                 const WangARMSubtarget &STI);
private:
  /// Subtarget - Keep a pointer to the WangARMSubtarget around so that we can
  /// make the right decision when generating code for different targets.
  const WangARMSubtarget *Subtarget;
};

} // namespace llvm

#endif //
