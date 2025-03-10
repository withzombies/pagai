/**
 * \file AIpf.cc
 * \brief Implementation of the AIpf pass (Path Focusing)
 * \author Julien Henry
 */
#include <vector>
#include <list>

#include "AIpf.h"
#include "Expr.h"
#include "Node.h"
#include "apron.h"
#include "Live.h"
#include "SMTpass.h"
#include "Pr.h"
#include "Debug.h"
#include "Analyzer.h"
#include "PathTree.h"
#include "PathTree_br.h"
#include "ModulePassWrapper.h"

using namespace llvm;

static RegisterPass<AIpf> X("AIPass", "Abstract Interpretation Pass", false, true);
static RegisterPass<ModulePassWrapper<AIpf, 0> > Y0("AIpfPass_wrapped0", "Abstract Interpretation Pass", false, true);
static RegisterPass<ModulePassWrapper<AIpf, 1> > Y1("AIpfPass_wrapped1", "Abstract Interpretation Pass", false, true);

char AIpf::ID = 0;

const char * AIpf::getPassName() const {
	return "AIpf";
}

void AIpf::getAnalysisUsage(AnalysisUsage &AU) const {
	AU.setPreservesAll();
	AU.addRequired<Live>();
}

bool AIpf::runOnModule(Module &M) {
	Function * F = nullptr;
	LSMT = SMTpass::getInstance();
	*Dbg << "// analysis: " << getPassName() << "\n";
	for (Module::iterator mIt = M.begin(); mIt != M.end(); ++mIt) {
		F = mIt;

		// if the function is only a declaration, do nothing
		if (F->begin() == F->end()) continue;
		if (definedMain() && !isMain(F)) continue;
		Pr * FPr = Pr::getInstance(F);
		if (SVComp() && FPr->getAssert().empty()) continue;

		TimePoint start_time = time_now();

		initFunction(F);

		// we create the new pathtree
		for (BasicBlock * bb : FPr->getPr()) {
			if (bb->getTerminator()->getNumSuccessors() > 0
					&& ! FPr->inUndefBehaviour(bb)
					&& ! FPr->inAssert(bb)
					) {
				U[bb] = new PathTree_br(bb);
				V[bb] = new PathTree_br(bb);
			}
		}

		computeFunction(F);
		Total_time[passID][F] = time_now() - start_time;

		TerminateFunction(F);
		printResult(F);

		// deleting the pathtrees
		ClearPathtreeMap(U);
		ClearPathtreeMap(V);

		LSMT->reset_SMTcontext();
	}
	assert(F != nullptr);
	generateAnnotatedFiles(F->getParent(),OutputAnnotatedFile());

	SMTpass::releaseMemory();
	return false;
}

void AIpf::computeFunction(Function * F) {
	BasicBlock * b;
	Node * const n = Nodes[F->begin()];
	unknown = false;

	// A = {first basicblock}
	b = F->begin();
	if (b == F->end()) return;


	// get the information about live variables from the LiveValues pass
	LV = &(getAnalysis<Live>(*F));

	LSMT->push_context();
	DEBUG(
	if (!quiet_mode())
		*Dbg << "Computing Rho...";
		);
	LSMT->SMT_assert(LSMT->getRho(*F));

	// we assert b_i => I_i for each block
	params P;
	P.T = SIMPLE;
	P.D = getApronManager();
	P.N = useNewNarrowing();
	P.TH = useThreshold();
	assert_properties(P,F);

	DEBUG(
	if (!quiet_mode())
		*Dbg << "OK\n";
		);

	addFunctionArgumentsTo(n, F);

	// first abstract value is top
	computeEnv(n);
	Environment env(n,LV);
	n->X_s[passID]->set_top(&env);
	n->X_d[passID]->set_top(&env);
	n->X_i[passID]->set_top(&env);
	A.push(n);

	START();
	ascendingIter(n);
	if (unknown) goto end;

	if (SVComp() && asserts_proved(F)) goto end;
	narrowingIter(n);
	if (unknown) goto end;
	// then we move X_d abstract values to X_s abstract values
	{
		int step = 0;
		while (copy_Xd_to_Xs(F) && step <= 1) {
			if (SVComp() && asserts_proved(F)) goto end;
			narrowingIter(n);
			if (unknown) goto end;
			step++;
		}
	}
end:
	LSMT->pop_context();
}

std::set<BasicBlock*> AIpf::getPredecessors(BasicBlock * b) const {
	Pr * FPr = Pr::getInstance(b->getParent());
	return FPr->getPrPredecessors(b);
}

std::set<BasicBlock*> AIpf::getSuccessors(BasicBlock * b) const {
	Pr * FPr = Pr::getInstance(b->getParent());
	return FPr->getPrSuccessors(b);
}

void AIpf::computeNode(Node * n) {
	BasicBlock * b = n->bb;
	Abstract * Xtemp;
	Node * Succ;
	std::vector<Abstract*> Join;
	bool only_join = false;

	if (is_computed.count(n) && is_computed[n]) {
		return;
	}
	DEBUG (
		changeColor(raw_ostream::GREEN);
		*Dbg << "#######################################################\n";
		*Dbg << "Computing node: " << b << "\n";
		resetColor();
		*Dbg << *b << "\n";
	);

	if (U.count(b)) {
		U[b]->clear();
		V[b]->clear();
	} else {
		// this is a block without any successors...
		is_computed[n] = true;
		return;
	}

	while (true) {
		is_computed[n] = true;
		DEBUG(
			changeColor(raw_ostream::RED);
			*Dbg << "--------------- NEW SMT SOLVE -------------------------\n";
			resetColor();
		);
		LSMT->push_context();
		// creating the SMTpass formula we want to check
		SMT_expr T = LSMT->man->SMT_mk_true();
		SMT_expr smtexpr = LSMT->createSMTformula(n->bb,false,passID,T);
		std::list<BasicBlock*> path;
		DEBUG_SMT(
			*Dbg
				<< "\n"
				<< "FORMULA"
				<< "(COMPUTENODE)"
				<< "\n\n";
			LSMT->man->SMT_print(smtexpr);
		);
		// if the result is unsat, then the computation of this node is finished
		int res = LSMT->SMTsolve(smtexpr, path, n->bb->getParent(), passID);

		LSMT->pop_context();
		if (res != 1 || path.size() == 1) {
			if (res == -1) {
				unknown = true;
			}
			return;
		}

		TIMEOUT(unknown = true; return;);

		DEBUG(
			printPath(path);
		);

		Succ = Nodes[path.back()];

		asc_iterations[passID][n->bb->getParent()]++;

		// computing the image of the abstract value by the path's tranformation
		Xtemp = aman->NewAbstract(n->X_s[passID]);
		computeTransform(aman,path,Xtemp);

		DEBUG(
			*Dbg << "POLYHEDRON AT THE STARTING NODE\n";
			n->X_s[passID]->print();
			*Dbg << "POLYHEDRON AFTER PATH TRANSFORMATION\n";
			Xtemp->print();
		);

		Environment Xtemp_env(Xtemp);
		Succ->X_s[passID]->change_environment(&Xtemp_env);

		bool succ_bottom = (Succ->X_s[passID]->is_bottom());

		// if we have a self loop, we apply loopiter
		if (Succ == n) {
			loopiter(n,Xtemp,&path,only_join,U[n->bb],V[n->bb]);
		}

		Join.clear();
		Join.push_back(aman->NewAbstract(Succ->X_s[passID]));
		Join.push_back(aman->NewAbstract(Xtemp));
		Xtemp->join_array(&Xtemp_env,Join);

		Pr * FPr = Pr::getInstance(b->getParent());
		if (FPr->inPw(Succ->bb) && ((Succ != n) || !only_join)) {
			if (use_threshold)
				Xtemp->widening_threshold(Succ->X_s[passID],threshold);
			else
				Xtemp->widening(Succ->X_s[passID]);
			DEBUG(
				*Dbg << "WIDENING! \n";
			);
		} else {
			DEBUG(
				*Dbg << "NO WIDENING\n";
			);
		}

		DEBUG(
			*Dbg << "BEFORE:\n";
			Succ->X_s[passID]->print();
		);
		delete Succ->X_s[passID];
		if (succ_bottom) {
			delete Succ->X_i[passID];
			Succ->X_i[passID] = aman->NewAbstract(Xtemp);
		}

		// intersection with the previous invariant
		params P;
		P.T = SIMPLE;
		P.D = getApronManager();
		P.N = useNewNarrowing();
		P.TH = useThreshold();
		intersect_with_known_properties(Xtemp,Succ,P);

		Succ->X_s[passID] = Xtemp;

		DEBUG(
			*Dbg << "RESULT:\n";
			Succ->X_s[passID]->print();
		);

		A.push(Succ);
		is_computed[Succ] = false;
	}
}

void AIpf::narrowNode(Node * n) {
	Abstract * Xtemp;
	Node * Succ;

	if (is_computed.count(n) && is_computed[n]) {
		return;
	}

	while (true) {
		is_computed[n] = true;

		DEBUG(
			changeColor(raw_ostream::RED);
			*Dbg << "NARROWING------ NEW SMT SOLVE -------------------------\n";
			resetColor();
		);
		LSMT->push_context();
		// creating the SMTpass formula we want to check
		SMT_expr T = LSMT->man->SMT_mk_true();
		SMT_expr smtexpr = LSMT->createSMTformula(n->bb,true,passID,T);
		std::list<BasicBlock*> path;
		DEBUG_SMT(
			*Dbg
				<< "\n"
				<< "FORMULA"
				<< "(NARROWNODE)"
				<< "\n\n";
			LSMT->man->SMT_print(smtexpr);
		);
		// if the result is unsat, then the computation of this node is finished
		int res = LSMT->SMTsolve(smtexpr, path, n->bb->getParent(), passID);

		LSMT->pop_context();
		if (res != 1 || path.size() == 1) {
			if (res == -1) {
				unknown = true;
			}
			return;
		}

		TIMEOUT(unknown = true; return;);

		DEBUG(
			printPath(path);
		);

		desc_iterations[passID][n->bb->getParent()]++;

		Succ = Nodes[path.back()];

		// computing the image of the abstract value by the path's tranformation
		Xtemp = aman->NewAbstract(n->X_s[passID]);
		DEBUG(
			*Dbg << "STARTING POLYHEDRON\n";
			Xtemp->print();
		);
		computeTransform(aman,path,Xtemp);

		DEBUG(
			*Dbg << "POLYHEDRON TO JOIN\n";
			Xtemp->print();
		);

		// intersection with the previous invariant
		params P;
		P.T = SIMPLE;
		P.D = getApronManager();
		P.N = useNewNarrowing();
		P.TH = useThreshold();
		intersect_with_known_properties(Xtemp,Succ,P);

		if (Succ->X_d[passID]->is_bottom()) {
			delete Succ->X_d[passID];
			Succ->X_d[passID] = Xtemp;
		} else {
			std::vector<Abstract*> Join;
			Join.push_back(aman->NewAbstract(Succ->X_d[passID]));
			Join.push_back(Xtemp);
			Environment Xtemp_env(Xtemp);
			Succ->X_d[passID]->join_array(&Xtemp_env,Join);
		}
		A.push(Succ);
		is_computed[Succ] = false;
	}
}
