#include "WangARMMCTargetDesc.h"
#include "WangARM.h"
#include "WangARMTargetMachine.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/MC/MCAsmInfoCOFF.h"
#include "llvm/MC/MCDwarf.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCContext.h"
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

class WangARMTargetAsmStreamer : public ARMTargetStreamer {
  formatted_raw_ostream &OS;
  MCInstPrinter &InstPrinter;
  bool IsVerboseAsm;

  void emitFnStart() override;
  void emitFnEnd() override;
  void emitCantUnwind() override;
  void emitPersonality(const MCSymbol *Personality) override;
  void emitPersonalityIndex(unsigned Index) override;
  void emitHandlerData() override;
  void emitSetFP(unsigned FpReg, unsigned SpReg, int64_t Offset = 0) override;
  void emitMovSP(unsigned Reg, int64_t Offset = 0) override;
  void emitPad(int64_t Offset) override;
  void emitRegSave(const SmallVectorImpl<unsigned> &RegList,
                   bool isVector) override;
  void emitUnwindRaw(int64_t Offset,
                     const SmallVectorImpl<uint8_t> &Opcodes) override;

  void switchVendor(StringRef Vendor) override;
  void emitAttribute(unsigned Attribute, unsigned Value) override;
  void emitTextAttribute(unsigned Attribute, StringRef String) override;
  void emitIntTextAttribute(unsigned Attribute, unsigned IntValue,
                            StringRef StringValue) override;
  void emitArch(ARM::ArchKind Arch) override;
  void emitArchExtension(unsigned ArchExt) override;
  void emitObjectArch(ARM::ArchKind Arch) override;
  void emitFPU(unsigned FPU) override;
  void emitInst(uint32_t Inst, char Suffix = '\0') override;
  void finishAttributeSection() override;

  void AnnotateTLSDescriptorSequence(const MCSymbolRefExpr *SRE) override;
  void emitThumbSet(MCSymbol *Symbol, const MCExpr *Value) override;

public:
  WangARMTargetAsmStreamer(MCStreamer &S, formatted_raw_ostream &OS,
                       MCInstPrinter &InstPrinter, bool VerboseAsm);
};

WangARMTargetAsmStreamer::WangARMTargetAsmStreamer(MCStreamer &S,
                                           formatted_raw_ostream &OS,
                                           MCInstPrinter &InstPrinter,
                                           bool VerboseAsm)
    : ARMTargetStreamer(S), OS(OS), InstPrinter(InstPrinter),
      IsVerboseAsm(VerboseAsm) {}

void WangARMTargetAsmStreamer::emitFnStart() { OS << "\t.fnstart\n"; }
void WangARMTargetAsmStreamer::emitFnEnd() { OS << "\t.fnend\n"; }
void WangARMTargetAsmStreamer::emitCantUnwind() { OS << "\t.cantunwind\n"; }

void WangARMTargetAsmStreamer::emitPersonality(const MCSymbol *Personality) {
  OS << "\t.personality " << Personality->getName() << '\n';
}

void WangARMTargetAsmStreamer::emitPersonalityIndex(unsigned Index) {
  OS << "\t.personalityindex " << Index << '\n';
}

void WangARMTargetAsmStreamer::emitHandlerData() { OS << "\t.handlerdata\n"; }

void WangARMTargetAsmStreamer::emitSetFP(unsigned FpReg, unsigned SpReg,
                                     int64_t Offset) {
  OS << "\t.setfp\t";
  InstPrinter.printRegName(OS, FpReg);
  OS << ", ";
  InstPrinter.printRegName(OS, SpReg);
  if (Offset)
    OS << ", #" << Offset;
  OS << '\n';
}

void WangARMTargetAsmStreamer::emitMovSP(unsigned Reg, int64_t Offset) {
  assert((Reg != WangARM::SP && Reg != WangARM::PC) &&
         "the operand of .movsp cannot be either sp or pc");

  OS << "\t.movsp\t";
  InstPrinter.printRegName(OS, Reg);
  if (Offset)
    OS << ", #" << Offset;
  OS << '\n';
}

void WangARMTargetAsmStreamer::emitPad(int64_t Offset) {
  OS << "\t.pad\t#" << Offset << '\n';
}

void WangARMTargetAsmStreamer::emitRegSave(const SmallVectorImpl<unsigned> &RegList,
                                       bool isVector) {
  assert(RegList.size() && "RegList should not be empty");
  if (isVector)
    OS << "\t.vsave\t{";
  else
    OS << "\t.save\t{";

  InstPrinter.printRegName(OS, RegList[0]);

  for (unsigned i = 1, e = RegList.size(); i != e; ++i) {
    OS << ", ";
    InstPrinter.printRegName(OS, RegList[i]);
  }

  OS << "}\n";
}

void WangARMTargetAsmStreamer::switchVendor(StringRef Vendor) {}

void WangARMTargetAsmStreamer::emitAttribute(unsigned Attribute, unsigned Value) {
  OS << "\t.eabi_attribute\t" << Attribute << ", " << Twine(Value);
  if (IsVerboseAsm) {
    StringRef Name = ARMBuildAttrs::AttrTypeAsString(Attribute);
    if (!Name.empty())
      OS << "\t@ " << Name;
  }
  OS << "\n";
}

void WangARMTargetAsmStreamer::emitTextAttribute(unsigned Attribute,
                                             StringRef String) {
  switch (Attribute) {
  case ARMBuildAttrs::CPU_name:
    OS << "\t.cpu\t" << String.lower();
    break;
  default:
    OS << "\t.eabi_attribute\t" << Attribute << ", \"" << String << "\"";
    if (IsVerboseAsm) {
      StringRef Name = ARMBuildAttrs::AttrTypeAsString(Attribute);
      if (!Name.empty())
        OS << "\t@ " << Name;
    }
    break;
  }
  OS << "\n";
}

void WangARMTargetAsmStreamer::emitIntTextAttribute(unsigned Attribute,
                                                unsigned IntValue,
                                                StringRef StringValue) {
  switch (Attribute) {
  default:
    llvm_unreachable("unsupported multi-value attribute in asm mode");
  case ARMBuildAttrs::compatibility:
    OS << "\t.eabi_attribute\t" << Attribute << ", " << IntValue;
    if (!StringValue.empty())
      OS << ", \"" << StringValue << "\"";
    if (IsVerboseAsm)
      OS << "\t@ " << ARMBuildAttrs::AttrTypeAsString(Attribute);
    break;
  }
  OS << "\n";
}

void WangARMTargetAsmStreamer::emitArch(ARM::ArchKind Arch) {
  OS << "\t.arch\t" << ARM::getArchName(Arch) << "\n";
}

void WangARMTargetAsmStreamer::emitArchExtension(unsigned ArchExt) {
  OS << "\t.arch_extension\t" << ARM::getArchExtName(ArchExt) << "\n";
}

void WangARMTargetAsmStreamer::emitObjectArch(ARM::ArchKind Arch) {
  OS << "\t.object_arch\t" << ARM::getArchName(Arch) << '\n';
}

void WangARMTargetAsmStreamer::emitFPU(unsigned FPU) {
  OS << "\t.fpu\t" << ARM::getFPUName(FPU) << "\n";
}

void WangARMTargetAsmStreamer::finishAttributeSection() {}

void WangARMTargetAsmStreamer::AnnotateTLSDescriptorSequence(
    const MCSymbolRefExpr *S) {
  OS << "\t.tlsdescseq\t" << S->getSymbol().getName();
}

void WangARMTargetAsmStreamer::emitThumbSet(MCSymbol *Symbol, const MCExpr *Value) {
  const MCAsmInfo *MAI = Streamer.getContext().getAsmInfo();

  OS << "\t.thumb_set\t";
  Symbol->print(OS, MAI);
  OS << ", ";
  Value->print(OS, MAI);
  OS << '\n';
}

void WangARMTargetAsmStreamer::emitInst(uint32_t Inst, char Suffix) {
  OS << "\t.inst";
  if (Suffix)
    OS << "." << Suffix;
  OS << "\t0x" << Twine::utohexstr(Inst) << "\n";
}

void WangARMTargetAsmStreamer::emitUnwindRaw(
    int64_t Offset, const SmallVectorImpl<uint8_t> &Opcodes) {
  OS << "\t.unwind_raw " << Offset;
  for (SmallVectorImpl<uint8_t>::const_iterator OCI = Opcodes.begin(),
                                                OCE = Opcodes.end();
       OCI != OCE; ++OCI)
    OS << ", 0x" << Twine::utohexstr(*OCI);
  OS << '\n';
}

MCTargetStreamer *createWangARMTargetAsmStreamer(MCStreamer &S,
                                                 formatted_raw_ostream &OS,
                                                 MCInstPrinter *InstPrint,
                                                 bool isVerboseAsm) {
	
	return new WangARMTargetAsmStreamer(S,OS,*InstPrint,isVerboseAsm);
}

// Force static initialization.
extern "C" void LLVMInitializeWangARMTargetMC() {
  // Register the MC asm info.
  RegisterMCAsmInfoFn X(getTheWangARMTarget(), createWangARMMCAsmInfo);

  // Register the MCInstPrinter.
  TargetRegistry::RegisterMCInstPrinter(getTheWangARMTarget(),
                                        createWangARMMCInstPrinter);

  // Register the asm streamer.
  TargetRegistry::RegisterAsmTargetStreamer(getTheWangARMTarget(),
                                            createWangARMTargetAsmStreamer);
}
