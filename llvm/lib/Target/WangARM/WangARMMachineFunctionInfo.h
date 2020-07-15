#ifndef __LLVM_TARGET_WANGARM_MACHINEFUNCTIONINFO_HEADER__
#define __LLVM_TARGET_WANGARM_MACHINEFUNCTIONINFO_HEADER__

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/Support/ErrorHandling.h"
#include <utility>

namespace llvm {

/// WangARMFunctionInfo - This class is derived from MachineFunctionInfo and
/// contains private ARM-specific information for each MachineFunction.
class WangARMFunctionInfo : public MachineFunctionInfo {
public:
  virtual void anchor();
  WangARMFunctionInfo::WangARMFunctionInfo(
      MachineFunction &MF);
  unsigned getArgRegsSaveSize() const { return ArgRegsSaveSize; }
  void setArgRegsSaveSize(unsigned s) { ArgRegsSaveSize = s; }
  unsigned getArgumentStackSize() const { return ArgumentStackSize; }
  void setArgumentStackSize(unsigned size) { ArgumentStackSize = size; }
  int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
  void setVarArgsFrameIndex(int Index) { VarArgsFrameIndex = Index; }

private:
  /// VarArgsRegSaveSize - Size of the register save area for vararg functions.
  ///
  unsigned ArgRegsSaveSize = 0;
  /// ArgumentStackSize - amount of bytes on stack consumed by the arguments
  /// being passed on the stack
  unsigned ArgumentStackSize = 0;

  /// VarArgsFrameIndex - FrameIndex for start of varargs area.
  int VarArgsFrameIndex = 0;
};

} // namespace llvm

#endif