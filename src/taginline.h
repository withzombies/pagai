#ifndef TAGINLINE_H
#define TAGINLINE_H

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
