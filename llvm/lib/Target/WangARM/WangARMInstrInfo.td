def WangARMimm8 : Operand<i32>, ImmLeaf<i32, [{
  return Imm >= 0 && Imm < 256;
}]>;

//===----------------------------------------------------------------------===//
// ALU Instructions
//===----------------------------------------------------------------------===//

multiclass BinOp<bits<4> opcode, string opstr, SDNode opnode> {
  def rr : ALUInst<opcode, (outs GRRegs:$dst),
                   (ins GRRegs:$src1, GRRegs:$src2),
                   !strconcat(opstr, " $dst, $src1, $src2"),
                   [(set i32:$dst, (opnode i32:$src1, i32:$src2))]> {
    bits<4> dst;
    bits<4> src1;
    bits<4> src2;
    let Inst{25} = 0;
    let Inst{20} = 0;  // We never update flags
    let Inst{19-16} = src1;
    let Inst{15-12} = dst;
    let Inst{11-4}  = 0b00000000;
    let Inst{3-0}   = src2;
  }
  def ri : ALUInst<opcode, (outs GRRegs:$dst),
                   (ins GRRegs:$src1, i32imm:$imm),
                   !strconcat(opstr, " $dst, $src1, $imm"),
                   [(set i32:$dst, (opnode i32:$src1, WangARMimm8:$imm))]> {
    bits<4> dst;
    bits<4> src1;
    bits<12> imm;
    let Inst{25} = 1;
    let Inst{20} = 0;  // We never update flags
    let Inst{19-16} = dst;
    let Inst{15-12} = src1;
    let Inst{11-0}  = imm;
  }
}

defm ADD  : BinOp<0b0100, "add", add>;
defm SUB  : BinOp<0b0010, "sub", sub>;