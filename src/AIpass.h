/**
 * \file AIpass.h
 * \brief Declaration of the AIpass class
 * \author Julien Henry
 */
#ifndef _AIPASS_H
#define _AIPASS_H

#include <queue>
#include <vector>

#include "config.h"

#include "begin_3rdparty.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/IR/Constants.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/AliasSetTracker.h"
#if LLVM_VERSION_ATLEAST(3, 5)
#   include "llvm/IR/InstVisitor.h"
#else
#   include "llvm/InstVisitor.h"
#endif
#include "end_3rdparty.h"

#include "Analyzer.h"
#include "apron.h"
#include "PathTree.h"
#include "Constraint.h"
#include "AbstractMan.h"
#include "AnalysisPass.h"

class SMTpass;
class Live;
class Node;

/**
 * \class AIPass
 * \brief Base class for abstract interpretation
 *
 * Base class factoring helper functions and data-structure to
 * perform Abstract Interpretation (i.e. graph traversal on the CFG,
 * Apron Manager, SMTpass solver, ...).
 */
class AIPass : public AnalysisPass, private llvm::InstVisitor<AIPass> {

	friend class llvm::InstVisitor<AIPass>;

	protected:
		/**
		 * \brief access to the Live pass
		 */
		Live * LV;

		/* in case we want an alias analysis
		 * AliasAnalysis * AA;
		 * AliasSetTracker *AST;
		 */

		/**
		 * \brief array of lincons we use to do widening with threshold
		 *
		 * this array is computed in computeTransform
		 */
		Constraint_array * threshold;

		/**
		 * \brief true iff threshold is empty and cannot be cleared
		 */
		bool threshold_empty;

		/**
		 * \brief set to true when the analysis fails (timeout, ...)
		 */
		bool unknown;

		/**
		 * \brief if true, apply Halbwach's narrowing
		 */
		bool NewNarrowing;

		/**
		 * \brief if true, apply widening with threshold
		 * instead of classic widening
		 */
		bool use_threshold;

		/**
		 * \brief path we currently focus on
		 */
		std::vector<llvm::BasicBlock*> focuspath;

		/**
		 * \brief index in focuspath of the focuspath's basicblock we are working on
		 */
		unsigned focusblock;

		/**
		 * \brief list of all the constraints that need to be satisfied
		 * along the path
		 */
		std::list<std::vector<Constraint*>*> constraints;

		/**
		 * \brief set of Phi variables with their associated expression,
		 * that are defined in the last basicblock of the path
		 */
		phivar PHIvars_prime;

		/**
		 * \brief set of Phi variables with their associated expression,
		 * that are defined in the "middle" of the path
		 * (i.e. not at the last basicblock)
		 */
		phivar PHIvars;

		/**
		 * \brief list of active Nodes, that still have to be computed
		 */
		std::priority_queue<Node*,std::vector<Node*>,NodeCompare> A;

		/**
		 * \brief remembers the Nodes that don't need to be recomputed.
		 * This is used to remove duplicates in the A list.
		 */
		std::map<Node*,bool> is_computed;

		/**
		 * \brief apron manager we use along the pass
		 */
		ap_manager_t* man;

		/**
		 * \brief manager that creates abstract values
		 */
		AbstractMan* aman;

		/**
		 * \brief result of the SMTpass pass
		 */
		SMTpass * LSMT;


		/**
		 * \brief assert in the SMT formula the invariants found by a previous
		 * analysis
		 * \param P parameters of the previous analysis. This analysis has to be
		 * run before calling this function
		 * \param F the function
		 */
		void assert_invariant(
				params P,
				llvm::Function * F
				);

		/**
		 * \brief returns false iff the technique computes an invariant at
		 * each control point
		 */
		virtual bool is_SMT_technique() {return false;}

	public:

		AIPass (Apron_Manager_Type _man, bool use_New_Narrowing, bool _use_Threshold) :
			LV(NULL),
			unknown(false),
			NewNarrowing(use_New_Narrowing),
			use_threshold(_use_Threshold),
			LSMT(NULL) {
				man = create_manager(_man);
				init();
			}

		AIPass () :
			LV(NULL),
			unknown(false),
			LSMT(NULL) {
				man = create_manager(getApronManager());
				NewNarrowing = useNewNarrowing();
				use_threshold = useThreshold();
				init();
			}

		void init() {
				init_apron();
				Environment empty_env;
				threshold = new Constraint_array();
				threshold_empty = false;
		}

		virtual ~AIPass () {
				ap_manager_free(man);
				if (!threshold_empty)
					delete threshold;
			}

		/**
		 * \brief print a path on standard output
		 */
		static void printPath(std::list<llvm::BasicBlock*> path);
	protected:

		/**
		 * \brief add all function's arguments into the environment of the first bb
		 */
		void addFunctionArgumentsTo(Node * n, llvm::Function * F);

		virtual void computeFunction(llvm::Function * F) = 0;

		/**
		 * \brief compute and update the Abstract value of the Node n
		 * This function should update the set A of active nodes to
		 * reflect changes performed on Node n.
		 */
		virtual void computeNode(Node * n) = 0;

		/**
		 * \brief apply narrowing at node n
		 * This function should update the set A of active nodes to
		 * reflect changes performed on Node n.
		 */
		virtual void narrowNode(Node * n) = 0;

		/**
		 * \brief Basic abstract interpretation ascending iterations
		 * (iterates over the nodes, calling computeNode for each of
		 * them)
		 */
		virtual void ascendingIter(Node * n, bool dont_reset = false);

		/**
		 * \brief Narrowing algorithm (iterates over the nodes, calling
		 * narrowNode() for each of them)
		 */
		virtual void narrowingIter(Node * n);

		void loopiter(
			Node * n,
			Abstract * &Xtemp,
			std::list<llvm::BasicBlock*> * path,
			bool &only_join,
			PathTree * const U,
			PathTree * const V);

		/**
		 * \brief delete all pathtrees inside the map and clear the map
		 */
		void ClearPathtreeMap(std::map<llvm::BasicBlock*,PathTree*> & pathtree);

		/**
		 * \{
		 * \name copy methods
		 *
		 * \brief copy the elements in X_d into X_s abstract values
		 * return true iff there there some Xd values that were not equal to Xs
		 * same principle for the two other functions
		 */
		bool copy_Xd_to_Xs(llvm::Function * F);
		void copy_Xs_to_Xf(llvm::Function * F);
		void copy_Xf_to_Xs(llvm::Function * F);
		/**
		 * \}
		 */

		/**
		 * \brief computes in Xtemp the polyhedra resulting from
		 * the transformation through the path
		 */
		void computeTransform (
			AbstractMan * aman,
			std::list<llvm::BasicBlock*> path,
			Abstract *Xtemp);

		/**
		 * \brief compute Seeds for Halbwach's narrowing
		 * returns true iff one ore more seeds have been found
		 */
		bool computeNarrowingSeed(llvm::Function * F);

		/**
		 * \brief compute the new environment of Node n, based on
		 * its intVar and RealVar maps
		 */
		void computeEnv(Node * n);

		/**
		 * \brief creates the constraint arrays resulting from a
		 * value.
		 */
		bool computeCondition(llvm::Value * val,
				bool result,
				size_t cons_index,
				std::vector< std::vector<Constraint*> * > * cons);

		/**
		 * \brief creates the constraint arrays resulting from a
		 * comparison instruction.
		 */
		bool computeCmpCondition(llvm::CmpInst * inst,
				bool result,
				size_t cons_index,
				std::vector< std::vector<Constraint*> * > * cons);

		/**
		 * \brief creates the constraint arrays resulting from a
		 * Constant integer
		 */
		bool computeConstantCondition(llvm::ConstantInt * inst,
				bool result,
				size_t cons_index,
				std::vector< std::vector<Constraint*> * > * cons);

		/**
		 * \brief creates the constraint arrays resulting from a
		 * boolean PHINode
		 */
		bool computePHINodeCondition(llvm::PHINode * inst,
				bool result,
				size_t cons_index,
				std::vector< std::vector<Constraint*> * > * cons);

		/**
		 * \brief creates the constraint arrays resulting from a
		 * boolean Binary Operator
		 */
		bool computeBinaryOpCondition(llvm::BinaryOperator * inst,
				bool result,
				size_t cons_index,
				std::vector< std::vector<Constraint*> * > * cons);

		/**
		 * \brief creates the constraint arrays resulting from a
		 * cast between a boolean and an integer
		 */
		bool computeCastCondition(llvm::CastInst * inst,
				bool result,
				size_t cons_index,
				std::vector< std::vector<Constraint*> * > * cons);

		/**
		 * \brief Insert all the dimensions of the environment into the node
		 * variables of n
		 */
		void insert_env_vars_into_node_vars(Environment * env, Node * n, llvm::Value * V);

		/**
		 * \brief initialize the function by creating the Node
		 * objects, and computing the strongly connected components.
		 */
		void initFunction(llvm::Function * F);

		/**
		 * \brief free internal data after the analysis of a
		 * function
		 * Has to be called after the analysis of each function
		 */
		void TerminateFunction(llvm::Function * F);

		/**
		 * \brief print a basicBlock on standard output
		 */
		static void printBasicBlock(llvm::BasicBlock * b);

		/**
		 * \brief process the sequence of positions where an invariant has to be
		 * displayed
		 */
		void computeResultsPositions(
			llvm::Function * F,
			std::map<std::string, std::multimap<std::pair<int, int>, llvm::BasicBlock*> > & files
		);

		/**
		 * \brief inserts pagai invariants into the LLVM Module
		 */
		void InstrumentLLVMBitcode(llvm::Function * F);

		/**
		 * \brief print an invariant on oss, with an optional padding
		 */
		void printInvariant(llvm::BasicBlock * b, std::string left, llvm::raw_ostream * oss);

		/**
		 * \brief computes the set of predecessors for a BasicBlock
		 */
		virtual std::set<llvm::BasicBlock*> getPredecessors(llvm::BasicBlock * b) const = 0;

		/**
		 * \brief computes the set of Successors for a BasicBlock
		 */
		virtual std::set<llvm::BasicBlock*> getSuccessors(llvm::BasicBlock * b) const = 0;

	private:
		/**
		 * \brief Print canonized forms of invariant lists (apron's canonicalize seems not to do the job)
		 *
		 * The terms are in-order from run to run, and the inequalities are in-order from run to run.
		 */
		void printCanonizedInvariant(const Abstract * abs, llvm::raw_ostream & stream, std::string * left = NULL) const;

		void visitInstAndAddVarIfNecessary(llvm::Instruction &I);

		/** \{
		 *  \name Visit methods
		 */
		void visitReturnInst (llvm::ReturnInst &I);
		void visitBranchInst (llvm::BranchInst &I);
		void visitSwitchInst (llvm::SwitchInst &I);
		void visitIndirectBrInst (llvm::IndirectBrInst &I);
		void visitInvokeInst (llvm::InvokeInst &I);
#if LLVM_VERSION_MAJOR < 3 || LLVM_VERSION_MINOR == 0
		void visitUnwindInst (llvm::UnwindInst &I);
#endif
		void visitUnreachableInst (llvm::UnreachableInst &I);
		void visitICmpInst (llvm::ICmpInst &I);
		void visitFCmpInst (llvm::FCmpInst &I);
		void visitAllocaInst (llvm::AllocaInst &I);
		void visitLoadInst (llvm::LoadInst &I);
		void visitStoreInst (llvm::StoreInst &I);
		void visitGetElementPtrInst (llvm::GetElementPtrInst &I);
		void visitPHINode (llvm::PHINode &I);
		void visitTruncInst (llvm::TruncInst &I);
		void visitZExtInst (llvm::ZExtInst &I);
		void visitSExtInst (llvm::SExtInst &I);
		void visitFPTruncInst (llvm::FPTruncInst &I);
		void visitFPExtInst (llvm::FPExtInst &I);
		void visitFPToUIInst (llvm::FPToUIInst &I);
		void visitFPToSIInst (llvm::FPToSIInst &I);
		void visitUIToFPInst (llvm::UIToFPInst &I);
		void visitSIToFPInst (llvm::SIToFPInst &I);
		void visitPtrToIntInst (llvm::PtrToIntInst &I);
		void visitIntToPtrInst (llvm::IntToPtrInst &I);
		void visitBitCastInst (llvm::BitCastInst &I);
		void visitSelectInst (llvm::SelectInst &I);
		void visitCallInst(llvm::CallInst &I);
		void visitVAArgInst (llvm::VAArgInst &I);
		void visitExtractElementInst (llvm::ExtractElementInst &I);
		void visitInsertElementInst (llvm::InsertElementInst &I);
		void visitShuffleVectorInst (llvm::ShuffleVectorInst &I);
		void visitExtractValueInst (llvm::ExtractValueInst &I);
		void visitInsertValueInst (llvm::InsertValueInst &I);
		void visitTerminatorInst (llvm::TerminatorInst &I);
		void visitBinaryOperator (llvm::BinaryOperator &I);
		void visitCmpInst (llvm::CmpInst &I);
		void visitCastInst (llvm::CastInst &I);


		void visitInstruction(llvm::Instruction &I) {
			llvm::ferrs() << I;
			assert(0 && "Instruction not interpretable yet!");
		}
		/**
		 * \}
		 */
};

extern AIPass * CurrentAIpass;

#endif
