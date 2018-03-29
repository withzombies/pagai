/**
 * \file AIpf.h
 * \brief Declaration of the AIpf class
 * \author Julien Henry
 */
#ifndef _AIPF_H
#define _AIPF_H

#include <queue>
#include <vector>

#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/CFG.h"

#include "AIpass.h"

/**
 * \class AIpf 
 * \brief Abstract Interpretation with Path Focusing algorithm (using SMT-solving)
 */
class AIpf : public llvm::ModulePass, public AIPass {

	public:
		static char ID;	

	protected:
		
		std::map<llvm::BasicBlock*, PathTree*> U;
		std::map<llvm::BasicBlock*, PathTree*> V;

		void init()
			{
				aman = new AbstractManClassic();
				passID.T = PATH_FOCUSING;
			}
		
		bool is_SMT_technique() {return true;}

	public:

		AIpf(char &_ID, Apron_Manager_Type _man, bool _NewNarrow, bool _Threshold) : ModulePass(_ID), AIPass(_man,_NewNarrow, _Threshold) {
			init();
			passID.D = _man;
			passID.N = _NewNarrow;
			passID.TH = _Threshold;
		}

		AIpf (char &_ID): ModulePass(_ID) {
			init();
			passID.D = getApronManager();
			passID.N = useNewNarrowing();
			passID.TH = useThreshold();
		}

		AIpf (): ModulePass(ID) {
			init();
			passID.D = getApronManager();
			passID.N = useNewNarrowing();
			passID.TH = useThreshold();
		}

		~AIpf () {}

		const char *getPassName() const;

		void getAnalysisUsage(llvm::AnalysisUsage &AU) const;

		bool runOnModule(llvm::Module &M);

		void computeFunction(llvm::Function * F);

		std::set<llvm::BasicBlock*> getPredecessors(llvm::BasicBlock * b) const;
		std::set<llvm::BasicBlock*> getSuccessors(llvm::BasicBlock * b) const;

		virtual void assert_properties(params P, llvm::Function * F) {}
		virtual void intersect_with_known_properties(Abstract * Xtemp, Node * n, params P) {}

		/**
		 * \brief compute and update the Abstract value of the Node n
		 * \param n the starting point
		 */
		void computeNode(Node * n);
		
		/**
		 * \brief apply narrowing at node n
		 * \param n the starting point
		 */
		void narrowNode(Node * n);
};

#endif
