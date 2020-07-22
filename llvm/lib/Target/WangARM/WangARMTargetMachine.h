#ifndef _LLVM_WANG_TARGETMACHINE__
#define _LLVM_WANG_TARGETMACHINE__
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Target/TargetMachine.h"
#include <memory>
#include "WangARMSubtarget.h"
//#include "WangARMGenInstrInfo.inc"

namespace llvm {

class Module;

class WangARMTargetMachine : public LLVMTargetMachine {
  WangARMSubtarget Subtarget;
  //WangInstrInfo InstrInfo;
  //TargetFrameInfo FrameInfo;

protected:
  //virtual const TargetAsmInfo *createTargetAsmInfo() const;
  std::unique_ptr<TargetLoweringObjectFile> TLOF;

public:
  WangARMTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                       StringRef FS, const TargetOptions &Options,
                       Optional<Reloc::Model> RM, Optional<CodeModel::Model> CM,
                       CodeGenOpt::Level OL, bool JIT);

  // Pass Pipeline Configuration
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;
  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }

  //virtual const WangInstrInfo *getInstrInfo() const {return &InstrInfo; }
  //virtual const TargetFrameInfo *getFrameInfo() const {return &FrameInfo; }
  virtual const TargetSubtargetInfo *getSubtargetImpl(const Function &F) const {
    return &Subtarget;
  }

  // DO NOT IMPLEMENT: There is no such thing as a valid default subtarget,
  // subtargets are per-function entities based on the target-specific
  // attributes of each function.
  const TargetSubtargetInfo *getSubtargetImpl() const = delete;
  //virtual const TargetRegisterInfo *getRegisterInfo() const {
  //  return &InstrInfo.getRegisterInfo();
  //}
  //virtual const DataLayout *getDataLayout() const { return &DataLayout; }
  //static unsigned getModuleMatchQuality(const Module &M);

  //// Pass Pipeline Configuration
  //virtual bool addInstSelector(PassManagerBase &PM, bool Fast);
  //virtual bool addPreEmitPass(PassManagerBase &PM, bool Fast);
};

} // end namespace llvm

#endif