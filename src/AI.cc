#include "llvm/Module.h"
#include "llvm/Instructions.h"
#include "llvm/BasicBlock.h"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Analysis/LiveValues.h"

#include "llvm/Analysis/Passes.h"

#include "ap_global1.h"
#include "pk.h"

#include "AI.h"
#include "Expr.h"
#include "Node.h"
#include "apron.h"
#include "InitVerif.h"

using namespace llvm;

char AI::ID = 0;

const char * AI::getPassName() const {
	return "AI";
}

void AI::getAnalysisUsage(AnalysisUsage &AU) const {
	AU.setPreservesAll();
	AU.addRequired<LiveValues>();
	AU.addRequired<initVerif>();
}

bool AI::runOnModule(Module &M) {
	ap_manager_t* man;
	Function * F = M.getFunction("main");

	man = pk_manager_alloc(true);
	if (F == NULL) {
		fouts() << "main function not found\n";
	} else {
		fouts() << "main function found\n";
		computeFunction(F);
	}
	ap_manager_free(man);
	return 0;
}

void AI::computeFunction(Function * F) {

	LV = &(getAnalysis<LiveValues>(*F));
	
	for (Function::arg_iterator a = F->arg_begin(), e = F->arg_end(); a != e; ++a) {
		if (!(a->use_empty()))
			fouts() << "non empty ! : NOT IMPLEMENTED\n";
		else
			fouts() << "argument " << *a << " never used !\n";
	}

	for (Function::iterator i = F->begin(), e = F->end(); i != e; ++i) {
		computeBasicBlock(i);
	}
}

void AI::computeBasicBlock(BasicBlock* b) {
	Node * pred;

	/* creation of the polyhedra at the beginning of the basicblock */
	for (pred_iterator p = pred_begin(b), E = pred_end(b); p != E; ++p) {
		BasicBlock *pb = *p;
		pred = Nodes[pb];
	}

	/* */
	for (BasicBlock::iterator i = b->begin(), e = b->end();
			i != e; ++i) {
		visit(*i);
	}
}

void AI::visitReturnInst (ReturnInst &I){
	fouts() << "returnInst\n" << I << "\n";
}

void AI::visitBranchInst (BranchInst &I){
	fouts() << "BranchInst\n" << I << "\n";	
}

void AI::visitSwitchInst (SwitchInst &I){
	fouts() << "SwitchInst\n" << I << "\n";	
}

void AI::visitIndirectBrInst (IndirectBrInst &I){
	fouts() << "IndirectBrInst\n" << I << "\n";	
}

void AI::visitInvokeInst (InvokeInst &I){
	fouts() << "InvokeInst\n" << I << "\n";	
}

void AI::visitUnwindInst (UnwindInst &I){
	fouts() << "UnwindInst\n" << I << "\n";	
}

void AI::visitUnreachableInst (UnreachableInst &I){
	fouts() << "UnreachableInst\n" << I << "\n";	
}

void AI::visitICmpInst (ICmpInst &I){
	fouts() << "ICmpInst\n" << I << "\n";	
}

void AI::visitFCmpInst (FCmpInst &I){
	fouts() << "FCmpInst\n" << I << "\n";	
}

void AI::visitAllocaInst (AllocaInst &I){
	fouts() << "AllocaInst\n" << I << "\n";	
}

void AI::visitLoadInst (LoadInst &I){
	fouts() << "LoadInst\n" << I << "\n";	
}

void AI::visitStoreInst (StoreInst &I){
	fouts() << "StoreInst\n" << I << "\n";	
}

void AI::visitGetElementPtrInst (GetElementPtrInst &I){
	fouts() << "GetElementPtrInst\n" << I << "\n";	
}

void AI::visitPHINode (PHINode &I){
	fouts() << "PHINode\n" << I << "\n";	
	ap_var_t var = (Value *) &I; 
	ap_environment_t* env = ap_environment_alloc(&var,1,NULL,0);
	ap_texpr1_t * exp = ap_texpr1_var(env,var);
	Expr::set_ap_expr(&I,exp);
	print_texpr(Expr::get_ap_expr(&I));

}

void AI::visitTruncInst (TruncInst &I){
	fouts() << "TruncInst\n" << I << "\n";	
}

void AI::visitZExtInst (ZExtInst &I){
	fouts() << "ZExtInst\n" << I << "\n";	
}

void AI::visitSExtInst (SExtInst &I){
	fouts() << "SExtInst\n" << I << "\n";	
}

void AI::visitFPTruncInst (FPTruncInst &I){
	fouts() << "FPTruncInst\n" << I << "\n";	
}

void AI::visitFPExtInst (FPExtInst &I){
	fouts() << "FPExtInst\n" << I << "\n";	
}

void AI::visitFPToUIInst (FPToUIInst &I){
	fouts() << "FPToUIInst\n" << I << "\n";	
}

void AI::visitFPToSIInst (FPToSIInst &I){
	fouts() << "FPToSIInst\n" << I << "\n";	
}

void AI::visitUIToFPInst (UIToFPInst &I){
	fouts() << "UIToFPInst\n" << I << "\n";	
}

void AI::visitSIToFPInst (SIToFPInst &I){
	fouts() << "SIToFPInst\n" << I << "\n";	
}

void AI::visitPtrToIntInst (PtrToIntInst &I){
	fouts() << "PtrToIntInst\n" << I << "\n";	
}

void AI::visitIntToPtrInst (IntToPtrInst &I){
	fouts() << "IntToPtrInst\n" << I << "\n";	
}

void AI::visitBitCastInst (BitCastInst &I){
	fouts() << "BitCastInst\n" << I << "\n";	
}

void AI::visitSelectInst (SelectInst &I){
	fouts() << "SelectInst\n" << I << "\n";	
}

void AI::visitCallInst(CallInst &I){
	fouts() << "CallInst\n" << I << "\n";	
	ap_var_t var = (Value *) &I; 
	ap_environment_t* env = ap_environment_alloc(&var,1,NULL,0);
	ap_texpr1_t * exp = ap_texpr1_var(env,var);
	Expr::set_ap_expr(&I,exp);
	print_texpr(exp);
}

void AI::visitVAArgInst (VAArgInst &I){
	fouts() << "VAArgInst\n" << I << "\n";	
}

void AI::visitExtractElementInst (ExtractElementInst &I){
	fouts() << "ExtractElementInst\n" << I << "\n";	
}

void AI::visitInsertElementInst (InsertElementInst &I){
	fouts() << "InsertElementInst\n" << I << "\n";	
}

void AI::visitShuffleVectorInst (ShuffleVectorInst &I){
	fouts() << "ShuffleVectorInst\n" << I << "\n";	
}

void AI::visitExtractValueInst (ExtractValueInst &I){
	fouts() << "ExtractValueInst\n" << I << "\n";	
}

void AI::visitInsertValueInst (InsertValueInst &I){
	fouts() << "InsertValueInst\n" << I << "\n";	
}

void AI::visitTerminatorInst (TerminatorInst &I){
	fouts() << "TerminatorInst\n" << I << "\n";	
}

void AI::visitBinaryOperator (BinaryOperator &I){
	fouts() << "BinaryOperator\n" << I << "\n";	
	ap_texpr_op_t op;
	switch(I.getOpcode()) {
		// Standard binary operators...
		case Instruction::Add : 
		case Instruction::FAdd: 
			op = AP_TEXPR_ADD;
			break;
		case Instruction::Sub : 
		case Instruction::FSub: 
			op = AP_TEXPR_SUB;
			break;
		case Instruction::Mul : 
		case Instruction::FMul: 
			op = AP_TEXPR_MUL;
			break;
		case Instruction::UDiv: 
		case Instruction::SDiv: 
		case Instruction::FDiv: 
			op = AP_TEXPR_DIV;
			break;
		case Instruction::URem: 
		case Instruction::SRem: 
		case Instruction::FRem: 
			op = AP_TEXPR_MOD;
			break;
			// Logical operators
		case Instruction::Shl : // Shift left  (logical)
		case Instruction::LShr: // Shift right (logical)
		case Instruction::AShr: // Shift right (arithmetic)
		case Instruction::And :
		case Instruction::Or  :
		case Instruction::Xor :
		case Instruction::BinaryOpsEnd:
			// NOT IMPLEMENTED
			return;
	}
	ap_texpr_rtype_t type = Expr::get_ap_type(&I);
	ap_texpr_rdir_t dir = AP_RDIR_NEAREST;
	Value * op1 = I.getOperand(0);
	Value * op2 = I.getOperand(1);

	ap_texpr1_t * exp1 = Expr::get_ap_expr(op1);
	ap_texpr1_t * exp2 = Expr::get_ap_expr(op2);

	/* we compute the least common environment for the two expressions */
	ap_dimchange_t ** dimchange1;
	ap_dimchange_t ** dimchange2;
	ap_environment_t* lcenv = ap_environment_lce(	exp1->env,
			exp2->env,
			dimchange1,
			dimchange2);
	/* we extend the environments such that both expressions have the same one */
	exp1 = ap_texpr1_extend_environment(exp1,lcenv);
	exp2 = ap_texpr1_extend_environment(exp2,lcenv);
	/* we create the expression associated to the binary op */
	ap_texpr1_t * exp = ap_texpr1_binop(op,exp1, exp2, type, dir);
	Expr::set_ap_expr(&I,exp);

	print_texpr(exp);
}

void AI::visitCmpInst (CmpInst &I){
	fouts() << "CmpInst\n" << I << "\n";	
}

void AI::visitCastInst (CastInst &I){
	fouts() << "CastInst\n" << I << "\n";	
}


