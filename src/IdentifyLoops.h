#ifndef IDENTIFYLOOPS_H
#define IDENTIFYLOOPS_H

#include <set>

#include "llvm/Config/llvm-config.h"
#include "llvm/Analysis/CFG.h"
#include "config.h"
#if LLVM_VERSION_ATLEAST(3, 5)
#   include "llvm/IR/InstVisitor.h"
#else
#   include "llvm/InstVisitor.h"
#endif
#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"


using namespace llvm;

extern std::set<BasicBlock *> Loop_headers;

class IdentifyLoops : public FunctionPass {
 public:
  static char ID;
  IdentifyLoops() : FunctionPass(ID) {}

  bool runOnFunction(Function &F);
		
  virtual void getAnalysisUsage(AnalysisUsage &AU) const;
	
};

#endif

