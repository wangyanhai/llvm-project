#ifndef __LLVM_TARGET_WANGARM_FIXUPKINDS__
#define __LLVM_TARGET_WANGARM_FIXUPKINDS__

#include "llvm/MC/MCFixup.h"

namespace llvm {
namespace WangARM {
enum Fixups {
  fixup_wangarm_mov_hi16_pcrel = FirstTargetFixupKind,
  fixup_wangarm_mov_lo16_pcrel,

  // The following fixups handle the ARM BL instructions. These can be
  // conditionalised; however, the ARM ELF ABI requires a different relocation
  // in that case: R_ARM_JUMP24 instead of R_ARM_CALL. The difference is that
  // R_ARM_CALL is allowed to change the instruction to a BLX inline, which has
  // no conditional version; R_ARM_JUMP24 would have to insert a veneer.
  //
  // MachO does not draw a distinction between the two cases, so it will treat
  // fixup_arm_uncondbl and fixup_arm_condbl as identical fixups.

  // Fixup for unconditional ARM BL instructions.
  fixup_wangarm_uncondbl,

  // Fixup for ARM BL instructions with nontrivial conditionalisation.
  fixup_wangarm_condbl,	


  // Marker
  LastTargetFixupKind,
  NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
};
}
}

#endif

