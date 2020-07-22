#include "WangARMTargetInfo.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

Target &llvm::getTheWangARMTarget() {
  static Target TheWangARMLETarget;
  return TheWangARMLETarget;
}

extern "C" void LLVMInitializeWangARMTargetInfo() {

  RegisterTarget<Triple::wangarm, /*HasJIT=*/false> X(
      getTheWangARMTarget(), "wangarm", "WangARM", "WangARM");
}
