#include "WangARMRegisterInfo.h"
#include "WangARM.h"
#include "WangARMFrameLowering.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/VirtRegMap.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include <cassert>
#include <utility>

#define DEBUG_TYPE "wangarm-register-info"

#define GET_REGINFO_TARGET_DESC
#include "WangARMGenRegisterInfo.inc"

namespace llvm {

WangARMRegisterInfo::WangARMRegisterInfo()
    : WangARMGenRegisterInfo(WangARM::LR, 0, 0, WangARM::PC) {
  static int a = 0;
}

const uint16_t *
WangARMRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  static const uint16_t CalleeSavedRegs[] = {WangARM::R4,
                                             WangARM::R5,
                                             WangARM::R6,
                                             WangARM::R7,
                                             WangARM::R8,
                                             WangARM::R9,
                                             0};
  return CalleeSavedRegs;
}

BitVector
WangARMRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());

  Reserved.set(WangARM::SP);
  Reserved.set(WangARM::LR);
  return Reserved;
}

const uint32_t *
WangARMRegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                          CallingConv::ID) const {
  return CC_Save_RegMask;
  //return NULL;
}

bool WangARMRegisterInfo::requiresRegisterScavenging(
    const MachineFunction &MF) const {
  return true;
}

bool WangARMRegisterInfo::trackLivenessAfterRegAlloc(
    const MachineFunction &MF) const {
  return true;
}

bool WangARMRegisterInfo::useFPForScavengingIndex(
    const MachineFunction &MF) const {
  return false;
}

void WangARMRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                              int SPAdj, unsigned FIOperandNum,
                                              RegScavenger *RS) const {
  MachineInstr &MI = *II;
  const MachineFunction &MF = *MI.getParent()->getParent();
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  MachineOperand &FIOp = MI.getOperand(FIOperandNum);
  unsigned FI = FIOp.getIndex();

  // Determine if we can eliminate the index from this kind of
  // instruction.
  unsigned ImmOpIdx = 0;
  switch (MI.getOpcode()) {
  default:
    // Not supported yet.
    return;
  case WangARM::LDR:
  case WangARM::STR:
    ImmOpIdx = FIOperandNum + 1;
    break;
  }

  // FIXME: check the size of offset.
  MachineOperand &ImmOp = MI.getOperand(ImmOpIdx);
  int Offset = MFI.getObjectOffset(FI) + MFI.getStackSize() + ImmOp.getImm();
  FIOp.ChangeToRegister(WangARM::SP, false);
  ImmOp.setImm(Offset);
}

Register
WangARMRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return WangARM::SP;
}

} // namespace llvm