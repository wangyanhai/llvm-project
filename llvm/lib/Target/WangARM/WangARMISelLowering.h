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

namespace WangARMISD {
enum NodeType {
  // Start the numbering where the builtin ops and target ops leave off.
  FIRST_NUMBER = ISD::BUILTIN_OP_END,
  RET_FLAG,
  // This loads the symbol (e.g. global address) into a register.
  LOAD_SYM,
  // This loads a 32-bit immediate into a register.
  MOVEi32,
  CALL,
};
}

class WangARMTargetLowering : public TargetLowering {
public:
	  explicit WangARMTargetLowering(const TargetMachine &TM,const WangARMSubtarget &STI);

      SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                               bool isVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               const SDLoc &dl, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;

	  bool CanLowerReturn(CallingConv::ID CallConv, MachineFunction &MF,
                          bool isVarArg,
                          const SmallVectorImpl<ISD::OutputArg> &Outs,
                          LLVMContext &Context) const override;

	  SDValue LowerReturn(SDValue /*Chain*/, CallingConv::ID /*CallConv*/,
                              bool /*isVarArg*/,
                              const SmallVectorImpl<ISD::OutputArg> & /*Outs*/,
                              const SmallVectorImpl<SDValue> & /*OutVals*/,
                              const SDLoc & /*dl*/,
                              SelectionDAG & /*DAG*/) const override;

	  SDValue LowerCall(TargetLowering::CallLoweringInfo &CLI,SmallVectorImpl<SDValue> &InVals) const override;

	  SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

	  SDValue LowerGlobalAddress(SDValue Op,SelectionDAG &DAG) const ;

	  SDValue LowerCallResult(SDValue Chain, SDValue InGlue,
                          CallingConv::ID CallConv, bool isVarArg,
                          const SmallVectorImpl<ISD::InputArg> &Ins, SDLoc dl,
                          SelectionDAG &DAG,
                          SmallVectorImpl<SDValue> &InVals) const ;

	  CCAssignFn *CCAssignFnForCall(CallingConv::ID CC, bool isVarArg) const;

      CCAssignFn *CCAssignFnForReturn(CallingConv::ID CC, bool isVarArg) const;
      CCAssignFn *CCAssignFnForNode(CallingConv::ID CC, bool Return,bool isVarArg) const;

	  int StoreByValRegs(CCState &CCInfo, SelectionDAG &DAG, const SDLoc &dl, SDValue &Chain,
          const Value *OrigArg, unsigned InRegsParamRecordIdx, int ArgOffset,
          unsigned ArgSize) const;
      void VarArgStyleRegisters(CCState &CCInfo, SelectionDAG &DAG, const SDLoc &dl, SDValue &Chain,
          unsigned ArgOffset, unsigned TotalArgRegsSaveSize,
          bool ForceMutable) const;

	  const char *getTargetNodeName(unsigned Opcode) const override;

  private:
  /// Subtarget - Keep a pointer to the WangARMSubtarget around so that we can
  /// make the right decision when generating code for different targets.
	const WangARMSubtarget *Subtarget;
};

} // namespace llvm

#endif //
