#ifndef NAMEALLVALUES_H
#define NAMEALLVALUES_H

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

class NameAllValues : public llvm::FunctionPass,
	public llvm::InstVisitor<NameAllValues, void>
{
	// make sure we do not expand twice the same branch inst
	std::set<llvm::Value*> seen;

	public:
		static char ID;
		NameAllValues() : llvm::FunctionPass(ID) {}

		bool runOnFunction(llvm::Function &F);

};

#endif
