/**
 * \file AIdis.h
 * \brief Declaration of the AIdis pass
 * \author Julien Henry
 */
#ifndef _AIDIS_H
#define _AIDIS_H

#include <vector>

#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/CFG.h"

#include "AIpass.h"
#include "Sigma.h"

/**
 * \class AIdis
 * \brief Pass that computes disjunctive invariants
 */
class AIdis : public llvm::ModulePass, public AIPass {

	private:
		std::map<llvm::BasicBlock*,Sigma*> S;

		/**
		 * \brief maximum number of disjunct per control point
		 */
		int Max_Disj;

		/**
		 * \brief remembers all the paths that have already been
		 * visited
		 */
		std::map<llvm::BasicBlock*,PathTree*> pathtree;

		std::priority_queue<Node*,std::vector<Node*>,NodeCompare> A_prime;

		/**
		 * \brief Computes the new feasible paths and add them to pathtree
		 * \param n the starting point
		 */
		void computeNewPaths(Node * n);

		/**
		 * \brief computes the sigma function
		 *
		 * see Gulwani and Zuleger's paper : The Reachability Bound problem for
		 * details
		 */
		int sigma(
			std::list<llvm::BasicBlock*> path, 
			int start,
			Abstract * Xtemp,
			bool source);

		void init()
			{
				Max_Disj = 10;
				aman = new AbstractManDisj();
				passID.T = LW_WITH_PF_DISJ;
			}

	public:
		static char ID;	

	public:

		AIdis(char &_ID, Apron_Manager_Type _man, bool _NewNarrow, bool _Threshold) : ModulePass(_ID), AIPass(_man,_NewNarrow, _Threshold) {
			init();
			passID.D = _man;
			passID.N = _NewNarrow;
			passID.TH = _Threshold;
		}
		
		AIdis() : ModulePass(ID) {
			init();
			passID.D = getApronManager();
			passID.N = useNewNarrowing();
			passID.TH = useThreshold();
		}
		

		~AIdis () {
			for (std::map<llvm::BasicBlock*, PathTree*>::iterator 
				it = pathtree.begin(),
				et = pathtree.end(); 
				it != et; 
				it++) {
				if ((*it).second != NULL)
					delete (*it).second;
				}
			}

		const char *getPassName() const;

		void getAnalysisUsage(llvm::AnalysisUsage &AU) const;

		bool runOnModule(llvm::Module &M);

		void computeFunction(llvm::Function * F);

		std::set<llvm::BasicBlock*> getPredecessors(llvm::BasicBlock * b) const;
		std::set<llvm::BasicBlock*> getSuccessors(llvm::BasicBlock * b) const;

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

		void loopiter(
			Node * n, 
			int index,
			int Sigma,
			Abstract * &Xtemp,
			std::list<llvm::BasicBlock*> * path,
			bool &only_join,
			PathTree * const U,
			PathTree * const V);
};

#endif
