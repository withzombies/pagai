#ifndef _PATHTREE_H
#define _PATHTREE_H

#include <list>
#include <map>
#include <vector>
#include <string>
#include "llvm/BasicBlock.h"

#include "cuddObj.hh"

#include "SMT.h"

using namespace llvm;

class PathTree {

	private:
		/// mgr - manager of the CUDD library 
		Cudd * mgr;
		
		/// variables needed by some methods
		DdNode * background, * zero;

		/// BddVar - stores the index of the basicBlock in the BDD
		std::map<BasicBlock*,int> BddVar;
		/// BddVarStart - stores the index of the source basicBlock in the BDD
		std::map<BasicBlock*,int> BddVarStart;

		std::map<int, BasicBlock*> levels;

		/// Bdd - Bdd that stores the various seen paths
		BDD Bdd;

		/// Bdd_prime - Bdd that stores the paths that need to be added in Bdd
		/// in the next step
		BDD Bdd_prime;

		/// BddIndex - number of levels in the BDD
		int BddIndex;

		/// getBDDfromBasicBlock - returns the BDD node associated to a specific
		/// BasicBlock. When considering the source BasicBlock, the map is
		/// BddVarStart, else it is BddVar
		BDD getBDDfromBasicBlock(BasicBlock * b,std::map<BasicBlock*,int> &map);

		/// getNodeName - returns a string that names the basicblock. 
		/// If smt != NULL, this name is exactly the same as the one 
		/// used in the SMT pass
		std::string getNodeName(
			BasicBlock* b, 
			bool src,
			SMT * smt = NULL);

		/// getStringFromLevel - returns the name of the basicBlock associated
		/// to the level i of the Bdd.
		/// If smt != NULL, this name is exactly the same as the one 
		/// used in the SMT pass
		std::string getStringFromLevel(
			int i,
			SMT * smt = NULL);

		/// generateSMTformulaAux - method used by generateSMTformula
		void generateSMTformulaAux(
			SMT * smt,
			DdNode * node /* current node */,
			std::vector<int> list /* current recursion path */,
			std::vector<SMT_expr> &disjunct);

	public:
		PathTree();

		~PathTree();

		/// insert - insert a path in the Bdd
		void insert(std::list<BasicBlock*> path, bool primed = false);

		/// remove - remove a path from the Bdd
		void remove(std::list<BasicBlock*> path, bool primed = false);

		/// clear - clear the Bdd. The result will be an empty Bdd
		void clear(bool primed = false);

		/// exist - check if the Bdd contains the path given as argument
		bool exist(std::list<BasicBlock*> path, bool primed = false);

		/// merge - merge the two Bdds into Bdd. Bdd_prime is cleared
		void mergeBDD();

		bool isZero(bool primed = false);

		/// DumpDotBDD - dump the BDD "graph" in a .dot file. Name of the .dot
		/// file is given by the filename argument.
		void DumpDotBDD(BDD graph, std::string filename);

		/// generateSMTformula - generate the SMT formula associated to the Bdd
		SMT_expr generateSMTformula(
			SMT * smt);
};
#endif
