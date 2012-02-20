#ifndef _AISIMPLE_H
#define _AISIMPLE_H

#include "llvm/Module.h"
#include "llvm/Pass.h"
#include "AIpass.h"

using namespace llvm;

/// @brief Base class implementing the basic abstract interpretation algorithm
///
/// This class contains the parts of the algorimth common to
/// algorithms that do not use SMTpass solving to chose the order in which
/// nodes are visited (i.e. Classic, and Gopan&Reps).
class AISimple : public ModulePass, public AIPass {

	public:

		AISimple (char &_ID, Apron_Manager_Type _man, bool _NewNarrow) : ModulePass(_ID), AIPass(_man,_NewNarrow) {}

		AISimple (char & ID) : ModulePass(ID) {}

		~AISimple () {}
		/// Apply the simple abstract interpretation algorithm
		/// (ascending iterations + narrowing) on function F.
		void computeFunc(Function * F);

		std::set<BasicBlock*> getPredecessors(BasicBlock * b) const;
		std::set<BasicBlock*> getSuccessors(BasicBlock * b) const;

		/// @{
		/// @name LLVM pass manager stuff
		virtual const char *getPassName() const = 0;
		void getAnalysisUsage(AnalysisUsage &AU) const;
		bool runOnModule(Module &M);
		/// @}

		/// computeNode - compute and update the Abstract value of the Node n
		void computeNode(Node * n);
		
		/// narrowNode - apply narrowing at node n
		void narrowNode(Node * n);

		/// Simple wrapper around AISimple::computeFunc()
		void computeFunction(Function * F);
};

#endif
