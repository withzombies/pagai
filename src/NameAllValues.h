#ifndef NAMEALLVALUES_H
#define NAMEALLVALUES_H
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
#include <set>


using namespace llvm;

class NameAllValues : public FunctionPass, 
	public InstVisitor<NameAllValues, void> {
	// make sure we do not expand twice the same branch inst
	std::set<Value*> seen;
 public:
  static char ID;
  NameAllValues() : FunctionPass(ID) {}

  bool runOnFunction(Function &F);
	
};

#endif
