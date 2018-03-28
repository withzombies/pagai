/**
 * \file GenerateSMT.h
 * \brief Declaration of the GenerateSMT class
 * \author Julien Henry
 */
#ifndef GENERATE_SMT_H
#define GENERATE_SMT_H

#include "begin_3rdparty.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/CFG.h"
#include "end_3rdparty.h"

#include "SMT_manager.h"
#include "SMTpass.h"

/**
 * \class GenerateSMT
 * \brief Pass that only computes the SMT-formula and outputs it
 */
class GenerateSMT : public llvm::ModulePass {
	
	private:
		SMTpass * LSMT;
        llvm::DominatorTree * DT;

	public:
		static char ID;

		GenerateSMT();
		~GenerateSMT();

		const char * getPassName() const;
		void getAnalysisUsage(llvm::AnalysisUsage &AU) const;
		bool runOnModule(llvm::Module &M);
		bool runOnFunction(llvm::Function &F);

		void printBasicBlock(llvm::BasicBlock* b);

};

#endif
