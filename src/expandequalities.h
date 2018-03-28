#ifndef EXPANDEQUALITIES_H
#define EXPANDEQUALITIES_H

#include <set>

#include "config.h"

#include "begin_3rdparty.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#if LLVM_VERSION_ATLEAST(3, 5)
#   include "llvm/IR/InstVisitor.h"
#else
#   include "llvm/InstVisitor.h"
#endif
#include "end_3rdparty.h"

class ExpandEqualities : public llvm::FunctionPass, 
	public llvm::InstVisitor<ExpandEqualities, void> {
	// make sure we do not expand twice the same branch inst
	std::set<llvm::Value*> seen;
 public:
  static char ID;
  ExpandEqualities() : llvm::FunctionPass(ID) {}

  bool runOnFunction(llvm::Function &F);

  void visitBranchInst(llvm::BranchInst &I);

 private:
  llvm::TerminatorInst * SplitBlockAndInsertIfThen(llvm::Value *Cond,
		llvm::Instruction *SplitBefore,
		bool Unreachable,
		llvm::MDNode *BranchWeights = NULL);
	
};

#endif
