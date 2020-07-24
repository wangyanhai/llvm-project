#include "WangARMISelLowering.h"
#include "WangARM.h"
#include "WangARMMachineFunctionInfo.h"
#include "WangARMRegisterInfo.h"
#include "WangARMSubtarget.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/Triple.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/ISDOpcodes.h"
#include "llvm/CodeGen/IntrinsicLowering.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineJumpTableInfo.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RuntimeLibcalls.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetLowering.h"
#include "llvm/CodeGen/TargetOpcodes.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalAlias.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/User.h"
#include "llvm/IR/Value.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/MC/MCInstrItineraries.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSchedule.h"
#include "llvm/Support/AtomicOrdering.h"
#include "llvm/Support/BranchProbability.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/KnownBits.h"
#include "llvm/Support/MachineValueType.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <limits>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using namespace llvm;

#include "WangARMGenCallingConv.inc"

namespace llvm {

WangARMTargetLowering::WangARMTargetLowering(const TargetMachine &TM,
                                             const WangARMSubtarget &STI)
    : TargetLowering(TM), Subtarget(&STI) {
  addRegisterClass(MVT::i32, &WangARM::GRRegsRegClass);
  computeRegisterProperties(Subtarget->getRegisterInfo());

  setStackPointerRegisterToSaveRestore(WangARM::SP);

  setSchedulingPreference(Sched::Source);

  // Nodes that require custom lowering
  setOperationAction(ISD::GlobalAddress, MVT::i32, Custom);
}

// The APCS parameter registers.
static const MCPhysReg GPRArgRegs[] = {WangARM::R0, WangARM::R1, WangARM::R2,
                                       WangARM::R3};

SDValue WangARMTargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &dl,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  MachineFunction &MF = DAG.getMachineFunction();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();

  assert(!isVarArg && "VarArg not supported");

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), ArgLocs,
                 *DAG.getContext());

  CCInfo.AnalyzeFormalArguments(Ins, CC_WangARM);

  for (auto &VA : ArgLocs) {
    if (VA.isRegLoc()) {
      // Arguments passed in registers
      EVT RegVT = VA.getLocVT();
      // Transform the arguments in physical registers into virtual ones.
      unsigned VReg = MF.addLiveIn(VA.getLocReg(), &WangARM::GRRegsRegClass);

      SDValue ArgIn = DAG.getCopyFromReg(Chain, dl, VReg, RegVT);

      InVals.push_back(ArgIn);
      continue;
    }

    assert(VA.isMemLoc() &&
           "Can only pass arguments as either registers or via the stack");

    const unsigned Offset = VA.getLocMemOffset();

    const int FI = MF.getFrameInfo().CreateFixedObject(4, Offset, true);
    SDValue FIPtr = DAG.getFrameIndex(
        FI, getPointerTy(getTargetMachine().createDataLayout()));

    assert(VA.getValVT() == MVT::i32 &&
           "Only support passing arguments as i32");
    SDValue Load =
        DAG.getLoad(VA.getValVT(), dl, Chain, FIPtr, MachinePointerInfo());

    InVals.push_back(Load);
  }

  return Chain;

#if 0
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  WangARMFunctionInfo *AFI = MF.getInfo<WangARMFunctionInfo>();

  //为所有传入参数指定位置。
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), ArgLocs,
                 *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, CCAssignFnForCall(CallConv, isVarArg));

  SmallVector<SDValue, 16> ArgValues;
  SDValue ArgValue;
  Function::const_arg_iterator CurOrigArg = MF.getFunction().arg_begin();
  unsigned CurArgIdx = 0;

  //最初ArgRegsSaveSize为零。然后我们每次遇到byval参数时都会增加这个值。
  //对于varargs函数，我们还增加了这个值。
  AFI->setArgRegsSaveSize(0);

  //计算需要分配给存储传入的byval和variac参数的堆栈空间量注册。
  //我们在分配第一个byval或variadic参数之前需要知道这一点，
  //因为它们将被分配到CFA（Canonical Frame Address规范帧地址，函数入口的堆栈指针）下面的堆栈槽。
  unsigned ArgRegBegin = WangARM::R4;
  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    if (CCInfo.getInRegsParamsProcessed() >= CCInfo.getInRegsParamsCount())
      break;

    CCValAssign &VA = ArgLocs[i];
    unsigned Index = VA.getValNo();
    ISD::ArgFlagsTy Flags = Ins[Index].Flags;
    if (!Flags.isByVal())
      continue;

    assert(VA.isMemLoc() && "unexpected byval pointer in reg");
    unsigned RBegin, REnd;
    CCInfo.getInRegsParamInfo(CCInfo.getInRegsParamsProcessed(), RBegin, REnd);
    ArgRegBegin = std::min(ArgRegBegin, RBegin);

    CCInfo.nextInRegsParam();
  }
  CCInfo.rewindByValRegsInfo();

  int lastInsIndex = -1;
  if (isVarArg && MFI.hasVAStart()) {
    unsigned RegIdx = CCInfo.getFirstUnallocated(GPRArgRegs);
    if (RegIdx != array_lengthof(GPRArgRegs))
      ArgRegBegin = std::min(ArgRegBegin, (unsigned)GPRArgRegs[RegIdx]);
  }

  unsigned TotalArgRegsSaveSize = 4 * (WangARM::R4 - ArgRegBegin);
  AFI->setArgRegsSaveSize(TotalArgRegsSaveSize);
  auto PtrVT = getPointerTy(DAG.getDataLayout());

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    if (Ins[VA.getValNo()].isOrigArg()) {
      std::advance(CurOrigArg,
                   Ins[VA.getValNo()].getOrigArgIndex() - CurArgIdx);
      CurArgIdx = Ins[VA.getValNo()].getOrigArgIndex();
    }
    // Arguments stored in registers.
    if (VA.isRegLoc()) {
      EVT RegVT = VA.getLocVT();

      if (VA.needsCustom()) {
        assert(false &&"f64 and vector types are not supported");
        // f64 and vector types are split up into multiple registers or
        // combinations of registers and stack slots.
        //if (VA.getLocVT() == MVT::v2f64) {
        //  SDValue ArgValue1 =
        //      GetF64FormalArgument(VA, ArgLocs[++i], Chain, DAG, dl);
        //  VA = ArgLocs[++i]; // skip ahead to next loc
        //  SDValue ArgValue2;
        //  if (VA.isMemLoc()) {
        //    int FI = MFI.CreateFixedObject(8, VA.getLocMemOffset(), true);
        //    SDValue FIN = DAG.getFrameIndex(FI, PtrVT);
        //    ArgValue2 = DAG.getLoad(MVT::f64, dl, Chain, FIN,
        //                            MachinePointerInfo::getFixedStack(
        //                                DAG.getMachineFunction(), FI));
        //  } else {
        //    ArgValue2 = GetF64FormalArgument(VA, ArgLocs[++i], Chain, DAG, dl);
        //  }
        //  ArgValue = DAG.getNode(ISD::UNDEF, dl, MVT::v2f64);
        //  ArgValue =
        //      DAG.getNode(ISD::INSERT_VECTOR_ELT, dl, MVT::v2f64, ArgValue,
        //                  ArgValue1, DAG.getIntPtrConstant(0, dl));
        //  ArgValue =
        //      DAG.getNode(ISD::INSERT_VECTOR_ELT, dl, MVT::v2f64, ArgValue,
        //                  ArgValue2, DAG.getIntPtrConstant(1, dl));
        //} else
        //  ArgValue = GetF64FormalArgument(VA, ArgLocs[++i], Chain, DAG, dl);
      } else {
        const TargetRegisterClass *RC;

		if (RegVT == MVT::i32)
          RC = &WangARM::GRRegsRegClass;
        else
          llvm_unreachable("RegVT not supported by FORMAL_ARGUMENTS Lowering");

        // Transform the arguments in physical registers into virtual ones.
        unsigned Reg = MF.addLiveIn(VA.getLocReg(), RC);
        ArgValue = DAG.getCopyFromReg(Chain, dl, Reg, RegVT);
      }

      // If this is an 8 or 16-bit value, it is really passed promoted
      // to 32 bits.  Insert an assert[sz]ext to capture this, then
      // truncate to the right size.
      switch (VA.getLocInfo()) {
      default:
        llvm_unreachable("Unknown loc info!");
      case CCValAssign::Full:
        break;
      case CCValAssign::BCvt:
        ArgValue = DAG.getNode(ISD::BITCAST, dl, VA.getValVT(), ArgValue);
        break;
      case CCValAssign::SExt:
        ArgValue = DAG.getNode(ISD::AssertSext, dl, RegVT, ArgValue,
                               DAG.getValueType(VA.getValVT()));
        ArgValue = DAG.getNode(ISD::TRUNCATE, dl, VA.getValVT(), ArgValue);
        break;
      case CCValAssign::ZExt:
        ArgValue = DAG.getNode(ISD::AssertZext, dl, RegVT, ArgValue,
                               DAG.getValueType(VA.getValVT()));
        ArgValue = DAG.getNode(ISD::TRUNCATE, dl, VA.getValVT(), ArgValue);
        break;
      }

      InVals.push_back(ArgValue);
    } else { // VA.isRegLoc()
      // sanity check
      assert(VA.isMemLoc());
      assert(VA.getValVT() != MVT::i64 && "i64 should already be lowered");

      int index = VA.getValNo();

      // Some Ins[] entries become multiple ArgLoc[] entries.
      // Process them only once.
      if (index != lastInsIndex) {
        ISD::ArgFlagsTy Flags = Ins[index].Flags;
        // FIXME: For now, all byval parameter objects are marked mutable.
        // This can be changed with more analysis.
        // In case of tail call optimization mark all arguments mutable.
        // Since they could be overwritten by lowering of arguments in case of
        // a tail call.
        if (Flags.isByVal()) {
          assert(Ins[index].isOrigArg() &&
                 "Byval arguments cannot be implicit");
          unsigned CurByValIndex = CCInfo.getInRegsParamsProcessed();

          int FrameIndex = StoreByValRegs(CCInfo, DAG, dl, Chain, &*CurOrigArg,
                                          CurByValIndex, VA.getLocMemOffset(),
                                          Flags.getByValSize());
          InVals.push_back(DAG.getFrameIndex(FrameIndex, PtrVT));
          CCInfo.nextInRegsParam();
        } else {
          unsigned FIOffset = VA.getLocMemOffset();
          int FI = MFI.CreateFixedObject(VA.getLocVT().getSizeInBits() / 8,
                                         FIOffset, true);

          // Create load nodes to retrieve arguments from the stack.
          SDValue FIN = DAG.getFrameIndex(FI, PtrVT);
          InVals.push_back(DAG.getLoad(
              VA.getValVT(), dl, Chain, FIN,
              MachinePointerInfo::getFixedStack(DAG.getMachineFunction(), FI)));
        }
        lastInsIndex = index;
      }
    }
  }

  // varargs
  //if (isVarArg && MFI.hasVAStart())
  //  VarArgStyleRegisters(CCInfo, DAG, dl, Chain, CCInfo.getNextStackOffset(),
  //                       TotalArgRegsSaveSize);

  AFI->setArgumentStackSize(CCInfo.getNextStackOffset());

  return Chain;
#endif
}

CCAssignFn *WangARMTargetLowering::CCAssignFnForCall(CallingConv::ID CC,
                                                     bool isVarArg) const {
  return CCAssignFnForNode(CC, false, isVarArg);
}

CCAssignFn *WangARMTargetLowering::CCAssignFnForReturn(CallingConv::ID CC,
                                                       bool isVarArg) const {
  return CCAssignFnForNode(CC, true, isVarArg);
}

/// CCAssignFnForNode - Selects the correct CCAssignFn for the given
/// CallingConvention.
CCAssignFn *WangARMTargetLowering::CCAssignFnForNode(CallingConv::ID CC,
                                                     bool Return,
                                                     bool isVarArg) const {
  switch (CC) {
  default:
    report_fatal_error("Unsupported calling convention");
  case CallingConv::C:
    return (Return ? RetCC_WangARM : CC_WangARM);
  }
}

int WangARMTargetLowering::StoreByValRegs(CCState &CCInfo, SelectionDAG &DAG,
                                          const SDLoc &dl, SDValue &Chain,
                                          const Value *OrigArg,
                                          unsigned InRegsParamRecordIdx,
                                          int ArgOffset,
                                          unsigned ArgSize) const {
  // Currently, two use-cases possible:
  // Case #1. Non-var-args function, and we meet first byval parameter.
  //          Setup first unallocated register as first byval register;
  //          eat all remained registers
  //          (these two actions are performed by HandleByVal method).
  //          Then, here, we initialize stack frame with
  //          "store-reg" instructions.
  // Case #2. Var-args function, that doesn't contain byval parameters.
  //          The same: eat all remained unallocated registers,
  //          initialize stack frame.

  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  WangARMFunctionInfo *AFI = MF.getInfo<WangARMFunctionInfo>();
  unsigned RBegin, REnd;
  if (InRegsParamRecordIdx < CCInfo.getInRegsParamsCount()) {
    CCInfo.getInRegsParamInfo(InRegsParamRecordIdx, RBegin, REnd);
  } else {
    unsigned RBeginIdx = CCInfo.getFirstUnallocated(GPRArgRegs);
    RBegin = RBeginIdx == 4 ? (unsigned)WangARM::R4 : GPRArgRegs[RBeginIdx];
    REnd = WangARM::R4;
  }

  if (REnd != RBegin)
    ArgOffset = -4 * (WangARM::R4 - RBegin);

  auto PtrVT = getPointerTy(DAG.getDataLayout());
  int FrameIndex = MFI.CreateFixedObject(ArgSize, ArgOffset, false);
  SDValue FIN = DAG.getFrameIndex(FrameIndex, PtrVT);

  SmallVector<SDValue, 4> MemOps;
  const TargetRegisterClass *RC = &WangARM::GRRegsRegClass;

  for (unsigned Reg = RBegin, i = 0; Reg < REnd; ++Reg, ++i) {
    unsigned VReg = MF.addLiveIn(Reg, RC);
    SDValue Val = DAG.getCopyFromReg(Chain, dl, VReg, MVT::i32);
    SDValue Store = DAG.getStore(Val.getValue(1), dl, Val, FIN,
                                 MachinePointerInfo(OrigArg, 4 * i));
    MemOps.push_back(Store);
    FIN = DAG.getNode(ISD::ADD, dl, PtrVT, FIN, DAG.getConstant(4, dl, PtrVT));
  }

  if (!MemOps.empty())
    Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other, MemOps);
  return FrameIndex;
}

// Setup stack frame, the va_list pointer will start from.
void WangARMTargetLowering::VarArgStyleRegisters(
    CCState &CCInfo, SelectionDAG &DAG, const SDLoc &dl, SDValue &Chain,
    unsigned ArgOffset, unsigned TotalArgRegsSaveSize,
    bool ForceMutable) const {
  MachineFunction &MF = DAG.getMachineFunction();
  WangARMFunctionInfo *AFI = MF.getInfo<WangARMFunctionInfo>();

  // Try to store any remaining integer argument regs
  // to their spots on the stack so that they may be loaded by dereferencing
  // the result of va_next.
  // If there is no regs to be stored, just point address after last
  // argument passed via stack.
  int FrameIndex = StoreByValRegs(
      CCInfo, DAG, dl, Chain, nullptr, CCInfo.getInRegsParamsCount(),
      CCInfo.getNextStackOffset(), std::max(4U, TotalArgRegsSaveSize));
  AFI->setVarArgsFrameIndex(FrameIndex);
}

//===----------------------------------------------------------------------===//
//               Return Value Calling Convention Implementation
//===----------------------------------------------------------------------===//

bool WangARMTargetLowering::CanLowerReturn(
    CallingConv::ID CallConv, MachineFunction &MF, bool isVarArg,
    const SmallVectorImpl<ISD::OutputArg> &Outs, LLVMContext &Context) const {
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, MF, RVLocs, Context);
  if (!CCInfo.CheckReturn(Outs, RetCC_WangARM)) {
    return false;
  }
  if (CCInfo.getNextStackOffset() != 0 && isVarArg) {
    return false;
  }
  return true;
}

SDValue
WangARMTargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                                   bool isVarArg,
                                   const SmallVectorImpl<ISD::OutputArg> &Outs,
                                   const SmallVectorImpl<SDValue> &OutVals,
                                   const SDLoc &dl, SelectionDAG &DAG) const {
  if (isVarArg) {
    report_fatal_error("VarArg not supported");
  }

  // CCValAssign - represent the assignment of
  // the return value to a location
  SmallVector<CCValAssign, 16> RVLocs;

  // CCState - Info about the registers and stack slot.
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());

  CCInfo.AnalyzeReturn(Outs, RetCC_WangARM);

  SDValue Flag;
  SmallVector<SDValue, 4> RetOps(1, Chain);

  // Copy the result values into the output registers.
  for (unsigned i = 0, e = RVLocs.size(); i < e; ++i) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    Chain = DAG.getCopyToReg(Chain, dl, VA.getLocReg(), OutVals[i], Flag);

    Flag = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  RetOps[0] = Chain; // Update chain.

  // Add the flag if we have it.
  if (Flag.getNode()) {
    RetOps.push_back(Flag);
  }

  return DAG.getNode(WangARMISD::RET_FLAG, dl, MVT::Other, RetOps);
}

SDValue WangARMTargetLowering::LowerOperation(SDValue Op,SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  default:
    llvm_unreachable("Unimplemented operand");
  case ISD::GlobalAddress:
    return LowerGlobalAddress(Op, DAG);
  }
}

SDValue WangARMTargetLowering::LowerGlobalAddress(SDValue Op,
                                              SelectionDAG &DAG) const {
  EVT VT = Op.getValueType();
  GlobalAddressSDNode *GlobalAddr = cast<GlobalAddressSDNode>(Op.getNode());
  SDValue TargetAddr =
      DAG.getTargetGlobalAddress(GlobalAddr->getGlobal(), Op, MVT::i32);
  return TargetAddr;
  //return DAG.getNode(WangARMISD::LOAD_SYM, Op, VT, TargetAddr);
}

//===----------------------------------------------------------------------===//
//                  Call Calling Convention Implementation
//===----------------------------------------------------------------------===//

/// WangARM call implementation
SDValue
WangARMTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                                 SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  SDLoc &dl = CLI.DL;
  SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
  SmallVectorImpl<SDValue> &OutVals = CLI.OutVals;
  SmallVectorImpl<ISD::InputArg> &Ins = CLI.Ins;
  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  CallingConv::ID CallConv = CLI.CallConv;
  const bool isVarArg = CLI.IsVarArg;
  MachineFunction &MF = DAG.getMachineFunction();

  CLI.IsTailCall = false;

  if (isVarArg) {
    llvm_unreachable("Unimplemented");
  }

  // Analyze operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), ArgLocs,
                 *DAG.getContext());
  CCInfo.AnalyzeCallOperands(Outs, CC_WangARM);

  // Get the size of the outgoing arguments stack space requirement.
  const unsigned NumBytes = CCInfo.getNextStackOffset();

  // Chain = DAG.getCALLSEQ_START(Chain,
  // DAG.getIntPtrConstant(NumBytes,dl,true), dl);
  Chain = DAG.getCALLSEQ_START(Chain, NumBytes, NumBytes, dl);
  SmallVector<std::pair<unsigned, SDValue>, 8> RegsToPass;
  SmallVector<SDValue, 8> MemOpChains;

  // Walk the register/memloc assignments, inserting copies/loads.
  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    SDValue Arg = OutVals[i];

    // We only handle fully promoted arguments.
    assert(VA.getLocInfo() == CCValAssign::Full && "Unhandled loc info");

    if (VA.isRegLoc()) {
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
      continue;
    }

    assert(VA.isMemLoc() &&
           "Only support passing arguments through registers or via the stack");

    SDValue StackPtr = DAG.getRegister(WangARM::SP, MVT::i32);
    SDValue PtrOff = DAG.getIntPtrConstant(VA.getLocMemOffset(), dl, true);
    PtrOff = DAG.getNode(ISD::ADD, dl, MVT::i32, StackPtr, PtrOff);
    MemOpChains.push_back(
        DAG.getStore(Chain, dl, Arg, PtrOff, MachinePointerInfo()));
  }

  // Emit all stores, make sure they occur before the call.
  if (!MemOpChains.empty()) {
    Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other, MemOpChains);
  }

  // Build a sequence of copy-to-reg nodes chained together with token chain
  // and flag operands which copy the outgoing args into the appropriate regs.
  SDValue InFlag;
  for (auto &Reg : RegsToPass) {
    Chain = DAG.getCopyToReg(Chain, dl, Reg.first, Reg.second, InFlag);
    InFlag = Chain.getValue(1);
  }

  // We only support calling global addresses.
  GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee);
  assert(G && "We only support the calling of global addresses");

  Callee = DAG.getGlobalAddress(
      G->getGlobal(), dl, getPointerTy(getTargetMachine().createDataLayout()),
      0);

  std::vector<SDValue> Ops;
  Ops.push_back(Chain);
  Ops.push_back(Callee);

  // Add argument registers to the end of the list so that they are known live
  // into the call.
  for (auto &Reg : RegsToPass) {
    Ops.push_back(DAG.getRegister(Reg.first, Reg.second.getValueType()));
  }

  // Add a register mask operand representing the call-preserved registers.
  const uint32_t *Mask;
  const TargetRegisterInfo *TRI = Subtarget->getRegisterInfo();
  Mask = TRI->getCallPreservedMask(MF, CallConv);

  assert(Mask && "Missing call preserved mask for calling convention");
  Ops.push_back(DAG.getRegisterMask(Mask));

  if (InFlag.getNode()) {
    Ops.push_back(InFlag);
  }

  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);

  // Returns a chain and a flag for retval copy to use.
  Chain = DAG.getNode(WangARMISD::CALL, dl, NodeTys, Ops);
  InFlag = Chain.getValue(1);

  Chain = DAG.getCALLSEQ_END(Chain, DAG.getIntPtrConstant(NumBytes, dl, true),
                             DAG.getIntPtrConstant(0, dl, true), InFlag, dl);
  if (!Ins.empty()) {
    InFlag = Chain.getValue(1);
  }

  // Handle result values, copying them out of physregs into vregs that we
  // return.
  return LowerCallResult(Chain, InFlag, CallConv, isVarArg, Ins, dl, DAG,
                         InVals);
}

SDValue WangARMTargetLowering::LowerCallResult(
    SDValue Chain, SDValue InGlue, CallingConv::ID CallConv, bool isVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, SDLoc dl, SelectionDAG &DAG,
    SmallVectorImpl<SDValue> &InVals) const {
  assert(!isVarArg && "Unsupported");

  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());

  CCInfo.AnalyzeCallResult(Ins, RetCC_WangARM);

  // Copy all of the result registers out of their specified physreg.
  for (auto &Loc : RVLocs) {
    Chain =
        DAG.getCopyFromReg(Chain, dl, Loc.getLocReg(), Loc.getValVT(), InGlue)
            .getValue(1);
    InGlue = Chain.getValue(2);
    InVals.push_back(Chain.getValue(0));
  }

  return Chain;
}

const char *WangARMTargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch ((WangARMISD::NodeType)Opcode) {
  case WangARMISD::FIRST_NUMBER:
     break;
  case WangARMISD::RET_FLAG:   return "WangARMISD::RET_FLAG";
  case WangARMISD::LOAD_SYM:   return "WangARMISD::LOAD_SYM";
  case WangARMISD::MOVEi32:    return "WangARMISD::MOVEi32";
  case WangARMISD::CALL:       return "WangARMISD::CALL";
  }

  return nullptr;
  }

} // namespace llvm