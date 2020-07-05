/// WangPass.cpp
/// This file contains the source code for the custom LLVM IR pass presented in

#include "llvm/IR/Function.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/PassRegistry.h"
#include "llvm/InitializePasses.h"

using namespace llvm;

namespace {
class WangPass : public FunctionPass {
public:
  static char ID;
  WangPass() : FunctionPass(ID) {}

  virtual bool runOnFunction(Function &F) {
    errs() << "wangpass --- ";
    errs() << F.getName() << ": ";
    errs() << F.getFunctionType()->params().size() << '\n';
    return false;
  }
};
} // namespace

char WangPass::ID = 0;

INITIALIZE_PASS(WangPass,
                "wang-pass",
                "Function Argument Count Pass.", false,
                false)

void llvm::initializeWangPass(PassRegistry &Registry) {
  initializeWangPassPass(Registry);
}