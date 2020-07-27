#ifndef _LLVM_WANGARM_MCASMSTREAMER__
#define _LLVM_WANGARM_MCASMSTREAMER__
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

using namespace llvm;

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


#endif