//this file is target model of arm-simple.
//created by wangyanhai

include "llvm/Target/Target.td"
include "WangARMRegisterInfo.td"
include "WangArmInstrFormats.td"
include "WangArmInstrInfo.td"

def FeatureFPRegs         : SubtargetFeature<"fpregs", "HasFPRegs", "true",
                                             "Enable FP registers">;

def HasV4TOps   : SubtargetFeature<"v4t", "HasV4TOps", "true", "Support ARM v4T instructions">;

class ProcNoItin<string Name, list<SubtargetFeature> Features>
  : Processor<Name, NoItineraries, Features>;

def : ProcNoItin<"arm8",                                [HasV4TOps]>;

//===----------------------------------------------------------------------===//
// Declare the target which we are implementing
//===----------------------------------------------------------------------===//
def WangARMInstrInfo : InstrInfo;

def WangARM : Target {
  let InstructionSet = WangARMInstrInfo;
}
