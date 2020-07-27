
#include "WangARMFixupKinds.h"
#include "WangARMMCTargetDesc.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
#if 0
namespace {
class WangARMELFObjectTargetWriter : public MCELFObjectTargetWriter {
public:
  WangARMELFObjectTargetWriter(uint8_t OSABI);

  virtual ~WangARMELFObjectTargetWriter();

  unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                        const MCFixup &Fixup, bool IsPCRel) const override;

};
} // namespace

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
  case WangARM::fixup_toy_mov_hi16_pcrel:
    Type = ELF::R_ARM_MOVT_PREL;
    break;
  case WangARM::fixup_toy_mov_lo16_pcrel:
    Type = ELF::R_ARM_MOVW_PREL_NC;
    break;
  }
  return Type;
}

WangARMELFObjectTargetWriter::WangARMELFObjectTargetWriter(uint8_t OSABI)
    : MCELFObjectTargetWriter(/*Is64Bit*/ false, OSABI,
                              /*ELF::EM_TOY*/ ELF::EM_ARM,
                              /*HasRelocationAddend*/ false) {}

WangARMELFObjectTargetWriter::~WangARMELFObjectTargetWriter() {}

#endif