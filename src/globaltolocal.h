#ifndef GLOBALTOLOCAL_H
#define GLOBALTOLOCAL_H

#include "begin_3rdparty.h"
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
#include "end_3rdparty.h"

class GlobalToLocal : public llvm::ModulePass {
 public:
  static char ID;
  GlobalToLocal() : llvm::ModulePass(ID) {}

  bool runOnModule(llvm::Module &M);
	
  bool hasOnlyOneFunction(llvm::Module &M);

  bool replaceAllUsesInFunction(llvm::Function * F, llvm::Value * oldvalue, llvm::Value * newvalue);
};

#endif
