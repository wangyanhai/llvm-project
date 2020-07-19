#ifndef __LLVM_TARGET_WANGARM_ASMPRINTER__
#define __LLVM_TARGET_WANGARM_ASMPRINTER__

#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {

class MCOperand;
class MachineConstantPool;
class MachineOperand;
class MCSymbol;

class LLVM_LIBRARY_VISIBILITY WangARMAsmPrinter : public AsmPrinter {

 
public:
  explicit WangARMAsmPrinter(TargetMachine &TM,
                         std::unique_ptr<MCStreamer> Streamer);

  StringRef getPassName() const override { return "WangARM Assembly Printer"; }
  void EmitInstruction(const MachineInstr *MI);
};

}

#endif