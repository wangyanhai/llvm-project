#include "WangARMMCTargetDesc.h"
#include "WangARM.h"
#include "WangARMTargetMachine.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/MC/MCAsmInfoCOFF.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDwarf.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/TargetParser.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Transforms/Scalar.h"
#include <assert.h>
//#include "WangARMMCCodeEmitter.h"

using namespace llvm;

class WangARMMCAsmInfoMicrosoft : public MCAsmInfoMicrosoft {
  void anchor() override {}

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
  // unsigned Reg = MRI.getDwarfRegNum(0, true);
  // MAI->addInitialFrameState(MCCFIInstruction::createDefCfa(nullptr, Reg, 0));
  return MAI;
}

MCInstPrinter *createWangARMMCInstPrinter(const Triple &T,
                                          unsigned SyntaxVariant,
                                          const MCAsmInfo &MAI,
                                          const MCInstrInfo &MII,
                                          const MCRegisterInfo &MRI);

MCCodeEmitter *createWangARMMCCodeEmitter(const MCInstrInfo &MCII,
                                          const MCRegisterInfo &MRI,
                                          MCContext &Ctx);

MCTargetStreamer *createWangARMTargetAsmStreamer(MCStreamer &S,
                                                 formatted_raw_ostream &OS,
                                                 MCInstPrinter *InstPrint,
                                                 bool isVerboseAsm);

MCAsmBackend *createWangARMAsmBackend(const Target &T,
                                      const MCSubtargetInfo &STI,
                                      const MCRegisterInfo &MRI,
                                      const MCTargetOptions &Options); 

#define GET_REGINFO_MC_DESC
#include "WangARMGenRegisterInfo.inc"

#define GET_INSTRINFO_MC_DESC
#include "WangARMGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "WangARMGenSubtargetInfo.inc"

static MCInstrInfo *createWangARMMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitWangARMMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createWangARMMCRegisterInfo(const Triple &Triple) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitWangARMMCRegisterInfo(X, WangARM::LR, 0, 0, WangARM::PC);
  return X;
}

// Force static initialization.
extern "C" void LLVMInitializeWangARMTargetMC() {
  // Register the MC asm info.
  RegisterMCAsmInfoFn X(getTheWangARMTarget(), createWangARMMCAsmInfo);

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(getTheWangARMTarget(),createWangARMMCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(getTheWangARMTarget(),createWangARMMCRegisterInfo);


  // Register the MCInstPrinter.
  TargetRegistry::RegisterMCInstPrinter(getTheWangARMTarget(),
                                        createWangARMMCInstPrinter);

  // Register the asm streamer.
  TargetRegistry::RegisterAsmTargetStreamer(getTheWangARMTarget(),
                                            createWangARMTargetAsmStreamer);

  // Register the MCCodeEmitter
  TargetRegistry::RegisterMCCodeEmitter(getTheWangARMTarget(),
                                        createWangARMMCCodeEmitter);


  TargetRegistry::RegisterMCAsmBackend(getTheWangARMTarget(),
                                       createWangARMAsmBackend);
}
