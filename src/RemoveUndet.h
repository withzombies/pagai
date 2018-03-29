#ifndef REMOVEUNDET_H
#define REMOVEUNDET_H

#include "llvm/Config/llvm-config.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"
#include "config.h"
#if LLVM_VERSION_ATLEAST(3, 5)
#   include "llvm/IR/InstVisitor.h"
#else
#   include "llvm/InstVisitor.h"
#endif

class RemoveUndet : public llvm::ModulePass {
	
	private:
		std::map<const llvm::Type*, llvm::Constant*> undet_functions;
		
        llvm::Constant* getNondetFn (llvm::Module * M, llvm::Type *type);

	public:
		static char ID;
		RemoveUndet() : llvm::ModulePass(ID) {}

		bool runOnModule(llvm::Module &M);

};

#endif
