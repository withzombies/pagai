/**
 * \file Pr.cc
 * \brief Implementation of the Pr class
 * \author Julien Henry
 */
#include "begin_3rdparty.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Analysis/CFG.h"
#include "end_3rdparty.h"

#include "Pr.h"
#include "Analyzer.h"
#include "Debug.h"
#include "IdentifyLoops.h"

using namespace llvm;

std::map<Function *, Pr *> PR_instances;

Pr::Pr(Function * _F) : F(_F) {
	if (!F->isDeclaration())
		computePr();
}

Pr::~Pr() {
}

Pr * Pr::getInstance(Function * F) {
	if (PR_instances.count(F) == 0) {
		PR_instances[F] = new Pr(F);
	}
	return PR_instances[F];
}

void Pr::releaseMemory() {
	for (auto entry : PR_instances) {
		delete entry.second;
	}
	PR_instances.clear();
}

std::set<BasicBlock*> & Pr::getPr() {
	return Pr_set;
}

std::set<BasicBlock*> & Pr::getPw() {
	return Pw_set;
}

std::set<BasicBlock*> & Pr::getAssert() {
	return Assert_set;
}

std::set<BasicBlock*> & Pr::getUndefinedBehaviour() {
	return UndefBehaviour_set;
}

bool Pr::inPr(BasicBlock * b) {
	return Pr_set.count(b);
}

bool Pr::inPw(BasicBlock * b) {
	return Pw_set.count(b);
}

bool Pr::inAssert(BasicBlock * b) {
	return Assert_set.count(b);
}
bool Pr::inUndefBehaviour(BasicBlock * b) {
	return UndefBehaviour_set.count(b);
}

bool Pr::check_acyclic(const std::set<BasicBlock*> & FPr) {
	std::set<BasicBlock*> start;
	start.insert(FPr.begin(), FPr.end());
	start.insert(&F->front());

	for (BasicBlock * bb : start) {
		Node * n = Nodes[bb];
		for (Function::iterator i = F->begin(); i != F->end(); ++i) {
			index[Nodes[i]] = 0;
			isInStack[Nodes[i]] = false;
		}
		std::stack<Node*> S;
		int N = 1;
		if (!check_acyclic_rec(n, N, S, FPr)) return false;
	}
	return true;
}

bool Pr::check_acyclic_rec(Node * n, int & N, std::stack<Node*> & S, const std::set<BasicBlock*> & FPr) {
	Node * nsucc;
	index[n] = N;
	lowlink[n] = N;
	N++;
	S.push(n);
	isInStack[n] = true;
	for (succ_iterator s = succ_begin(n->bb); s != succ_end(n->bb); ++s) {
		BasicBlock * succ = *s;
		nsucc = Nodes[succ];
		if (FPr.count(nsucc->bb)) {
			continue;
		}
		switch (index[nsucc]) {
			case 0:
				if (!check_acyclic_rec(nsucc, N, S, FPr)) return false;
				lowlink[n] = std::min(lowlink[n], lowlink[nsucc]);
				break;
			default:
				if (isInStack[nsucc]) {
					lowlink[n] = std::min(lowlink[n], index[nsucc]);
					return false;
				}
		}
	}
	if (lowlink == index) {
		do {
			nsucc = S.top();
			S.pop();
			isInStack[nsucc]=false;
		} while (nsucc != n);
	}
	return true;
}

bool Pr::computeLoopHeaders(std::set<BasicBlock*> & FPr) {
	for (Function::iterator i = F->begin(); i != F->end(); ++i) {
		index[Nodes[i]] = 0;
	}
#if 1
	for (BasicBlock * bb : Loop_headers) {
		if (bb->getParent() == F) {
			FPr.insert(bb);
		}
	}
#else
	std::set<Node*> S;
	std::set<Node*> Seen;
	Node * n = Nodes[&F->front()];
	computeLoopHeaders_rec(n, Seen, S, FPr);
#endif
	return true;
}

bool Pr::computeLoopHeaders_rec(Node * n, std::set<Node*> & Seen, std::set<Node*> & S, std::set<BasicBlock*> & FPr) {
	Node * nsucc;
	Seen.insert(n);

	if (S.count(n)) {
		FPr.insert(n->bb);
		return true;
	}

	std::set<Node*> Set;
	Set.insert(S.begin(), S.end());
	Set.insert(n);

	for (succ_iterator s = succ_begin(n->bb); s != succ_end(n->bb); ++s) {
		BasicBlock * succ = *s;
		nsucc = Nodes[succ];
		if (FPr.count(nsucc->bb)) {
			continue;
		}
		if (Seen.count(nsucc)) {
			if (Set.count(nsucc)) {
				FPr.insert(nsucc->bb);
			}
			continue;
		}
		computeLoopHeaders_rec(nsucc, Seen, Set, FPr);
	}
	return true;
}

// computePr - computes the set Pr of BasicBlocks
// for the moment - Pr = Pw + blocks with a ret inst
void Pr::computePr() {
	Node * n;
	BasicBlock * b;

	for (Function::iterator i = F->begin(); i != F->end(); ++i) {
		if (Nodes.count(i) == 0) {
			n = new Node(i);
			Nodes[i] = n;
		}
	}

	if (F->size() > 0) {
		// we find the Strongly Connected Components
		Node * front = Nodes[&(F->front())];
		front->computeSCC();
	}

	computeLoopHeaders(Pr_set);

	//minimize_Pr(F);

	//if (!check_acyclic(&Pr_set)) {
	//	*Out << "ERROR : GRAPH IS NOT ACYCLIC !\n";
	//	assert(false);
	//}

	Pw_set.insert(Pr_set.begin(),Pr_set.end());
	Pr_set.insert(F->begin());

	const std::string assert_fail ("__assert_fail");
	const std::string SVcomp_error ("__VERIFIER_error");
	const std::string llvm_trap ("llvm.trap");
	const std::string assert_fail_overflow ("__assert_fail_overflow");
	const std::string gnat_rcheck ("__gnat_rcheck_");

	for (Function::iterator i = F->begin(); i != F->end(); ++i) {
		b = i;
		for (BasicBlock::iterator it = b->begin(); it != b->end(); ++it) {
			if (isa<ReturnInst>(*it) || isa<UnreachableInst>(*it)) {
				Pr_set.insert(b);
			} else if (CallInst * c = dyn_cast<CallInst>((Instruction*)it)) {
				Function * cF = c->getCalledFunction();
				std::string fname;
				if (cF == NULL) {
					Value * calledvalue = c->getCalledValue();
					ConstantExpr * bc;
					if (calledvalue != NULL && (bc = dyn_cast<ConstantExpr>(calledvalue))) {
						Instruction * inst = bc->getAsInstruction();
						if (BitCastInst * bitcast = dyn_cast<BitCastInst>(inst)) {
							fname = bitcast->getOperand(0)->getName();
						}
						delete inst;
					}
				} else {
					fname = cF->getName();
				}
				if (fname.compare(assert_fail) == 0
						|| fname.compare(SVcomp_error) == 0) {
					Pr_set.insert(b);
					Assert_set.insert(b);
				}
				if (fname.compare(llvm_trap) == 0
						|| fname.compare(assert_fail_overflow) == 0
						|| fname.substr(0, gnat_rcheck.length()).compare(gnat_rcheck) == 0) {
					Pr_set.insert(b);
					UndefBehaviour_set.insert(b);
				}
			}
		}

	}

	DEBUG(
		std::set<BasicBlock*> & Pr = getPr();
		*Out << "FINAL Pr SET:\n";
		for (BasicBlock * bb : getPr()) {
			*Out << *bb << "\n";
		}
	);
}

void Pr::minimize_Pr() {
	std::set<BasicBlock*> & FPr = getPr();

	for (BasicBlock * bb : FPr) {
		std::set<BasicBlock*> Pr;
		Pr.insert(FPr.begin(), FPr.end());
		Pr.erase(bb);
		if (check_acyclic(Pr)) {
			Pr_set.erase(bb);
			minimize_Pr();
			return;
		} else {
			FPr.insert(bb);
		}
	}
}

std::set<BasicBlock*> Pr::getPrPredecessors(BasicBlock * b) {
	return Pr_pred[b];
}

std::set<BasicBlock*> Pr::getPrSuccessors(BasicBlock * b) {
	return Pr_succ[b];
}

