#include "WangARMFixupKinds.h"
#include "WangARMMCTargetDesc.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "WangARM.h"


using namespace llvm;

#define DEBUG_TYPE "mccodeemitter"

STATISTIC(MCNumEmitted, "Number of MC instructions emitted.");


namespace llvm{
class WangARMMCCodeEmitter : public MCCodeEmitter {
  WangARMMCCodeEmitter(const WangARMMCCodeEmitter &) = delete;
  void operator=(const WangARMMCCodeEmitter &) = delete;
  const MCInstrInfo &MCII;
  const MCContext &CTX;

public:
  WangARMMCCodeEmitter(const MCInstrInfo &mcii, MCContext &ctx)
      : MCII(mcii), CTX(ctx) {}

  ~WangARMMCCodeEmitter() {}

  // getBinaryCodeForInstr - TableGen'erated function for getting the
  // binary encoding for an instruction.
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  /// getMachineOpValue - Return binary encoding of operand. If the machine
  /// operand requires relocation, record the relocation and return zero.
  unsigned getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  unsigned getMemSrcValue(const MCInst &MI, unsigned OpIdx,
                          SmallVectorImpl<MCFixup> &Fixups,
                          const MCSubtargetInfo &STI) const;

  void EmitByte(unsigned char C, raw_ostream &OS) const { OS << (char)C; }

  void EmitConstant(uint64_t Val, unsigned Size, raw_ostream &OS) const {
    // Output the constant in little endian byte order.
    for (unsigned i = 0; i != Size; ++i) {
      EmitByte(Val & 255, OS);
      Val >>= 8;
    }
  }

  void encodeInstruction(const MCInst &MI, raw_ostream &OS,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;

  uint32_t getARMBLTargetOpValue(const MCInst &MI, unsigned OpIdx,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;
};

} // end anonymous namespace

/// getMachineOpValue - Return binary encoding of operand. If the machine
/// operand requires relocation, record the relocation and return zero.
unsigned
WangARMMCCodeEmitter::getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                                        SmallVectorImpl<MCFixup> &Fixups,
                                        const MCSubtargetInfo &STI) const {
  if (MO.isReg()) {
    return CTX.getRegisterInfo()->getEncodingValue(MO.getReg());
  }

  if (MO.isImm()) {
    return static_cast<unsigned>(MO.getImm());
  }

  assert(MO.isExpr() && "unknown operand kind in printOperand");

  const MCExpr *Expr = MO.getExpr();
  MCExpr::ExprKind Kind = Expr->getKind();

  if (Kind == MCExpr::Binary) {
    Expr = static_cast<const MCBinaryExpr *>(Expr)->getLHS();
    Kind = Expr->getKind();
  }

  assert(Kind == MCExpr::SymbolRef);

  unsigned FixupKind;
  switch (cast<MCSymbolRefExpr>(Expr)->getKind()) {
  default:
    llvm_unreachable("Unknown fixup kind!");
  case MCSymbolRefExpr::VK_WangARM_LO: {
    FixupKind = WangARM::fixup_wangarm_mov_lo16_pcrel;
    break;
  }
  case MCSymbolRefExpr::VK_WangARM_HI: {
    FixupKind = WangARM::fixup_wangarm_mov_hi16_pcrel;
    break;
  }
  }

  Fixups.push_back(MCFixup::create(0, MO.getExpr(), MCFixupKind(FixupKind)));
  return 0;
}

unsigned
WangARMMCCodeEmitter::getMemSrcValue(const MCInst &MI, unsigned OpIdx,
                                     SmallVectorImpl<MCFixup> &Fixups,
                                     const MCSubtargetInfo &STI) const {
  unsigned Bits = 0;
  const MCOperand &RegMO = MI.getOperand(OpIdx);
  const MCOperand &ImmMO = MI.getOperand(OpIdx + 1);
  assert(ImmMO.getImm() >= 0);
  Bits |= (getMachineOpValue(MI, RegMO, Fixups, STI) << 12);
  Bits |= (unsigned)ImmMO.getImm() & 0xfff;
  return Bits;
}

void WangARMMCCodeEmitter::encodeInstruction(const MCInst &MI, raw_ostream &OS,
                                             SmallVectorImpl<MCFixup> &Fixups,
                                             const MCSubtargetInfo &STI) const {
  const MCInstrDesc &Desc = MCII.get(MI.getOpcode());
  if (Desc.getSize() != 4) {
    llvm_unreachable("Unexpected instruction size!");
  }

  const uint32_t Binary = getBinaryCodeForInstr(MI, Fixups, STI);

  EmitConstant(Binary, Desc.getSize(), OS);
  ++MCNumEmitted;
}

/// getBranchTargetOpValue - Helper function to get the branch target operand,
/// which is either an immediate or requires a fixup.
static uint32_t getBranchTargetOpValue(const MCInst &MI, unsigned OpIdx,
                                       unsigned FixupKind,
                                       SmallVectorImpl<MCFixup> &Fixups,
                                       const MCSubtargetInfo &STI) {
  const MCOperand &MO = MI.getOperand(OpIdx);

  // If the destination is an immediate, we have nothing to do.
  if (MO.isImm())
    return MO.getImm();
  assert(MO.isExpr() && "Unexpected branch target type!");
  const MCExpr *Expr = MO.getExpr();
  MCFixupKind Kind = MCFixupKind(FixupKind);
  Fixups.push_back(MCFixup::create(0, Expr, Kind, MI.getLoc()));

  // All of the information is in the fixup.
  return 0;
}

/// Return true if this branch has a non-always predication
static bool HasConditionalBranch(const MCInst &MI) {
  int NumOp = MI.getNumOperands();
  //if (NumOp >= 2) {
  //  for (int i = 0; i < NumOp - 1; ++i) {
  //    const MCOperand &MCOp1 = MI.getOperand(i);
  //    const MCOperand &MCOp2 = MI.getOperand(i + 1);
  //    if (MCOp1.isImm() && MCOp2.isReg() &&
  //        (MCOp2.getReg() == 0 || MCOp2.getReg() == ARM::CPSR)) {
  //      if (ARMCC::CondCodes(MCOp1.getImm()) != ARMCC::AL)
  //        return true;
  //    }
  //  }
  //}
  return false;
}

uint32_t WangARMMCCodeEmitter::getARMBLTargetOpValue(const MCInst &MI, unsigned OpIdx,
                                        SmallVectorImpl<MCFixup> &Fixups,
                                        const MCSubtargetInfo &STI) const {
  const MCOperand MO = MI.getOperand(OpIdx);
  if (MO.isExpr()) {
    if (HasConditionalBranch(MI))
      return ::getBranchTargetOpValue(MI, OpIdx, WangARM::fixup_wangarm_condbl,
                                      Fixups, STI);
    return ::getBranchTargetOpValue(MI, OpIdx, WangARM::fixup_wangarm_uncondbl, Fixups,
                                    STI);
  }

  return MO.getImm() >> 2;
}

#include "WangARMGenMCCodeEmitter.inc"

MCCodeEmitter *createWangARMMCCodeEmitter(const MCInstrInfo &MCII,
                                          const MCRegisterInfo &MRI,
                                          MCContext &Ctx) {
  return new WangARMMCCodeEmitter(MCII, Ctx);
}