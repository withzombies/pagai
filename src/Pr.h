/**
 * \file Pr.h
 * \brief Declaration of the Pr class
 * \author Julien Henry
 */
#ifndef PR_H
#define PR_H

#include <map>
#include <set>

#include "begin_3rdparty.h"
#include "llvm/Analysis/CFG.h"
#include "end_3rdparty.h"

#include "Node.h"

/**
 * \class Pr
 * \brief Pr computation pass
 */
class Pr {

	private:
        llvm::Function * F;

		/**
		 * \brief associate to each formula its set of Pr nodes
		 */
		std::set<llvm::BasicBlock*> Pr_set;

		/**
		 * \brief widening points (subset of Pr where we apply widening)
		 */
		std::set<llvm::BasicBlock*> Pw_set;

		std::set<llvm::BasicBlock*> Assert_set;
		std::set<llvm::BasicBlock*> UndefBehaviour_set;

		std::map<Node*,int> index;
		std::map<Node*,int> lowlink;
		std::map<Node*,bool> isInStack;

		/**
		 * \brief compute the set Pr for a function
		 */
		void computePr();

		bool check_acyclic(std::set<llvm::BasicBlock*>* FPr);
		bool check_acyclic_rec(
				Node * n, 
				int & N,
				std::stack<Node*> * S,
				std::set<llvm::BasicBlock*>* FPr);

		bool computeLoopHeaders(std::set<llvm::BasicBlock*>* FPr);
		bool computeLoopHeaders_rec(
				Node * n, 
				std::set<Node*> * Seen,
				std::set<Node*> * S,
				std::set<llvm::BasicBlock*>* FPr);

		void minimize_Pr();

		/**
		 * \brief private constructor
		 */
		Pr(llvm::Function * F);

	public:
		static char ID;	

		static Pr * getInstance(llvm::Function * F);

		static void releaseMemory();

		~Pr();

		/**
		 * \brief associate to each basicBlock its successors in Pr
		 *
		 * WARNING : this set is filled by SMTpass
		 */
		std::map<llvm::BasicBlock*,std::set<llvm::BasicBlock*> > Pr_succ;
		/**  
		 * \brief associate to each basicBlock its predecessors in Pr
		 *
		 * WARNING : this set is filled by SMTpass
		 */
		std::map<llvm::BasicBlock*,std::set<llvm::BasicBlock*> > Pr_pred;

		/** 
		 * \brief get the set Pr. The set Pr is computed only once
		 */
		std::set<llvm::BasicBlock*>* getPr();

		/** 
		 * \brief get the set Pw. The set Pw is computed only once
		 */
		std::set<llvm::BasicBlock*>* getPw();

		std::set<llvm::BasicBlock*>* getAssert();
		std::set<llvm::BasicBlock*>* getUndefinedBehaviour();

		bool inPr(llvm::BasicBlock * b);
		bool inPw(llvm::BasicBlock * b);
		bool inAssert(llvm::BasicBlock * b);
		bool inUndefBehaviour(llvm::BasicBlock * b);

		/** 
		 * \brief returns a set containing all the predecessors of
		 * b in Pr
		 */
		std::set<llvm::BasicBlock*> getPrPredecessors(llvm::BasicBlock * b);

		/** 
		 * \brief returns a set containing all the successors of
		 * b in Pr
		 */
		std::set<llvm::BasicBlock*> getPrSuccessors(llvm::BasicBlock * b);
};

extern std::map<llvm::Function *, Pr *> PR_instances;

#endif
