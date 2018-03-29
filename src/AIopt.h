/**
 * \file AIopt.h
 * \brief Declaration of the AIopt class
 * \author Julien Henry
 */
#ifndef _AIOPT_H
#define _AIOPT_H

#include <queue>
#include <vector>

#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/CFG.h"

#include "AIpass.h"

/**
 * \class AIopt
 * \brief Path Focusing + Guided Static analysis implementation
 */
class AIopt : public llvm::ModulePass, public AIPass {

	protected:
		/**
		 * \brief remembers all the paths that have already been
		 * visited
		 */
		std::map<llvm::BasicBlock*, PathTree*> pathtree;

		/**
		 * \brief remembers all the paths that have already been
		 * visited in the current ComputeNode call
		 */
		PathTree * W;

		/**
		 * \brief Set of paths U
		 */
		std::map<llvm::BasicBlock*, PathTree*> U;

		/**
		 * \brief Set of paths V
		 */
		std::map<llvm::BasicBlock*, PathTree*> V;

		std::priority_queue<Node*,std::vector<Node*>,NodeCompare> A_prime;

		/**
		 * \brief Computes the new feasible paths and add them to pathtree
		 * \param n the starting point
		 */
		void computeNewPaths(Node * n);

		void init()
			{
				aman = new AbstractManClassic();
				passID.T = LW_WITH_PF;
			}
		
		bool is_SMT_technique() {return true;}

	public:
		static char ID;	

	public:

		AIopt(char &_ID, Apron_Manager_Type _man, bool _NewNarrow, bool _Threshold) : ModulePass(_ID), AIPass(_man,_NewNarrow, _Threshold) {
			init();
			passID.D = _man;
			passID.N = _NewNarrow;
			passID.TH = _Threshold;
		}
		
		AIopt (char &_ID): ModulePass(_ID) {
			init();
			passID.D = getApronManager();
			passID.N = useNewNarrowing();
			passID.TH = useThreshold();
		}

		AIopt() : ModulePass(ID) {
			init();
			passID.D = getApronManager();
			passID.N = useNewNarrowing();
			passID.TH = useThreshold();
		}

		~AIopt () {
			for (std::map<llvm::BasicBlock*, PathTree*>::iterator 
				it = pathtree.begin(),
				et = pathtree.end(); 
				it != et; 
				it++) {
				if ((*it).second != NULL)
					delete (*it).second;
				}
				delete aman;
			}

		/**
		 * \{
		 * \name LLVM pass manager stuff
		 */
		const char *getPassName() const;
		void getAnalysisUsage(llvm::AnalysisUsage &AU) const;
		/**
		 * \}
		 */

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
