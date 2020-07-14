#include "WangARMTargetMachine.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/CodeGen/ExecutionDomainFix.h"
#include "llvm/CodeGen/GlobalISel/CallLowering.h"
#include "llvm/CodeGen/GlobalISel/IRTranslator.h"
#include "llvm/CodeGen/GlobalISel/InstructionSelect.h"
#include "llvm/CodeGen/GlobalISel/InstructionSelector.h"
#include "llvm/CodeGen/GlobalISel/Legalizer.h"
#include "llvm/CodeGen/GlobalISel/LegalizerInfo.h"
#include "llvm/CodeGen/GlobalISel/RegBankSelect.h"
#include "llvm/CodeGen/GlobalISel/RegisterBankInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineScheduler.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/MC/MCAsmInfoCOFF.h"
#include "llvm/Pass.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetParser.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/MC/MCDwarf.h"
#include <cassert>
#include <memory>
#include <string>
#include "WangARM.h"
using namespace llvm;

Target &llvm::getTheWangARMTarget() {
  static Target TheWangARMLETarget;
  return TheWangARMLETarget;
}

extern "C" void LLVMInitializeWangARMTargetInfo() {

  RegisterTarget<Triple::wangarm, /*HasJIT=*/false> X(
      getTheWangARMTarget(), "wangarm", "WangARM", "WangARM");
}

extern "C" void LLVMInitializeWangARMTarget() {
  // Register the target.
  RegisterTargetMachine<WangARMTargetMachine> X(getTheWangARMTarget());

  PassRegistry &Registry = *PassRegistry::getPassRegistry();
  initializeGlobalISel(Registry);
}

class WangARMMCAsmInfoMicrosoft : public MCAsmInfoMicrosoft {
  void anchor() override
  {}

public:
	explicit WangARMMCAsmInfoMicrosoft() {
    AlignmentIsInBytes = false;
    ExceptionsType = ExceptionHandling::WinEH;
    PrivateGlobalPrefix = "$M";
    PrivateLabelPrefix = "$M";
    CommentString = ";";

    MaxInstLength = 6;
  }
};

MCAsmInfo *createWangARMMCAsmInfo(const MCRegisterInfo &MRI,
                                         const Triple &TheTriple) {
	
  MCAsmInfo *MAI = new WangARMMCAsmInfoMicrosoft();
  //unsigned Reg = MRI.getDwarfRegNum(0, true);
  //MAI->addInitialFrameState(MCCFIInstruction::createDefCfa(nullptr, Reg, 0));
  return MAI;
}

MCInstPrinter *createWangARMMCInstPrinter(const Triple &T,
                                          unsigned SyntaxVariant,
                                          const MCAsmInfo &MAI,
                                          const MCInstrInfo &MII,
                                          const MCRegisterInfo &MRI);

// Force static initialization.
extern "C" void LLVMInitializeWangARMTargetMC() {
  // Register the MC asm info.
  RegisterMCAsmInfoFn X(getTheWangARMTarget(), createWangARMMCAsmInfo);

   // Register the MCInstPrinter.
  TargetRegistry::RegisterMCInstPrinter(getTheWangARMTarget(), createWangARMMCInstPrinter);


}

namespace llvm {

	static std::unique_ptr<TargetLoweringObjectFile> createTLOF(const Triple &TT) {
  if (TT.isOSWindows())
    return llvm::make_unique<TargetLoweringObjectFileCOFF>();
  return llvm::make_unique<TargetLoweringObjectFileCOFF>();
}

	class WangARMPassConfig : public TargetPassConfig {
public:
  WangARMPassConfig(WangARMTargetMachine *TM, PassManagerBase &PM)
      : TargetPassConfig(*TM, PM) {}

  WangARMTargetMachine &getWangARMTargetMachine() const {
    return getTM<WangARMTargetMachine>();
  }

  virtual bool addPreISel();
  virtual bool addInstSelector();
  virtual void addPreEmitPass();
};

	
	bool WangARMPassConfig::addPreISel() { return false; }

	bool WangARMPassConfig::addInstSelector() {
	  addPass(createWangARMISelDag(getWangARMTargetMachine(), getOptLevel()));
	  return false;
	}

	void WangARMPassConfig::addPreEmitPass() {  }

	WangARMTargetMachine::WangARMTargetMachine(const Target &T, const Triple &TT,
											   StringRef CPU, StringRef FS,
											   const TargetOptions &Options,
											   Optional<Reloc::Model> RM,
											   Optional<CodeModel::Model> CM,
											   CodeGenOpt::Level OL, bool JIT)
		: LLVMTargetMachine(
			  T, "e-m:e-p:32:32-Fi8-i64:64-v128:64:128-a:0:32-n32-S64", 
			TT, CPU, FS, Options, Reloc::Static, CodeModel::Small, OL),
          TLOF(createTLOF(getTargetTriple())),
		  Subtarget(TT,CPU,FS,*this){
          initAsmInfo();
	}


	TargetPassConfig *WangARMTargetMachine::createPassConfig(PassManagerBase &PM) {
            return new WangARMPassConfig(this, PM);
	}
} // namespace llvm
