/**
 * \file instrOverflow.h
 * \brief LLVM pass that instruments LLVM IR for overflow checking
 * \author Julien Henry
 */
#ifndef _INSTROVERFLOW_H
#define _INSTROVERFLOW_H

#include <map>

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

class instrOverflow : public llvm::FunctionPass,
	public llvm::InstVisitor<instrOverflow, bool> {

		private:
			std::map<llvm::CallInst*, llvm::Instruction*> replaced;

			bool pass1;

		public:
			static char ID;
			instrOverflow() : llvm::FunctionPass(ID) {}
			void getAnalysisUsage(llvm::AnalysisUsage &AU) const;

			bool runOnFunction(llvm::Function &F);
			bool updateFunction(llvm::Function &F);

			void replaceWithUsualOp(
					llvm::Instruction * old,
					unsigned intrinsicID,
					const std::vector<llvm::Value*> & args,
					llvm::CallInst * intrinsicCall
			);

			void replaceWithCmp(
					llvm::Instruction * old,
					unsigned intrinsicID,
					llvm::CallInst * intrinsicCall
			);

			bool visitExtractValueInst(llvm::ExtractValueInst &inst);
			bool visitBranchInst(llvm::BranchInst &inst);
			bool visitCallInst(llvm::CallInst &inst);

			bool visitInstruction(llvm::Instruction &inst) { (void) inst; return false; }
	};


#endif
