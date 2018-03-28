/**
 * \file Expr.h
 * \brief Declaration of the Expr class
 * \author Julien Henry
 */
#ifndef _EXPR_H
#define _EXPR_H

#include <map>

#include "config.h"

#include "begin_3rdparty.h"
#include "ap_global1.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/IR/Constants.h"
#if LLVM_VERSION_ATLEAST(3, 5)
#   include "llvm/IR/InstVisitor.h"
#else
#   include "llvm/InstVisitor.h"
#endif
#include "end_3rdparty.h"

#include "Abstract.h"
#include "Environment.h"
#include "Constraint.h"

/**
 * \class Expr
 * \brief wrapper around apron expressions
 */
class Expr : public llvm::InstVisitor<Expr,ap_texpr1_t*> {

	friend class Abstract;

	private:
		ap_texpr1_t * ap_expr;

		ap_texpr1_t * create_expression(llvm::Value * val);
		ap_texpr1_t * create_ap_expr(llvm::Constant * val);
		ap_texpr1_t * create_ap_expr(ap_var_t var);
		ap_texpr1_t * create_ap_expr(llvm::UndefValue * undef);

		static void texpr1_print(ap_texpr1_t * expr);

		Expr();

	public:

		/**
		 * \brief compute the Apron type of the LLVM Value
		 * \return 0 iff the type is int or real, 1 in the other cases
		 */
		static int get_ap_type(llvm::Value * val,ap_texpr_rtype_t &ap_type);

		static void tcons1_array_print(ap_tcons1_array_t * cons);

		static bool is_undef_ap_var(ap_var_t var);
	public:

		~Expr();

		Expr(llvm::Value * val);
		Expr(ap_var_t var);
		Expr(const Expr &exp);
		Expr(Expr * exp);
		Expr(double d);

		Expr(ap_texpr_op_t op, Expr * exp1, Expr * exp2, ap_texpr_rtype_t type, ap_texpr_rdir_t round);

		/**
		 * \brief Overloaded copy assignment operator
		 */
		Expr & operator= (const Expr & exp);

		/**
		 * \brief clear all expressions stored in the internal map
		 */
		static void clear_exprs();

		/**
		 * \brief create a constraint and insert it into t_cons
		 */
		static void create_constraints (
			ap_constyp_t constyp,
			Expr * expr,
			Expr * nexpr,
			std::vector<Constraint*> * t_cons);

		/**
		 * \brief modifies the two expression by giving them the same
		 * least common environment.
		 */
		static void common_environment(Expr * exp1, Expr * exp2);

		Environment * getEnv();
		ap_texpr1_t * getExpr();

		static void set_expr(llvm::Value * val, Expr * exp);

		void print();

		/** 
		 * \{
		 * \name Visit methods
		 */
		ap_texpr1_t * visitReturnInst (llvm::ReturnInst &I);
		ap_texpr1_t * visitBranchInst (llvm::BranchInst &I);
		ap_texpr1_t * visitSwitchInst (llvm::SwitchInst &I);
		ap_texpr1_t * visitIndirectBrInst (llvm::IndirectBrInst &I);
		ap_texpr1_t * visitInvokeInst (llvm::InvokeInst &I);
#if LLVM_VERSION_MAJOR < 3 || LLVM_VERSION_MINOR == 0
		ap_texpr1_t * visitUnwindInst (llvm::UnwindInst &I);
#endif
		ap_texpr1_t * visitUnreachableInst (llvm::UnreachableInst &I);
		ap_texpr1_t * visitICmpInst (llvm::ICmpInst &I);
		ap_texpr1_t * visitFCmpInst (llvm::FCmpInst &I);
		ap_texpr1_t * visitAllocaInst (llvm::AllocaInst &I);
		ap_texpr1_t * visitLoadInst (llvm::LoadInst &I);
		ap_texpr1_t * visitStoreInst (llvm::StoreInst &I);
		ap_texpr1_t * visitGetElementPtrInst (llvm::GetElementPtrInst &I);
		ap_texpr1_t * visitPHINode (llvm::PHINode &I);
		ap_texpr1_t * visitTruncInst (llvm::TruncInst &I);
		ap_texpr1_t * visitZExtInst (llvm::ZExtInst &I);
		ap_texpr1_t * visitSExtInst (llvm::SExtInst &I);
		ap_texpr1_t * visitFPTruncInst (llvm::FPTruncInst &I);
		ap_texpr1_t * visitFPExtInst (llvm::FPExtInst &I);
		ap_texpr1_t * visitFPToUIInst (llvm::FPToUIInst &I);
		ap_texpr1_t * visitFPToSIInst (llvm::FPToSIInst &I);
		ap_texpr1_t * visitUIToFPInst (llvm::UIToFPInst &I);
		ap_texpr1_t * visitSIToFPInst (llvm::SIToFPInst &I);
		ap_texpr1_t * visitPtrToIntInst (llvm::PtrToIntInst &I);
		ap_texpr1_t * visitIntToPtrInst (llvm::IntToPtrInst &I);
		ap_texpr1_t * visitBitCastInst (llvm::BitCastInst &I);
		ap_texpr1_t * visitSelectInst (llvm::SelectInst &I);
		ap_texpr1_t * visitCallInst(llvm::CallInst &I);
		ap_texpr1_t * visitVAArgInst (llvm::VAArgInst &I);
		ap_texpr1_t * visitExtractElementInst (llvm::ExtractElementInst &I);
		ap_texpr1_t * visitInsertElementInst (llvm::InsertElementInst &I);
		ap_texpr1_t * visitShuffleVectorInst (llvm::ShuffleVectorInst &I);
		ap_texpr1_t * visitExtractValueInst (llvm::ExtractValueInst &I);
		ap_texpr1_t * visitInsertValueInst (llvm::InsertValueInst &I);
		ap_texpr1_t * visitTerminatorInst (llvm::TerminatorInst &I);
		ap_texpr1_t * visitBinaryOperator (llvm::BinaryOperator &I);
		ap_texpr1_t * visitCmpInst (llvm::CmpInst &I);
		ap_texpr1_t * visitCastInst (llvm::CastInst &I);

		ap_texpr1_t * visitInstruction(llvm::Instruction &I) {
			(void) I;
			return NULL;
		}
		/**
		 * \}
		 */
		ap_texpr1_t * visitInstAndAddVar(llvm::Instruction &I);
};

#endif
