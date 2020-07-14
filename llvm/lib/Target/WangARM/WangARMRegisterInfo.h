#ifndef __LLVM_TARGET_WANGARM_REGISTERINFO_HEADER__
#define __LLVM_TARGET_WANGARM_REGISTERINFO_HEADER__

#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/MC/MCRegisterInfo.h"
#include <cstdint>
#define GET_REGINFO_HEADER
#include "WangARMGenRegisterInfo.inc"

namespace llvm {

	class WangARMRegisterInfo : public WangARMGenRegisterInfo {
	public:
	    explicit WangARMRegisterInfo();

		/// Code Generation virtual methods...
		const uint16_t *getCalleeSavedRegs(const MachineFunction *MF = 0) const override;

		const uint32_t *getCallPreservedMask(const MachineFunction &MF,CallingConv::ID) const override;

		BitVector getReservedRegs(const MachineFunction &MF) const override;

		bool requiresRegisterScavenging(const MachineFunction &MF) const override;

		bool trackLivenessAfterRegAlloc(const MachineFunction &MF) const override;

		bool
		useFPForScavengingIndex(const MachineFunction &MF) const override;

		void eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj,
								unsigned FIOperandNum,
								RegScavenger *RS = NULL) const override;

		// Debug information queries.
        Register getFrameRegister(const MachineFunction &MF) const;
    };
 
}


#endif /*__LLVM_TARGET_WANGARM_REGISTERINFO_HEADER__*/