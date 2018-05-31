/**
 * \file SMTpass.h
 * \brief Declaration of the SMTpass class
 * \author Julien Henry
 */
#ifndef SMT_H
#define SMT_H

#include <map>
#include <vector>
#include <list>
#include <set>

#include "config.h"
#include "begin_3rdparty.h"
#if LLVM_VERSION_ATLEAST(3, 5)
#   include "llvm/IR/InstVisitor.h"
#else
#   include "llvm/InstVisitor.h"
#endif
#include "end_3rdparty.h"

#include "Analyzer.h"
#include "Abstract.h"
#include "Node.h"
#include "AbstractDisj.h"
#include "SMT_manager.h"

/**
 * \class SMTpass
 * \brief SMT-formula creation pass
 *
 * Uses SMTpass::man as an abstraction layer to access the SMT solver.
 * It is a singleton design : we use getInstance() to get an object of type
 * SMTpass *
 */
class SMTpass : private llvm::InstVisitor<SMTpass> {

	friend class llvm::InstVisitor<SMTpass>;

	private:

		SMTpass();
		~SMTpass();

		static int nundef;

		int stack_level;

		/**
		 * \brief stores the rho formula associated to each function
		 */
		std::map<llvm::Function*,SMT_expr> rho;

		/**
		 * \brief stores the already computed varnames, since the computation of
		 * VarNames seems costly
		 */
		static std::map<llvm::Value*,std::string> VarNames;

		/**
		 * \brief when constructing rho, we use this vector
		 */
		std::vector<SMT_expr> rho_components;

		/**
		 * \brief when constructing instruction-related SMT formula, we
		 * use this vector
		 */
		std::vector<SMT_expr> instructions;

		/**
		 * \{
		 * \name get*Name functions
		 * these following methods are used to create a variable name for
		 * edges, nodes, values, undeterministic choices, ...
		 */
		static const std::string getDisjunctiveIndexName(AbstractDisj * A, int index);
		static const std::string getUndeterministicChoiceName(llvm::Value * v);
		static const std::string getValueName(llvm::Value * v, bool primed);
		/**
		 * \}
		 */


		/**
		 * \brief get the expression associated to a value
		 */
		SMT_expr getValueExpr(llvm::Value * v, bool primed);

		/**
		 * \return the SMT type of the value
		 */
		SMT_type getValueType(llvm::Value * v);

		/**
		 * \brief function to use for getting a variable from a value
		 */
		SMT_var getVar(llvm::Value * v, bool primed);

		SMT_var getBoolVar(llvm::Value * v, bool primed);

		/**
		 * \brief take a string name as input, and find if it
		 * is the name of an edge, a node, or a the index of a disjunctive
		 * invariant
		 *
		 * if it is an edge :
		 *  - src becomes the source of the edge
		 *  - dest becomes the destination of the edge
		 * if it is a Node :
		 *  - src becomes the basicblock of this node
		 *  - start is true iff the block is a start Node
		 * if it is an index for a disjunctive invariant :
		 * - isIndex is true
		 * - index is the associated index
		 */
		static void getElementFromString(
			std::string name,
			bool &isEdge,
			bool &isIndex,
			bool &start,
			llvm::BasicBlock * &src,
			llvm::BasicBlock * &dest,
			int &index);

		/**
		 * \brief called by visitPHINode
		 */
		SMT_expr construct_phi_ite(llvm::PHINode &I, unsigned i, unsigned n);

		/**
		 * \brief say if the value needs to be primed in the basicblock
		 */
		bool is_primed(llvm::BasicBlock * b, llvm::Instruction &I);

		/**
		 * \brief recursive function called by computeRho
		 */
		void computeRhoRec(	llvm::Function &F,
							std::set<llvm::BasicBlock*> & visited,
							llvm::BasicBlock * dest);
		void computeRho(llvm::Function &F);
		void computePrSuccAndPred(llvm::Function &F);

	public:
		static char ID;

		/**
		 * \brief manager (Microsoft z3 or Yices)
		 */
		SMT_manager * man;

		static SMTpass * getInstance();
		static SMTpass * getInstanceForAbstract();
		static void releaseMemory();

		void reset_SMTcontext();

		/**
		 * \brief get the SMT formula Rho. Rho is computed only once
		 */
		SMT_expr getRho(llvm::Function &F);

		/**
		 * \brief returns a name for a string
		 * this name is unique for the Value *
		 */
		static const std::string getVarName(llvm::Value * v);
		static const std::string getEdgeName(llvm::BasicBlock* b1, llvm::BasicBlock* b2);

		/**
		 * \brief push the context of the SMT manager
		 */
		void push_context();

		/**
		 * \brief pop the context of the SMT manager
		 */
		void pop_context();

		/**
		 * \brief assert an expression in the solver
		 */
		void SMT_assert(SMT_expr expr);

		/**
		 * \brief compute and return the SMT formula associated to
		 * the BasicBlock source
		 *
		 * See description in the paper SAS12
		 * if we are in narrowing phase, use_X_d has to be true.
		 * We can cunjunct this formula with an SMT_expr formula given as
		 * parameter of the function: constraint
		 */
		SMT_expr createSMTformula(
			llvm::BasicBlock * source,
			bool use_X_d,
			params t,
			SMT_expr constraint);

		/**
		 * \brief solve the SMT expression expr
		 * \return return true iff expr is
		 * satisfiable. In this case, path containts the path extracted from
		 * the model
		 */
		int SMTsolve(
				SMT_expr expr,
				std::list<llvm::BasicBlock*> & path,
				llvm::Function * F,
				params passID);

		/**
		 * \brief solve an SMT formula and computes its model in case of a 'sat'
		 * formula.
		 *
		 * In the case of a pass using disjunctive invariants, index is set to
		 * the associated index of the disjunct to focus on.
		 */
		int SMTsolve(
				SMT_expr expr,
				std::list<llvm::BasicBlock*> & path,
				int &index,
				llvm::Function * F,
				params passID);

		/**
		 * \brief solve the SMT expression
		 * \return 1 if satisfiable, 0 if not, -1 if unknown
		 */
		int SMTsolve_simple(SMT_expr expr);

		/**
		 * \brief gets the name of the node associated to a specific basicblock
		 */
		static const std::string getNodeName(llvm::BasicBlock* b, bool src);

		static const std::string getNodeSubName(llvm::BasicBlock* b);
		static llvm::BasicBlock * getNodeBasicBlock(std::string name);

		/**
		 * \{
		 * \name XToSmt - transform an apron object of type X into an SMT expression
		 */
		SMT_expr texpr1ToSmt(ap_texpr1_t texpr);
		SMT_expr linexpr1ToSmt(llvm::BasicBlock * b, ap_linexpr1_t linexpr, bool &integer, bool &skip);
		SMT_expr scalarToSmt(ap_scalar_t * scalar, bool integer, double &value, bool &infinity);
		SMT_expr tcons1ToSmt(ap_tcons1_t tcons);
		SMT_expr lincons1ToSmt(llvm::BasicBlock * b, ap_lincons1_t lincons,bool &skip);
		SMT_expr AbstractToSmt(llvm::BasicBlock * b, Abstract * A);
		/**
		 * \}
		 */

		/**
		 * \brief Creates an SMT formula associated to a disjunctive invariant.
		 *
		 * If insert_booleans is true, each disjunct is cunjunct with a boolean
		 * predicate, as detailed in the paper, so that we can deduce which
		 * disjunct to choose.
		 */
		SMT_expr AbstractDisjToSmt(llvm::BasicBlock * b, AbstractDisj * A, bool insert_booleans);

		/**
		 * \{
		 * \name Visit methods
		 */
	private:
		void visitReturnInst (llvm::ReturnInst &I);
		void visitBranchInst (llvm::BranchInst &I);
		void visitSwitchInst (llvm::SwitchInst &I);
		void visitIndirectBrInst (llvm::IndirectBrInst &I);
		void visitInvokeInst (llvm::InvokeInst &I);
#if LLVM_VERSION_MAJOR < 3 || LLVM_VERSION_MINOR == 0
		void visitUnwindInst (llvm::UnwindInst &I);
#endif
		void visitUnreachableInst (llvm::UnreachableInst &I);
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
			(void) I;
			//ferrs() << I.getOpcodeName();
			//assert(0 && ": Instruction not interpretable yet!");
		}
		/**
		 * \}
		 */
};
#endif
