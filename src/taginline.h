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

class TagInline : public llvm::ModulePass {

	private:	
		static std::vector<const char *> ToAnalyze;

	public:
		static char ID;
		TagInline() : llvm::ModulePass(ID) {}

		bool runOnModule(llvm::Module &M);

		static llvm::ArrayRef<const char *> GetFunctionsToAnalyze();
	
		const char * getPassName() const;

		void getAnalysisUsage(llvm::AnalysisUsage &AU) const;

};
#endif
