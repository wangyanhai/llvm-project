#include "WangARMFixupKinds.h"
#include "WangARMMCTargetDesc.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/BinaryFormat/MachO.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDirectives.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCMachObjectWriter.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/MC/MCSectionMachO.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
using namespace llvm;

#if 1
namespace {
class WangARMELFObjectTargetWriter : public MCELFObjectTargetWriter {
public:
  WangARMELFObjectTargetWriter(uint8_t OSABI);

  virtual ~WangARMELFObjectTargetWriter();

  unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                        const MCFixup &Fixup, bool IsPCRel) const override;

};

class WangARMAsmBackend : public MCAsmBackend {
  // The STI from the target triple the MCAsmBackend was instantiated with
  // note that MCFragments may have a different local STI that should be
  // used in preference.
  const MCSubtargetInfo &STI;

public:
  WangARMAsmBackend(const Target &T, const MCSubtargetInfo &STI,
                    support::endianness Endian)
      : MCAsmBackend(Endian), STI(STI) {}

  ~WangARMAsmBackend() {}

  unsigned getNumFixupKinds() const override {
    return WangARM::NumTargetFixupKinds;
  }

  const MCFixupKindInfo &getFixupKindInfo(MCFixupKind Kind) const override {
    const static MCFixupKindInfo Infos[WangARM::NumTargetFixupKinds] = {
        // This table *must* be in the order that the fixup_* kinds are defined
        // in
        // TOYFixupKinds.h.
        //
        // Name                      Offset (bits) Size (bits)     Flags
        {"fixup_toy_mov_hi16_pcrel", 0, 32, MCFixupKindInfo::FKF_IsPCRel},
        {"fixup_toy_mov_lo16_pcrel", 0, 32, MCFixupKindInfo::FKF_IsPCRel},
        {"fixup_wangarm_uncondbl", 0, 32, MCFixupKindInfo::FKF_IsPCRel},
        {"fixup_wangarm_condbl", 0, 32, MCFixupKindInfo::FKF_IsPCRel},
    };

    if (Kind < FirstTargetFixupKind) {
      return MCAsmBackend::getFixupKindInfo(Kind);
    }

    assert(unsigned(Kind - FirstTargetFixupKind) < getNumFixupKinds() &&
           "Invalid kind!");
    return Infos[Kind - FirstTargetFixupKind];
  }

  void applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                  const MCValue &Target, MutableArrayRef<char> Data,
                  uint64_t Value, bool IsResolved,
                  const MCSubtargetInfo *STI) const override;

  bool mayNeedRelaxation(const MCInst &Inst,
                         const MCSubtargetInfo &STI) const override {
    return false;
  }

  bool fixupNeedsRelaxation(const MCFixup &Fixup, uint64_t Value,
                            const MCRelaxableFragment *DF,
                            const MCAsmLayout &Layout) const override {
    return false;
  }

  void relaxInstruction(const MCInst &Inst, const MCSubtargetInfo &STI,
                        MCInst &Res) const override {}

  bool writeNopData(raw_ostream &OS, uint64_t Count) const override {
    if (Count == 0) {
      return true;
    }
    return false;
  }

  unsigned getPointerSize() const { return 4; }
};

} // end anonymous namespace

unsigned WangARMELFObjectTargetWriter::getRelocType(MCContext &Ctx,
                                                    const MCValue &Target,
                                                    const MCFixup &Fixup,
                                                    bool IsPCRel) const {
  if (!IsPCRel) {
    llvm_unreachable("Only dealying with PC-relative fixups for now");
  }

  unsigned Type = 0;
  switch ((unsigned)Fixup.getKind()) {
  default:
    llvm_unreachable("Unimplemented");
  case WangARM::fixup_wangarm_mov_hi16_pcrel:
    Type = ELF::R_ARM_MOVT_PREL;
    break;
  case WangARM::fixup_wangarm_mov_lo16_pcrel:
    Type = ELF::R_ARM_MOVW_PREL_NC;
    break;
  }
  return Type;
}

WangARMELFObjectTargetWriter::WangARMELFObjectTargetWriter(uint8_t OSABI)
    : MCELFObjectTargetWriter(/*Is64Bit*/ false, OSABI,ELF::EM_ARM,
                              /*HasRelocationAddend*/ false) {}

WangARMELFObjectTargetWriter::~WangARMELFObjectTargetWriter() {}

static unsigned adjustFixupValue(const MCFixup &Fixup, uint64_t Value,
                                 MCContext *Ctx = NULL) {
  unsigned Kind = Fixup.getKind();
  switch (Kind) {
  default:
    llvm_unreachable("Unknown fixup kind!");
  case WangARM::fixup_wangarm_mov_hi16_pcrel:
    Value >>= 16;
  // Intentional fall-through
  case WangARM::fixup_wangarm_mov_lo16_pcrel:
  {
    unsigned Hi4 = (Value & 0xF000) >> 12;
    unsigned Lo12 = Value & 0x0FFF;
    // inst{19-16} = Hi4;
    // inst{11-0} = Lo12;
    Value = (Hi4 << 16) | (Lo12);
    return Value;
  }
  case WangARM::fixup_wangarm_uncondbl:
  case WangARM::fixup_wangarm_condbl:
    // These values don't encode the low two bits since they're always zero.
    // Offset by 8 just as above.
    if (const MCSymbolRefExpr *SRE =
            dyn_cast<MCSymbolRefExpr>(Fixup.getValue()))
      if (SRE->getKind() == MCSymbolRefExpr::VK_TLSCALL)
        return 0;
    return 0xffffff & ((Value - 8) >> 2);
  }
  return Value;
}

void WangARMAsmBackend::applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                                   const MCValue &Target,
                                   MutableArrayRef<char> Data, uint64_t Value,
                                   bool IsResolved,
                                   const MCSubtargetInfo *STI) const {
  unsigned NumBytes = 4;
  Value = adjustFixupValue(Fixup, Value);
  if (!Value) {
    return; // Doesn't change encoding.
  }

  unsigned Offset = Fixup.getOffset();
  assert(Offset + NumBytes <= Data.size() && "Invalid fixup offset!");

  // For each byte of the fragment that the fixup touches, mask in the bits from
  // the fixup value. The Value has been "split up" into the appropriate
  // bitfields above.
  for (unsigned i = 0; i != NumBytes; ++i) {
    Data[Offset + i] |= uint8_t((Value >> (i * 8)) & 0xff);
  }
}

namespace {

class WangARMELFAsmBackend : public WangARMAsmBackend {
public:
  uint8_t OSABI;
  WangARMELFAsmBackend(const Target &T, const MCSubtargetInfo &STI,
                       uint8_t _OSABI)
      : WangARMAsmBackend(T, STI, support::little),OSABI(_OSABI) {}

  std::unique_ptr<MCObjectTargetWriter> createObjectTargetWriter()
	  const override {
    //return nullptr;
    return std::make_unique<WangARMELFObjectTargetWriter>(OSABI);
  }

};

} // end anonymous namespace

// MCAsmBackend *createWangARMAsmBackend(const Target &T,
//                                        const MCRegisterInfo &MRI, StringRef
//                                        TT, StringRef CPU) {
//  Triple TheTriple(TT);
//  const uint8_t ABI = MCELFObjectTargetWriter::getOSABI(Triple(TT).getOS());
//  return new WangARMELFAsmBackend(T, TT, ABI);
//}
#endif

MCAsmBackend *createWangARMAsmBackend(const Target &T,
                                      const MCSubtargetInfo &STI,
                                      const MCRegisterInfo &MRI,
                                      const MCTargetOptions &Options) {
  uint8_t _OSABI = ELF::ELFOSABI_GNU;
  return new WangARMELFAsmBackend(T, STI,_OSABI);
}
