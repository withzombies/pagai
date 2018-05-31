#ifndef IDENTIFYLOOPS_H
#define IDENTIFYLOOPS_H

#include <set>

#include "begin_3rdparty.h"
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
#include "end_3rdparty.h"

extern std::set<llvm::BasicBlock *> Loop_headers;

class IdentifyLoops : public llvm::FunctionPass {

	public:
		static char ID;

		IdentifyLoops() : llvm::FunctionPass(ID) {}

		bool runOnFunction(llvm::Function &F);

		virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const;

};

#endif
