set(LLVM_TARGET_DEFINITIONS WangARM.td)

tablegen(LLVM WangARMGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM WangARMGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM WangARMGenCallingConv.inc -gen-callingconv)
tablegen(LLVM WangARMGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM WangARMGenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM WangARMGenFastISel.inc -gen-fast-isel)
tablegen(LLVM WangARMGenGlobalISel.inc -gen-global-isel)
tablegen(LLVM WangARMGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM WangARMGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM WangARMGenMCPseudoLowering.inc -gen-pseudo-lowering)
tablegen(LLVM WangARMGenRegisterBank.inc -gen-register-bank)
tablegen(LLVM WangARMGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM WangARMGenSubtargetInfo.inc -gen-subtarget)
#tablegen(LLVM WangARMGenSystemRegister.inc -gen-searchable-tables)

add_public_tablegen_target(WangARMCommonTableGen)

add_llvm_target(WangARMCodeGen
	WangARMTargetMachine.cpp
	WangARMISelDAGToDAG.cpp
	WangARMISelLowering.cpp
	WangARMAsmPrinter.cpp
	WangARMSubtarget.cpp
	WangARMFrameLowering.cpp
	WangARMInstrInfo.cpp
	WangARMRegisterInfo.cpp
	WangARMMachineFunctionInfo.cpp
)

add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)