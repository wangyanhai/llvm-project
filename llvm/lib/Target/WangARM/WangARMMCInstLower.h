#ifndef __LLVM_TARGET_WANGARM_MCINSTLOWER__
#define __LLVM_TARGET_WANGARM_MCINSTLOWER__

#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/Support/Compiler.h"

namespace llvm {
class MCContext;
class MCInst;
class MCOperand;
class MachineInstr;
class MachineFunction;
class Mangler;
class AsmPrinter;

namespace WangARMII {

/// Target Operand Flag enum.
enum TOF {
  //===------------------------------------------------------------------===//
  // TOY-Specific MachineOperand flags.

  MO_NO_FLAG = 0,

  /// MO_LO16 - On a symbol operand, this represents a relocation containing
  /// lower 16 bit of the address. Used only via movw instruction.
  MO_LO16 = 0x1,

  /// MO_HI16 - On a symbol operand, this represents a relocation containing
  /// higher 16 bit of the address. Used only via movt instruction.
  MO_HI16 = 0x2,

  /// MO_OPTION_MASK - Most flags are mutually exclusive; this mask selects
  /// just that part of the flag set.
  MO_OPTION_MASK = 0x7f,

  // It's undefined behaviour if an enum overflows the range between its
  // smallest and largest values, but since these are |ed together, it can
  // happen. Put a sentinel in (values of this enum are stored as "unsigned
  // char").
  MO_UNUSED_MAXIMUM = 0xff
};
} // end namespace TOYII

/// \brief This class is used to lower an MachineInstr into an MCInst.
class LLVM_LIBRARY_VISIBILITY WangARMMCInstLower {
  typedef MachineOperand::MachineOperandType MachineOperandType;
  MCContext *Ctx;
  Mangler *Mang;
  AsmPrinter &Printer;

public:
  WangARMMCInstLower(class AsmPrinter &asmprinter);
  void Initialize(Mangler *mang, MCContext *C);
  void Lower(const MachineInstr *MI, MCInst &OutMI) const;
  MCOperand LowerOperand(const MachineOperand &MO, unsigned offset = 0) const;

private:
  MCOperand LowerSymbolOperand(const MachineOperand &MO,
                               MachineOperandType MOTy, unsigned Offset) const;
};
} // namespace llvm

#endif

