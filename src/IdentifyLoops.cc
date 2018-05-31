#ifndef GLOBALTOLOCAL_H
#define GLOBALTOLOCAL_H

#include "begin_3rdparty.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Analysis/LoopInfo.h"
#include "end_3rdparty.h"

#include "IdentifyLoops.h"

using namespace llvm;

std::set<BasicBlock *> Loop_headers;

char IdentifyLoops::ID = 0;

void IdentifyLoops::getAnalysisUsage(AnalysisUsage &AU) const {
	AU.addRequired<LoopInfo>();
	AU.setPreservesAll();
}

bool IdentifyLoops::runOnFunction(Function &F) {
	LoopInfo * LI = &getAnalysis<LoopInfo>();

	for (Function::iterator it = F.begin(); it != F.end(); ++it) {
		BasicBlock * b = it;
		if (LI->isLoopHeader(b)) {
			Loop_headers.insert(b);
		}
	}
	return false;
}

static RegisterPass<IdentifyLoops> X("identifyloops", "search for loop headers", false, true);

#endif
