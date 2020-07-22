#include "WangARMInstPrinter.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cassert>
#include <cstdint>

using namespace llvm;

#define DEBUG_TYPE "asm-printer"

#define PRINT_ALIAS_INSTR
#include "WangARMGenAsmWriter.inc"

WangARMInstPrinter::WangARMInstPrinter(const MCAsmInfo &MAI,
                                       const MCInstrInfo &MII,
                               const MCRegisterInfo &MRI)
    : MCInstPrinter(MAI, MII, MRI) {}

void WangARMInstPrinter::printRegName(raw_ostream &OS, unsigned RegNo) const {
  OS << StringRef(getRegisterName(RegNo)).lower();
}

void WangARMInstPrinter::printInst(const MCInst *MI, raw_ostream &O,
                               StringRef Annot, const MCSubtargetInfo &STI) {
  unsigned Opcode = MI->getOpcode();

  switch (Opcode) {
  // Check for MOVs and print canonical forms, instead.
  case WangARM::ADDri: {
    const MCOperand &Dst = MI->getOperand(0);
    const MCOperand &MO1 = MI->getOperand(1);
 
    O << '\t';
    printRegName(O, Dst.getReg());
    O << ", ";
    printRegName(O, MO1.getReg());
    printAnnotation(O, Annot);
    return;
	}
  }

  if (!printAliasInstr(MI, O))
    printInstruction(MI, O);

  printAnnotation(O, Annot);
}

void WangARMInstPrinter::printOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isReg()) {
    unsigned Reg = Op.getReg();
    printRegName(O, Reg);
  } else if (Op.isImm()) {
    O << markup("<imm:") << '#' << formatImm(Op.getImm()) << markup(">");
  } else {
    assert(Op.isExpr() && "unknown operand kind in printOperand");
    const MCExpr *Expr = Op.getExpr();
    switch (Expr->getKind()) {
    case MCExpr::Binary:
      O << '#';
      Expr->print(O, &MAI);
      break;
    case MCExpr::Constant: {
      // If a symbolic branch target was added as a constant expression then
      // print that address in hex. And only print 32 unsigned bits for the
      // address.
      const MCConstantExpr *Constant = cast<MCConstantExpr>(Expr);
      int64_t TargetAddress;
      if (!Constant->evaluateAsAbsolute(TargetAddress)) {
        O << '#';
        Expr->print(O, &MAI);
      } else {
        O << "0x";
        O.write_hex(static_cast<uint32_t>(TargetAddress));
      }
      break;
    }
    default:
      // FIXME: Should we always treat this as if it is a constant literal and
      // prefix it with '#'?
      Expr->print(O, &MAI);
      break;
    }
  }
}

void WangARMInstPrinter::printAddrModeMemSrc(const MCInst *MI, unsigned OpNum,
                                         raw_ostream &O) {
  const MCOperand &Op1 = MI->getOperand(OpNum);
  const MCOperand &Op2 = MI->getOperand(OpNum + 1);
  O << "[";
  printRegName(O, Op1.getReg());

  unsigned Offset = Op2.getImm();
  if (Offset) {
    O << ", #" << Offset;
  }
  O << "]";
}

MCInstPrinter *createWangARMMCInstPrinter(const Triple &T, unsigned SyntaxVariant,
                                      const MCAsmInfo &MAI,
                                      const MCInstrInfo &MII,
                                      const MCRegisterInfo &MRI) {
  if (SyntaxVariant == 0)
    return new WangARMInstPrinter(MAI, MII, MRI);
  return nullptr;
}