#include <vector>
#include <list>

#include "llvm/Support/CFG.h"

#include "AISimple.h"
#include "Expr.h"
#include "Node.h"
#include "apron.h"
#include "Live.h"
#include "Pr.h"
#include "Debug.h"
#include "Analyzer.h"

using namespace llvm;

void AISimple::computeFunc(Function * F) {
	BasicBlock * b;
	Node * n;

	// A = {first basicblock}
	b = F->begin();
	if (b == F->end()) return;
	n = Nodes[b];

	// add all function's arguments into the environment of the first bb
	for (Function::arg_iterator a = F->arg_begin(), e = F->arg_end(); a != e; ++a) {
		Argument * arg = a;
		if (!(arg->use_empty()))
			n->add_var(arg);
		else 
			*Out << "argument " << *a << " never used !\n";
	}
	// first abstract value is top
	ap_environment_t * env = NULL;
	computeEnv(n);
	n->create_env(&env,LV);
	n->X_s[passID]->set_top(env);
	n->X_d[passID]->set_top(env);
	n->X_i[passID]->set_top(env);
	n->X_f[passID]->set_top(env);

	ascendingIter(n, F);

	narrowingIter(n);

	// then we move X_d abstract values to X_s abstract values
	int step = 0;
	while (copy_Xd_to_Xs(F) && step <= 5) {
		narrowingIter(n);
		step++;
	}


	if (NewNarrowing) {
		copy_Xs_to_Xf(F);

		if (!computeWideningSeed(F)) {
			DEBUG(
				*Out << "NO SEEDS\n";
			);
			copy_Xf_to_Xs(F);
			return;
		}
		
		copy_Xd_to_Xs(F);
	
		ascendingIter(n, F);
		narrowingIter(n);
		step = 0;
		while (copy_Xd_to_Xs(F) && step <= 5) {
			narrowingIter(n);
			step++;
		}
	}
}

std::set<BasicBlock*> AISimple::getPredecessors(BasicBlock * b) const {
	std::set<BasicBlock*> preds;
	pred_iterator p = pred_begin(b), E = pred_end(b);
	while (p != E) {
		preds.insert(*p);
		p++;
	}
	return preds;
}

std::set<BasicBlock*> AISimple::getSuccessors(BasicBlock * b) const {
	std::set<BasicBlock*> succs;
	succ_iterator p = succ_begin(b), E = succ_end(b);
	while (p != E) {
		succs.insert(*p);
		p++;
	}
	return succs;
}

void AISimple::getAnalysisUsage(AnalysisUsage &AU) const {
	AU.setPreservesAll();
	AU.addRequired<Live>();
	AU.addRequired<Pr>();
}

bool AISimple::runOnModule(Module &M) {
	Function * F;
	BasicBlock * b;
	Node * n;
	*Out << "Starting analysis: " << getPassName() << "\n";

	for (Module::iterator mIt = M.begin() ; mIt != M.end() ; ++mIt) {
		F = mIt;
		
		// if the function is only a declaration, do nothing
		if (F->empty()) continue;

		Out->changeColor(raw_ostream::BLUE,true);
		*Out << "\n\n\n"
				<< "------------------------------------------\n"
				<< "-         COMPUTING FUNCTION             -\n"
				<< "------------------------------------------\n";
		Out->resetColor();
		LSMT = SMTpass::getInstance();
		LSMT->reset_SMTcontext();

		sys::TimeValue * time = new sys::TimeValue(0,0);
		*time = sys::TimeValue::now();
		Total_time[passID][F] = time;

		initFunction(F);
		computeFunction(F);
		*Total_time[passID][F] = sys::TimeValue::now()-*Total_time[passID][F];
		printResult(F);
	}
	return 0;
}

void AISimple::computeNode(Node * n) {
	BasicBlock * b = n->bb;
	Abstract * Xtemp;
	Node * Succ;
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
		Out->changeColor(raw_ostream::GREEN,true);
		*Out << "#######################################################\n";
		*Out << "Computing node: " << b << "\n";
		Out->resetColor();
		*Out << *b << "\n";
	);


	for (succ_iterator s = succ_begin(b), E = succ_end(b); s != E; ++s) {
		path.clear();
		path.push_back(b);
		path.push_back(*s);
		Succ = Nodes[*s];
		
		asc_iterations[passID][n->bb->getParent()]++;

		// computing the image of the abstract value by the path's tranformation
		Xtemp = aman->NewAbstract(n->X_s[passID]);
		computeTransform(aman,n,path,*Xtemp);

		DEBUG(
			*Out << "POLYHEDRON AT THE STARTING NODE\n";
			n->X_s[passID]->print();
			*Out << "POLYHEDRON AFTER PATH TRANSFORMATION\n";
			Xtemp->print();
		);

		Succ->X_s[passID]->change_environment(Xtemp->main->env);

		bool succ_bottom = (Succ->X_s[passID]->is_bottom());

		if (Pr::inPw(Succ->bb)) {
			DEBUG(
				*Out << "WIDENING\n";
			);
			if (use_threshold) {
				Xtemp->widening_threshold(Succ->X_s[passID],&threshold);
			} else
				Xtemp->widening(Succ->X_s[passID]);
		} else {
			Xtemp->join_array_dpUcm(Xtemp->main->env,aman->NewAbstract(Succ->X_s[passID]));
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
			is_computed[Succ] = false;
		} else {
			delete Xtemp;
		}
		DEBUG(
			*Out << "RESULT FOR BASICBLOCK " << Succ->bb << ":\n";
			Succ->X_s[passID]->print();
		);
	}
}


void AISimple::narrowNode(Node * n) {
	Abstract * Xtemp;
	Node * Succ;
	std::list<BasicBlock*> path;

	if (is_computed.count(n) && is_computed[n]) {
		return;
	}

	DEBUG (
		Out->changeColor(raw_ostream::GREEN,true);
		*Out << "#######################################################\n";
		*Out << "narrowing node: " << n->bb << "\n";
		Out->resetColor();
		*Out << *(n->bb) << "\n";
	);

	is_computed[n] = true;

	DEBUG(
		*Out << "STARTING POLYHEDRON\n";
		n->X_s[passID]->print();
	);

	for (succ_iterator s = succ_begin(n->bb), E = succ_end(n->bb); s != E; ++s) {
		path.clear();
		path.push_back(n->bb);
		path.push_back(*s);
		Succ = Nodes[*s];

		// computing the image of the abstract value by the path's tranformation
		Xtemp = aman->NewAbstract(n->X_s[passID]);
		computeTransform(aman,n,path,*Xtemp);

		desc_iterations[passID][n->bb->getParent()]++;

		DEBUG(
			*Out << "POLYHEDRON TO JOIN\n";
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
			Succ->X_d[passID]->join_array(Xtemp->main->env,Join);
		}
		Xtemp = NULL;
		A.push(Succ);
	}
}

void AISimple::computeFunction(Function * F) {
	if (F->empty()) {
		return;
	}

	// get the information about live variables from the LiveValues pass
	LV = &(getAnalysis<Live>(*F));

	computeFunc(F);
}
