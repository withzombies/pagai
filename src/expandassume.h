#ifndef EXPANDASSUME_H
#define EXPANDASSUME_H
#include "llvm/Config/llvm-config.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "config.h"
#if LLVM_VERSION_ATLEAST(3, 5)
#   include "llvm/IR/InstVisitor.h"
#else
#   include "llvm/InstVisitor.h"
#endif
#include <set>


using namespace llvm;

class ExpandAssume : public FunctionPass {
	// make sure we do not expand twice the same branch inst
	std::set<Value*> seen;
 public:
  static char ID;
  ExpandAssume() : FunctionPass(ID) {}

  bool runOnFunction(Function &F);
  bool stepFunction(Function &F);

  bool visitCallInst(CallInst &CI);

 private:
  TerminatorInst * SplitBlockAndInsertIfThen(Value *Cond,
		Instruction *SplitBefore,
		bool Unreachable,
		MDNode *BranchWeights = NULL);
	
};

#endif
