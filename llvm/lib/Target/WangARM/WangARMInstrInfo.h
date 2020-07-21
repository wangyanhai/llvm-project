#ifndef __LLVM_TARGET_WANGARM_INSTRINFO_HEADER__
#define __LLVM_TARGET_WANGARM_INSTRINFO_HEADER__

#include "WangARMRegisterInfo.h"
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
  const WangARMRegisterInfo RI;

public:
  explicit WangARMInstrInfo(const WangARMSubtarget &STI);

  /// getRegisterInfo - TargetInstrInfo is a superset of MRegister info.  As
  /// such, whenever a client has an instance of instruction info, it should
  /// always be able to get register info as well (through this method).
  ///
  const WangARMRegisterInfo &getRegisterInfo() const { return RI; }

  /// isLoadFromStackSlot - If the specified machine instruction is a direct
  /// load from a stack slot, return the virtual or physical register number of
  /// the destination along with the FrameIndex of the loaded stack slot.  If
  /// not, return 0.  This predicate must return 0 if the instruction has
  /// any side effects other than loading from the stack slot.
  unsigned isLoadFromStackSlot(const MachineInstr &MI,int &FrameIndex) const override;

  /// isStoreToStackSlot - If the specified machine instruction is a direct
  /// store to a stack slot, return the virtual or physical register number of
  /// the source reg along with the FrameIndex of the loaded stack slot.  If
  /// not, return 0.  This predicate must return 0 if the instruction has
  /// any side effects other than storing to the stack slot.
  virtual unsigned isStoreToStackSlot(const MachineInstr &MI,int &FrameIndex) const override;

  virtual bool analyzeBranch(MachineBasicBlock &MBB, MachineBasicBlock *&TBB,
                             MachineBasicBlock *&FBB,
                             SmallVectorImpl<MachineOperand> &Cond,
                             bool AllowModify = false) const override;

  virtual unsigned removeBranch(MachineBasicBlock &MBB,
                                int *BytesRemoved = nullptr) const override;

  virtual void copyPhysReg(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MI, const DebugLoc &DL,
                           unsigned DestReg, unsigned SrcReg,
                           bool KillSrc) const override;

  virtual void storeRegToStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
                      unsigned SrcReg, bool isKill, int FrameIndex,
                      const TargetRegisterClass *RC,const TargetRegisterInfo *TRI) const override;

  virtual void loadRegFromStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
                       unsigned DestReg, int FrameIndex,
                       const TargetRegisterClass *RC,const TargetRegisterInfo *TRI) const override;

  virtual bool expandPostRAPseudo(MachineInstr &MI) const override;
};

} // namespace llvm

#endif