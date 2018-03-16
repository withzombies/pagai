#ifndef EXPANDEQUALITIES_H
#define EXPANDEQUALITIES_H
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

class ExpandEqualities : public FunctionPass, 
	public InstVisitor<ExpandEqualities, void> {
	// make sure we do not expand twice the same branch inst
	std::set<Value*> seen;
 public:
  static char ID;
  ExpandEqualities() : FunctionPass(ID) {}

  bool runOnFunction(Function &F);

  void visitBranchInst(BranchInst &I);

 private:
  TerminatorInst * SplitBlockAndInsertIfThen(Value *Cond,
		Instruction *SplitBefore,
		bool Unreachable,
		MDNode *BranchWeights = NULL);
	
};

#endif
