/**
 * \file AIGuided.cc
 * \brief Implementation of the AIGuided pass (Guided Static Analysis)
 * \author Julien Henry
 */
#include <vector>
#include <sstream>
#include <list>

#include "AIGuided.h"
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

static RegisterPass<AIGuided> X("AIGuidedPass", "Abstract Interpretation Pass", false, true);
static RegisterPass<ModulePassWrapper<AIGuided, 0> > Y0("AIGuidedPass_wrapped0", "Abstract Interpretation Pass", false, true);
static RegisterPass<ModulePassWrapper<AIGuided, 1> > Y1("AIGuidedtPass_wrapped1", "Abstract Interpretation Pass", false, true);

char AIGuided::ID = 0;

const char * AIGuided::getPassName() const {
	return "AIGuided";
}

void AIGuided::getAnalysisUsage(AnalysisUsage &AU) const {
	AU.setPreservesAll();
	AU.addRequired<Live>();
}

bool AIGuided::runOnModule(Module &M) {
	Function * F = nullptr;
	LSMT = SMTpass::getInstance();

	*Dbg << "// analysis: G\n";

	for (Module::iterator mIt = M.begin(); mIt != M.end(); ++mIt) {
		F = mIt;

		// if the function is only a declaration, do nothing
		if (F->begin() == F->end()) continue;
		if (definedMain() && !isMain(F)) continue;

		sys::TimeValue * time = new sys::TimeValue(0,0);
		*time = sys::TimeValue::now();
		Total_time[passID][F] = time;

		initFunction(F);


		// we create the new pathtree
		for (Function::iterator it = F->begin(); it != F->end(); ++it) {
			BasicBlock * b = it;
			pathtree[b] = new std::set<BasicBlock*>();
		}

		computeFunction(F);
		*Total_time[passID][F] = sys::TimeValue::now()-*Total_time[passID][F];

		TerminateFunction(F);
		printResult(F);

		// we delete the pathtree
		for (auto & entry : pathtree) {
			delete entry.second;
		}
		pathtree.clear();
	}
	assert(F != nullptr);
	generateAnnotatedFiles(F->getParent(),OutputAnnotatedFile());
	return 0;
}



void AIGuided::computeFunction(Function * F) {
	BasicBlock * b;
	Node * const n = Nodes[F->begin()];
	unknown = false;

	// A = {first basicblock}
	b = F->begin();
	if (b == F->end()) return;


	// get the information about live variables from the LiveValues pass
	LV = &(getAnalysis<Live>(*F));

	addFunctionArgumentsTo(n, F);

	// first abstract value is top
	computeEnv(n);
	Environment env(n,LV);
	n->X_s[passID]->set_top(&env);
	n->X_d[passID]->set_top(&env);
	while (!A_prime.empty()) {
		A_prime.pop();
	}
	while (!A.empty()) {
		A.pop();
	}

	//A' <- initial state
	A_prime.push(n);

	// Abstract Interpretation algorithm
	START();
	while (!A_prime.empty()) {

		// compute the new paths starting in a point in A'
		is_computed.clear();
		while (!A_prime.empty()) {
			Node * current = A_prime.top();
			A_prime.pop();
			computeNewPaths(current); // this method adds elements in A and A'
		}

		W = new PathTree_br(n->bb);
		is_computed.clear();
		ascendingIter(n, true);

		// we set X_d abstract values to bottom for narrowing
		Pr * FPr = Pr::getInstance(F);
		for (Function::iterator it = F->begin(); it != F->end(); ++it) {
			b = it;
			if (FPr->getPr().count(it) && Nodes[b] != n) {
				Nodes[b]->X_d[passID]->set_bottom(&env);
			}
		}

		narrowingIter(n);

		// then we move X_d abstract values to X_s abstract values
		int step = 0;
		while (copy_Xd_to_Xs(F) && step <= 5 && !unknown) {
			narrowingIter(n);
			TIMEOUT(unknown = true;);
			step++;
		}
		delete W;
	}
}

std::set<BasicBlock*> AIGuided::getPredecessors(BasicBlock * b) const {
	std::set<BasicBlock*> preds;
	pred_iterator p = pred_begin(b), E = pred_end(b);
	while (p != E) {
		preds.insert(*p);
		p++;
	}
	return preds;
}

std::set<BasicBlock*> AIGuided::getSuccessors(BasicBlock * b) const {
	std::set<BasicBlock*> succs;
	succ_iterator p = succ_begin(b), E = succ_end(b);
	while (p != E) {
		succs.insert(*p);
		p++;
	}
	return succs;
}

void AIGuided::computeNewPaths(Node * n) {
	Node * Succ;
	BasicBlock * b = n->bb;
	Abstract * Xtemp = NULL;
	std::vector<Abstract*> Join;
	std::list<BasicBlock*> path;

	if (is_computed.count(n) && is_computed[n]) {
		return;
	}

	DEBUG (
		changeColor(raw_ostream::GREEN);
		*Dbg << "#######################################################\n";
		*Dbg << "Computing new paths: " << b << "\n";
		resetColor();
		*Dbg << *b << "\n";
	);

	is_computed[n] = true;
	if (n->X_s[passID]->is_bottom()) {
		return;
	}

	for (succ_iterator s = succ_begin(b); s != succ_end(b); ++s) {
		path.clear();
		path.push_back(b);
		path.push_back(*s);

		if (pathtree[b]->count(*s)) continue;

		Succ = Nodes[*s];

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

		Xtemp->join_array_dpUcm(&Xtemp_env,aman->NewAbstract(Succ->X_s[passID]));

		if ( !Xtemp->is_leq(Succ->X_s[passID])) {
			delete Succ->X_s[passID];
			if (succ_bottom) {
				delete Succ->X_i[passID];
				Succ->X_i[passID] = aman->NewAbstract(Xtemp);
			}
			Succ->X_s[passID] = Xtemp;
			Xtemp = NULL;
			pathtree[n->bb]->insert(*s);
			A_prime.push(Succ);
			A.push(Succ);
			A.push(n);
			is_computed[Succ] = false;
		} else {
			delete Xtemp;
		}
		DEBUG(
			*Dbg << "RESULT FOR BASICBLOCK " << Succ->bb << ":\n";
			Succ->X_s[passID]->print();
		);
	}
}

void AIGuided::computeNode(Node * n) {
	BasicBlock * const b = n->bb;
	Abstract * Xtemp = NULL;
	Node * Succ = NULL;
	std::vector<Abstract*> Join;
	std::list<BasicBlock*> path;

	if (is_computed.count(n) && is_computed[n]) {
		return;
	}

	is_computed[n] = true;
	if (n->X_s[passID]->is_bottom()) {
		return;
	}

	DEBUG (
		changeColor(raw_ostream::GREEN);
		*Dbg << "#######################################################\n";
		*Dbg << "Computing node: " << b << "\n";
		resetColor();
		*Dbg << *b << "\n";
	);

	for (succ_iterator s = succ_begin(b); s != succ_end(b); ++s) {
		path.clear();
		path.push_back(b);
		path.push_back(*s);

		if (!pathtree[b]->count(*s)) continue;

		Succ = Nodes[*s];

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

		Pr * FPr = Pr::getInstance(b->getParent());
		if (FPr->inPw(Succ->bb)) {
			DEBUG(
				*Dbg << "WIDENING\n";
			);
			if (use_threshold) {
				Xtemp->widening_threshold(Succ->X_s[passID],threshold);
			} else
				Xtemp->widening(Succ->X_s[passID]);
		} else {
			Xtemp->join_array_dpUcm(&Xtemp_env,aman->NewAbstract(Succ->X_s[passID]));
		}

		if (!Succ->X_f[passID]->is_bottom()) {
			Xtemp->meet(Succ->X_f[passID]);
		}

		if ( !Xtemp->is_leq(Succ->X_s[passID])) {
			delete Succ->X_s[passID];
			if (succ_bottom) {
				delete Succ->X_i[passID];
				Succ->X_i[passID] = aman->NewAbstract(Xtemp);
			}
			Succ->X_s[passID] = Xtemp;
			Xtemp = NULL;
			A.push(Succ);
			A_prime.push(Succ);
			is_computed[Succ] = false;
		} else {
			delete Xtemp;
		}
		DEBUG(
			*Dbg << "RESULT FOR BASICBLOCK " << Succ->bb << ":\n";
			Succ->X_s[passID]->print();
		);
	}
}

void AIGuided::narrowNode(Node * n) {
	Abstract * Xtemp = NULL;
	Node * Succ;
	std::list<BasicBlock*> path;

	if (is_computed.count(n) && is_computed[n]) {
		return;
	}

	DEBUG (
		changeColor(raw_ostream::GREEN);
		*Dbg << "#######################################################\n";
		*Dbg << "narrowing node: " << n->bb << "\n";
		resetColor();
		*Dbg << *(n->bb) << "\n";
	);

	is_computed[n] = true;

	DEBUG(
		*Dbg << "STARTING POLYHEDRON\n";
		n->X_s[passID]->print();
	);

	for (succ_iterator s = succ_begin(n->bb); s != succ_end(n->bb); ++s) {
		path.clear();
		path.push_back(n->bb);
		path.push_back(*s);
		if (!pathtree[n->bb]->count(*s)) continue;
		Succ = Nodes[*s];

		// computing the image of the abstract value by the path's tranformation
		Xtemp = aman->NewAbstract(n->X_s[passID]);
		computeTransform(aman,path,Xtemp);

		desc_iterations[passID][n->bb->getParent()]++;

		DEBUG(
			*Dbg << "POLYHEDRON TO JOIN\n";
			Xtemp->print();
		);


		if (Succ->X_d[passID]->is_bottom()) {
			delete Succ->X_d[passID];
			Succ->X_d[passID] = Xtemp;
		} else {
			std::vector<Abstract*> Join;
			Join.clear();
			Join.push_back(aman->NewAbstract(Succ->X_d[passID]));
			Join.push_back(Xtemp);
			Environment Xtemp_env(Xtemp);
			Succ->X_d[passID]->join_array(&Xtemp_env,Join);
		}
		Xtemp = NULL;
		A.push(Succ);
	}
}
