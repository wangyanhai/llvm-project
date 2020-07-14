#ifndef __LLVM_TARGET_WANGARM_FRAMELOWERING__
#define __LLVM_TARGET_WANGARM_FRAMELOWERING__

#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include <vector>

namespace llvm {

class WangARMSubtarget;
class CalleeSavedInfo;
class MachineFunction;

class WangARMFrameLowering : public TargetFrameLowering {
protected:
  const WangARMSubtarget &STI;

public:
  explicit WangARMFrameLowering(const WangARMSubtarget &sti);

  bool hasFP(const MachineFunction &MF) const override;
  /// emitProlog/emitEpilog - These methods insert prolog and epilog code into
  /// the function.
  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  uint64_t  computeStackSize(MachineFunction &MF) const;
};

}

#endif /*__LLVM_TARGET_WANGARM_FRAMELOWERING__*/