/**
 * \file Sigma.h
 * \brief Declaration of the Sigma class
 * \author Julien Henry
 */
#ifndef SIGMA_H
#define SIGMA_H

#include <list>
#include <map>

#include "begin_3rdparty.h"
#include "llvm/IR/BasicBlock.h"

#include "cuddObj.hh"
#include "end_3rdparty.h"

/**
 * \class Sigma
 * \brief class used by AIdis for computing the sigma function
 *
 * This class uses ADD from the CUDD library
 */
class Sigma {

	private:
		/**
		 * \brief manager of the CUDD library
		 */
		Cudd * mgr;

		/**
		 * \brief maximum number of disjuncts
		 */
		int Max_Disj;

		void init(llvm::BasicBlock * Start);

		void createADDVars(llvm::BasicBlock * Start, const std::set<llvm::BasicBlock*> & Pr, std::map<llvm::BasicBlock*, int> & map, std::set<llvm::BasicBlock*> & seen, bool start = false);

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
		 * \brief stores the index of the basicBlock in the ADD
		 */
		std::map<llvm::BasicBlock*, int> AddVar;

		/**
		 * \brief stores the index of the source basicBlock in the ADD
		 */
		std::map<llvm::BasicBlock*, int> AddVarSource;

		/**
		 * \brief number of levels in the ADD
		 */
		int AddIndex;

		ADD getADDfromAddIndex(int n);

		/**
		 * \brief get the ADD node associated to a specific basicblock
		 *
		 * the map should be AddVar or AddVarSource, depending if we consider
		 * the starting point or not
		 */
		ADD getADDfromBasicBlock(llvm::BasicBlock * b, std::map<llvm::BasicBlock*, int> & map);
		ADD getADDfromBasicBlock(llvm::BasicBlock * b, std::map<llvm::BasicBlock*, int> & map, int &n);

		ADD computef(const std::list<llvm::BasicBlock*> & path);

		/**
		 * \brief Add that stores the various seen paths
		 */
		std::map<int, ADD*> Add;

		/**
		 * \brief insert a path in the Bdd
		 */
		void insert(const std::list<llvm::BasicBlock*> & path, int start);

		/**
		 * \brief remove a path from the Bdd
		 */
		void remove(const std::list<llvm::BasicBlock*> & path, int start);

		/**
		 * \brief check if the Bdd contains the path given as argument
		 */
		bool exist(const std::list<llvm::BasicBlock*> & path, int start);

		/**
		 * \brief get the actual value of sigma stored in the BDD for sigma(path,start)
		 */
		int getActualValue(const std::list<llvm::BasicBlock*> & path, int start);

		/**
		 * \brief set the value of sigma(path,start)
		 */
		void setActualValue(const std::list<llvm::BasicBlock*> & path, int start, int value);

		void DumpDotADD(ADD graph, const std::string & filename);

		bool isZero(int start);

	public:

		Sigma(llvm::BasicBlock * Start, int _Max_Disj);
		Sigma(llvm::BasicBlock * Start);
		~Sigma();

		/**
		 * \brief clear the Add.
		 *
		 * The result will be an empty Add
		 */
		void clear();

		int getSigma(
			std::list<llvm::BasicBlock*> & path,
			int start,
			Abstract * Xtemp,
			AIPass * pass);

};
#endif
