/**
 * \file PathTree.h
 * \brief Declaration of the PathTree class
 * \author Julien Henry
 */
#ifndef _PATHTREE_BR_H
#define _PATHTREE_BR_H

#include <list>
#include <map>
#include <vector>
#include <string>

#include "begin_3rdparty.h"
#include "llvm/IR/BasicBlock.h"

#include "cuddObj.hh"
#include "end_3rdparty.h"

#include "SMTpass.h"
#include "PathTree.h"

/**
 * \class PathTree_br
 * \brief represents set of paths in the graph, by storing the
 * corresponding BranchInst into a BDD
 */
class PathTree_br : public PathTree {

	private:
		/**
		 * \brief manager of the CUDD library 
		 */
		Cudd * mgr;
		
		/**
		 * \{
		 * \name variables needed by some methods
		 */
		DdNode * background;
		DdNode * zero;
		/** 
		 * \}
		 */

		/**
		 * \brief stores the index of the basicBlock in the BDD
		 */
		std::map<llvm::BranchInst*, int> BddVar;
		/**
		 * \brief stores the index of the source basicBlock in the BDD
		 */
		std::map<llvm::BranchInst*, int> BddVarStart;

		std::map<int, llvm::BranchInst*> levels;

		BDD computef(std::list<llvm::BasicBlock*> path);

		/**
		 * \brief Bdd that stores the various seen paths
		 */
		BDD * Bdd;

		/** 
		 * \brief Bdd that stores the paths that need to be added in Bdd
		 * in the next step
		 */
		BDD * Bdd_prime;

		/**
		 * \brief number of levels in the BDD
		 */
		int BddIndex;

		BDD getBDDfromBddIndex(int n);
		/**
		 * \brief returns the BDD node associated to a specific
		 * BasicBlock. 
		 *
		 * When considering the source BasicBlock, the map should be
		 * BddVarStart, else it should be BddVar
		 */
		BDD getBDDfromBranchInst(llvm::BranchInst * b, std::map<llvm::BranchInst*, int> &map, int &n);


        llvm::BranchInst * getConditionnalBranch(llvm::BasicBlock * b, bool start = false);

		/**
		 * \brief returns the name of the basicBlock associated
		 * to the level i of the Bdd.
		 */
		const std::string getStringFromLevel(int i);
		
        llvm::BranchInst * getBranchFromLevel(int const i);

		void createBDDVars(llvm::BasicBlock * Start, std::set<llvm::BasicBlock*> * Pr, std::map<llvm::BranchInst*, int> &map, std::set<llvm::BasicBlock*> * seen, bool start = false);

		/**
		 * \brief dump the BDD "graph" in a .dot file. 
		 * \param filename Name of the .dot file
		 */
		void DumpDotBDD(BDD graph, std::string filename);

	public:
		PathTree_br(llvm::BasicBlock * Start);

		~PathTree_br();

		/**
		 * \brief insert a path in the Bdd
		 */
		void insert(std::list<llvm::BasicBlock*> path, bool primed = false);

		/**
		 * \brief remove a path from the Bdd
		 */
		void remove(std::list<llvm::BasicBlock*> path, bool primed = false);

		/** 
		 * \brief clear the Bdd. The result will be an empty Bdd
		 */
		void clear(bool primed = false);

		/** 
		 * \brief check if the Bdd contains the path given as argument
		 */
		bool exist(std::list<llvm::BasicBlock*> path, bool primed = false);

		/** 
		 * \brief merge the two Bdds into Bdd. Bdd_prime is cleared
		 */
		void mergeBDD();

		bool isZero(bool primed = false);

		/** 
		 * \brief dump the graph
		 */
		void DumpDotBDD(std::string filename, bool prime);

		/** 
		 * \brief generate the SMTpass formula associated to the Bdd
		 */
		SMT_expr generateSMTformula(
			SMTpass * smt, bool neg = false);
};
#endif
