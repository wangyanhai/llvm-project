//this file is intruction-format of arm-simple.
//created by wangyanhai

//===----------------------------------------------------------------------===//
// Instruction format superclass
//===----------------------------------------------------------------------===//

class InstWang<dag outs, dag ins, string asmstr, list<dag> pattern>
    : Instruction{
  //instruction length: 32bit
  field bits<32> Inst;
  // Mask of bits that cause an encoding to be UNPREDICTABLE.
  // If a bit is set, then if the corresponding bit in the
  // target encoding differs from its value in the "Inst" field,
  // the instruction is UNPREDICTABLE (SoftFail in abstract parlance).
  field bits<32> Unpredictable = 0;
  // SoftFail is the generic name for this field, but we alias it so
  // as to make it more obvious what it means in ARM-land.
  field bits<32> SoftFail = Unpredictable;

  let Namespace = "WangARM";
  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString   = asmstr;
  let Pattern = pattern;
  let Size = 4;

  // Always execute - no condition flags for this example
  let Inst{31-28} = 0b1110;
}

// Wang pseudo instructions format
class WangARMPseudoInst<dag outs, dag ins, string asmstr, list<dag> pattern>
    : InstWang<outs, ins, asmstr, pattern> {
  let isPseudo = 1;
}

class ALUInst<bits<4> opcode, dag outs, dag ins, string asmstr, list<dag> pattern>
    : InstWang<outs, ins, asmstr, pattern> {
  let Inst{27-26} = 0b00;
  let Inst{24-21} = opcode;
}

class MulInst<bits<7> opcode, dag outs, dag ins, string asmstr,
              list<dag> pattern> : InstWang<outs, ins, asmstr, pattern> {
  let Inst{27-21} = opcode;
  let Inst{20}    = 0;  // We never update flags
  let Inst{7-4}   = 0b1001;
}

class BranchInst<bits<4> opcode, dag outs, dag ins,
                 string asmstr, list<dag> pattern>
    : InstWang<outs, ins, asmstr, pattern> {
  let Inst{27-24} = opcode;
}
