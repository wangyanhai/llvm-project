#include "WangARM.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineJumpTableInfo.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetOpcodes.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/Function.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDwarf.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <utility>
#include <vector>
#include "WangARMFrameLowering.h"
#include "WangARMSubtarget.h"


#define DEBUG_TYPE "wangarm-frame-lowering"

namespace llvm {

//具体化ADD/SUB-stack操作的偏移量。
//如果偏移量作为立即数放入指令，则返回零，或者返回实现偏移量的寄存器编号。
static unsigned materializeOffset(MachineFunction &MF, MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator MBBI,
                                  unsigned Offset) {
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  DebugLoc dl = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();
  const uint64_t MaxSubImm = 0xfff;  //最大12个bit
  if (Offset <= MaxSubImm) {
    // 堆栈偏移量适合ADD/SUB指令。
    return 0;
  } else {
    // 堆栈偏移量不适合ADD/SUB指令。使用MOVLO/MOVHI具体化偏移量。
    unsigned OffsetReg = WangARM::R4;
    unsigned OffsetLo = (unsigned)(Offset & 0xffff);
    unsigned OffsetHi = (unsigned)((Offset & 0xffff0000) >> 16);
    BuildMI(MBB, MBBI, dl, TII.get(WangARM::MOVLOi16), OffsetReg)
        .addImm(OffsetLo)
        .setMIFlag(MachineInstr::FrameSetup);
    if (OffsetHi) {
      BuildMI(MBB, MBBI, dl, TII.get(WangARM::MOVHIi16), OffsetReg)
          .addReg(OffsetReg)
          .addImm(OffsetHi)
          .setMIFlag(MachineInstr::FrameSetup);
    }
    return OffsetReg;
    //return 0; //暂时用0替代
  }
}

static uint64_t RoundUpToAlignment(uint64_t StackSize, uint64_t StackAlign) {
  uint64_t yu = StackSize % StackAlign;
  if (0 == yu){
    return StackSize;
  }

  return StackSize + StackAlign - yu;
}

	bool WangARMFrameLowering::hasFP(const MachineFunction &MF) const {
		return MF.getTarget().Options.DisableFramePointerElim(MF) || MF.getFrameInfo().hasVarSizedObjects();
	}

	WangARMFrameLowering::WangARMFrameLowering(const WangARMSubtarget &sti)
    : TargetFrameLowering(StackGrowsDown, 4, 0, 4),
      STI(sti) {}

	void WangARMFrameLowering::emitPrologue(MachineFunction &MF,MachineBasicBlock &MBB) const {
          // Compute the stack size, to determine if we need a prologue at all.
          const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
          MachineBasicBlock::iterator MBBI = MBB.begin();
          DebugLoc dl = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();
          uint64_t StackSize = computeStackSize(MF);
          if (!StackSize) {
            return;
          }

          // Adjust the stack pointer.
          unsigned StackReg = WangARM::SP;
          unsigned OffsetReg =
              materializeOffset(MF, MBB, MBBI, (unsigned)StackSize);
          if (OffsetReg) {
            BuildMI(MBB, MBBI, dl, TII.get(WangARM::SUBrr), StackReg)
                .addReg(StackReg)
                .addReg(OffsetReg)
                .setMIFlag(MachineInstr::FrameSetup);
          } else {
            BuildMI(MBB, MBBI, dl, TII.get(WangARM::SUBri), StackReg)
                .addReg(StackReg)
                .addImm(StackSize)
                .setMIFlag(MachineInstr::FrameSetup);
          }
        }

        void WangARMFrameLowering::emitEpilogue(MachineFunction &MF,
                                            MachineBasicBlock &MBB) const {
          // Compute the stack size, to determine if we need an epilogue at all.
          const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
          MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
          DebugLoc dl = MBBI->getDebugLoc();
          uint64_t StackSize = computeStackSize(MF);
          if (!StackSize) {
            return;
          }

          // Restore the stack pointer to what it was at the beginning of the
          // function.
          unsigned StackReg = WangARM::SP;
          unsigned OffsetReg =
              materializeOffset(MF, MBB, MBBI, (unsigned)StackSize);
          if (OffsetReg) {
            BuildMI(MBB, MBBI, dl, TII.get(WangARM::ADDrr), StackReg)
                .addReg(StackReg)
                .addReg(OffsetReg)
                .setMIFlag(MachineInstr::FrameSetup);
          } else {
            BuildMI(MBB, MBBI, dl, TII.get(WangARM::ADDri), StackReg)
                .addReg(StackReg)
                .addImm(StackSize)
                .setMIFlag(MachineInstr::FrameSetup);
          }
        }

		uint64_t WangARMFrameLowering::computeStackSize(MachineFunction &MF) const {
          MachineFrameInfo &MFI = MF.getFrameInfo();
          uint64_t StackSize = MFI.getStackSize();
          unsigned StackAlign = 4;
          if (StackAlign > 0) {
            StackSize = RoundUpToAlignment(StackSize, StackAlign);
          }
          return StackSize;
        }
 }