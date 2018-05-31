/**
 * \file PathTree.h
 * \brief Declaration of the PathTree class
 * \author Julien Henry
 */
#ifndef _PATHTREE_H
#define _PATHTREE_H

#include <list>
#include <map>
#include <vector>
#include <string>

#include "begin_3rdparty.h"
#include "llvm/IR/BasicBlock.h"

#include "cuddObj.hh"
#include "end_3rdparty.h"

#include "SMTpass.h"

/**
 * \class PathTree
 * \brief BDD representing sets of paths in the graph
 */
class PathTree {

	public:

		virtual ~PathTree()  {};

		/**
		 * \brief insert a path in the Bdd
		 */
		virtual void insert(const std::list<llvm::BasicBlock*> & path, bool primed = false) = 0;

		/**
		 * \brief remove a path from the Bdd
		 */
		virtual void remove(const std::list<llvm::BasicBlock*> & path, bool primed = false) = 0;

		/**
		 * \brief clear the Bdd. The result will be an empty Bdd
		 */
		virtual void clear(bool primed = false) = 0;

		/**
		 * \brief check if the Bdd contains the path given as argument
		 */
		virtual bool exist(const std::list<llvm::BasicBlock*> & path, bool primed = false) = 0;

		/**
		 * \brief merge the two Bdds into Bdd. Bdd_prime is cleared
		 */
		virtual void mergeBDD() = 0;

		virtual bool isZero(bool primed = false) = 0;

		/**
		 * \brief dump the graph
		 */
		virtual void DumpDotBDD(const std::string & filename, bool prime) = 0;

		/**
		 * \brief generate the SMTpass formula associated to the Bdd
		 */
		virtual SMT_expr generateSMTformula(
			SMTpass * smt, bool neg = false) = 0;
};
#endif
