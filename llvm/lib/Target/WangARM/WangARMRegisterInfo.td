//this file is register model of arm-simple.
//created by wangyanhai

class WangReg<bits<16> Enc, string n, list<string> altNames = []> : Register<n,altNames> {
  let HWEncoding = Enc;
  let Namespace = "WangARM";
}

// CPU registers
def R0  : WangReg< 0, "r0">;
def R1  : WangReg< 1, "r1">;
def R2  : WangReg< 2, "r2">;
def R3  : WangReg< 3, "r3">;
def R4  : WangReg< 4, "r4">;
def R5  : WangReg< 5, "r5">;
def R6  : WangReg< 6, "r6">;
def R7  : WangReg< 7, "r7">;
def R8  : WangReg< 8, "r8">;
def R9  : WangReg< 9, "r9">;

def SP  : WangReg<13, "sp", ["r13"]>;
def LR  : WangReg<14, "lr", ["r14"]>;
def PC  : WangReg<15, "pc", ["r15"]>;

// Register classes.
//
def GRRegs : RegisterClass<"WangARM", [i32], 32,
  // Return values and arguments
  (add R0, R1, R2, R3,
       R4, R5, R6, R7, R8, R9, SP)>;
