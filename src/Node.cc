/**
 * \file Node.cc
 * \brief Implementation of the Node class
 * \author Julien Henry
 */
#include <stack>
#include <map>
#include <set>

#include "begin_3rdparty.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/IR/Constants.h"
#include "end_3rdparty.h"

#include "Node.h"
#include "Live.h"
#include "Expr.h"
#include "Debug.h"
#include "Abstract.h"
#include "Analyzer.h"

using namespace llvm;

std::map<BasicBlock *,Node *> Nodes;

int i = 0;

Node::Node(BasicBlock * _bb) {
	index = 0;
	lowlink = 0;
	isInStack = false;
	bb = _bb;
	id = i++;
	env = new Environment();
}

Node::~Node() {
	// deleting all the abstract values attached to this node
	for (auto & entry : X_s) {
		delete entry.second;
	}
	for (auto & entry : X_d) {
		delete entry.second;
	}
	for (auto & entry : X_i) {
		delete entry.second;
	}
	for (auto & entry : X_f) {
		delete entry.second;
	}
	delete env;
}

Environment * Node::getEnv() {
	return env;
}

void Node::setEnv(Environment * e) {
	delete env;
	env = new Environment(*e);
}

void Node::computeSCC() {
	std::stack<Node*> * S = new std::stack<Node*>();
	int n = 1;
	computeSCC_rec(n,S);
	delete S;
}

void Node::computeSCC_rec(int & n,std::stack<Node*> * S) {
	Node * nsucc;
	index=n;
	lowlink=n;
	n++;
	S->push(this);
	isInStack=true;
	for (succ_iterator s = succ_begin(bb); s != succ_end(bb); ++s) {
		BasicBlock * succ = *s;
		nsucc = Nodes[succ];
		switch (nsucc->index) {
			case 0:
				nsucc->computeSCC_rec(n,S);
				lowlink = std::min(lowlink,nsucc->lowlink);
				break;
			default:
				if (nsucc->isInStack) {
					lowlink = std::min(lowlink,nsucc->index);
				}
		}
	}
	if (lowlink == index) {
		do {
			nsucc = S->top();
			S->pop();
			nsucc->isInStack=false;
			nsucc->sccId = index;
		} while (nsucc != this);
	}
}

void Node::add_var(Value * val) {
	assert(val != NULL);
	ap_var_t var = val;
	ap_texpr_rtype_t type;

	if (Expr::get_ap_type(val,type)) {
		return;
	}

	switch (type) {
		case AP_RTYPE_INT:
			intVar[val].insert(var);
			break;
		default:
			realVar[val].insert(var);
			break;
	}
	Expr exp(var);
	Expr::set_expr(val,&exp);
}

bool NodeCompare::operator() (Node * n1, Node * n2) {
	if (n1->sccId < n2->sccId) return true;
	return (n1->id > n2->id);
}
