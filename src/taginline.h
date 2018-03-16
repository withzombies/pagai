#ifndef TAGINLINE_H
#define TAGINLINE_H

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

using namespace llvm;

class TagInline : public ModulePass {

	private:	
		static std::vector<const char *> ToAnalyze;

	public:
		static char ID;
		TagInline() : ModulePass(ID) {}

		bool runOnModule(Module &M);

		static ArrayRef<const char *> GetFunctionsToAnalyze();
	
		const char * getPassName() const;

		void getAnalysisUsage(AnalysisUsage &AU) const;

};
#endif
