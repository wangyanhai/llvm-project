#include "llvm/Transforms/Scalar.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Pass.h"
using namespace llvm;

namespace{
	struct WangDCE : public FunctionPass {
		static char ID; // Pass identification, replacement for typeid
		WangDCE() : FunctionPass(ID) {
			initializeWangDCEPass(*PassRegistry::getPassRegistry());
		}
		bool runOnFunction(Function& F) override;
		void getAnalysisUsage(AnalysisUsage& AU) const override {
			AU.setPreservesCFG();
		}
	};	
}

char WangDCE::ID = 0;
INITIALIZE_PASS(WangDCE, "wang-dec", "My Aggressive Dead CodeElimination", false, false)

bool WangDCE::runOnFunction(Function &F) {
  if (skipFunction(F))
    return false;
  SmallPtrSet<Instruction *, 32> Alive;
  SmallVector<Instruction *, 32> Worklist;
  // 收集已知的根指令
  //for (Instruction &I : inst_range(F)) {
  //  if (/*isa<TerminatorInst>(I) ||*/ isa<DbgInfoIntrinsic>(I) ||
  //      isa<LandingPadInst>(I) || I.mayHaveSideEffects()) {
  //    Alive.insert(&I);
  //    Worklist.push_back(&I);
  //  }
  //}
  //// 向后传播生存性（liveness）
  //while (!Worklist.empty()) {
  //  Instruction Curr = Worklist.pop_back_val();
  //  for (Use &OI : Curr->operands()) {
  //    if (Instruction Inst = dyn_cast<Instruction>(OI))
  //      if (Alive.insert(Inst).second)
  //        Worklist.push_back(Inst);
  //  }
  //}
  //// 在这个Pass中，不在生存集合中的指令被认为是无用的。不影响控制流、返回值，以及没
  ////有任何副作用的指令直接删除
  //for (Instruction &I : inst_range(F)) {
  //  if (!Alive.count(&I)) {
  //    Worklist.push_back(&I);
  //    I.dropAllReferences();
  //  }
  //}
  //for (Instruction &I : Worklist) {
  //  I->eraseFromParent();
  //}
  //return !Worklist.empty();
  return false;
}

FunctionPass* llvm::createWangDCEPass() { return new WangDCE(); }