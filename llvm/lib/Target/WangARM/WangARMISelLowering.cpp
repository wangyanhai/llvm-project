#include "WangARMISelLowering.h"
#include "WangARM.h"
#include "WangARMRegisterInfo.h"
#include "WangARMSubtarget.h"


namespace llvm {

WangARMTargetLowering::WangARMTargetLowering(const TargetMachine &TM,
                                             const WangARMSubtarget &STI)
    : TargetLowering(TM), Subtarget(&STI) {
  addRegisterClass(MVT::i32, &WangARM::GRRegsRegClass);
  computeRegisterProperties(Subtarget->getRegisterInfo());
}

} // namespace llvm