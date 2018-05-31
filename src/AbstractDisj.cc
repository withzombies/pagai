/**
 * \file AbstractDisj.cc
 * \brief Implementation of the AbstractDisj class
 * \author Julien Henry
 */
#include <cstdio>

#include "begin_3rdparty.h"
#include "llvm/Support/FormattedStream.h"

#include "ap_global1.h"
#include "end_3rdparty.h"

#include "AbstractDisj.h"
#include "Node.h"
#include "Expr.h"
#include "Analyzer.h"

AbstractDisj::AbstractDisj(ap_manager_t* _man, Environment * env) {
	man_disj = new AbstractManClassic();
	disj.push_back(man_disj->NewAbstract(_man,env));
	main = disj[0]->main;
	pilot = NULL;
	man = _man;
}

AbstractDisj::AbstractDisj(ap_manager_t* _man, Environment * env, int max_index) {
	man_disj = new AbstractManClassic();
	for (int i = 0; i <= max_index; i++)
		disj.push_back(man_disj->NewAbstract(_man,env));
	main = disj[0]->main;
	pilot = NULL;
	man = _man;
}

int AbstractDisj::AddDisjunct(Environment * env) {
	disj.push_back(man_disj->NewAbstract(man,env));
	return disj.size()-1;
}

Abstract * AbstractDisj::getDisjunct(int index) {
	SetNDisjunct(index);
	return disj[index];
}

void AbstractDisj::setDisjunct(int index, Abstract * A) {
	SetNDisjunct(index);
	disj[index] = A;
	main = disj[0]->main;
}

int AbstractDisj::getMaxIndex() {
	return disj.size()-1;
}

void AbstractDisj::SetNDisjunct(size_t N) {
	if (N + 1 <= disj.size()) return;
	Environment env;
	while (N + 1 > disj.size()) {
		disj.push_back(man_disj->NewAbstract(man,&env));
	}
	main = disj[0]->main;
}

AbstractDisj::AbstractDisj(Abstract* A) {
	man_disj = new AbstractManClassic();
	man = A->man;
	disj.clear();
	if (AbstractDisj * A_dis = dynamic_cast<AbstractDisj*>(A)) {
		for (Abstract * d : A_dis->disj) {
			disj.push_back(man_disj->NewAbstract(d));
		}
		main = disj[0]->main;
	} else {
		*Out << "ERROR when trying to create a disjunctive invariant\n";
		// ERROR
		main = NULL;
	}
	pilot = NULL;
}

void AbstractDisj::clear_all() {
	for (Abstract * d : disj) {
		delete d;
	}
	disj.clear();
	main = NULL;
}

AbstractDisj::~AbstractDisj() {
	delete man_disj;
	clear_all();
}

void AbstractDisj::set_top(Environment * env) {
	set_top(env,0);
}

void AbstractDisj::set_top(Environment * env, int index) {
	SetNDisjunct(index);
	int i = 0;
	// every disjunct is at bottom except the one of index 'index'
	for (Abstract * d : disj) {
		if (i == index) {
			d->set_top(env);
		} else {
			d->set_bottom(env);
		}
	}
	main = disj[0]->main;
}

void AbstractDisj::set_bottom(Environment * env) {
	for (Abstract * d : disj) {
		d->set_bottom(env);
	}
	main = disj[0]->main;
}

void AbstractDisj::set_bottom(Environment * env, int index) {
	SetNDisjunct(index);
	disj[index]->set_bottom(env);
	main = disj[0]->main;
}


void AbstractDisj::change_environment(Environment * env) {
	for (Abstract * d : disj) {
		d->change_environment(env);
	}
	main = disj[0]->main;
}

void AbstractDisj::change_environment(Environment * env, int index) {
	SetNDisjunct(index);
	disj[index]->change_environment(env);
	if (index == 0)
		main = disj[0]->main;
}

bool AbstractDisj::is_leq_index (Abstract *d, int index) {
	SetNDisjunct(index);
	return disj[index]->is_leq(d);
}

bool AbstractDisj::is_eq_index (Abstract *d, int index) {
	SetNDisjunct(index);
	return disj[index]->is_eq(d);
}

bool AbstractDisj::is_bottom() {
	for (Abstract * d : disj) {
		if (! d->is_bottom()) return false;
	}
	return true;
}

bool AbstractDisj::is_top() {
	for (Abstract * d : disj) {
		if (d->is_top()) return true;
	}
	return false;
}

bool AbstractDisj::is_bottom(int index) {
	SetNDisjunct(index);
	return disj[index]->is_bottom();
}

//NOT IMPLEMENTED
void AbstractDisj::widening(Abstract * X) {
	(void) X;
}

void AbstractDisj::widening(Abstract * X, int index) {
	SetNDisjunct(index);
	disj[index]->widening(X);
	main = disj[0]->main;
}

//NOT IMPLEMENTED
void AbstractDisj::widening_threshold(Abstract * X, Constraint_array* cons) {
	(void) X;
	(void) cons;
}

void AbstractDisj::widening_threshold(Abstract * X, Constraint_array* cons, int index) {
	SetNDisjunct(index);
	disj[index]->widening_threshold(X,cons);
	main = disj[0]->main;
}

void AbstractDisj::meet_tcons_array(Constraint_array* tcons) {
	for (Abstract * d : disj) {
		d->meet_tcons_array(tcons);
	}
	main = disj[0]->main;
}

void AbstractDisj::meet_tcons_array(Constraint_array* tcons, int index) {
	SetNDisjunct(index);
	disj[index]->meet_tcons_array(tcons);
	main = disj[0]->main;
}

void AbstractDisj::canonicalize() {
	for (Abstract * d : disj) {
		d->canonicalize();
	}
}

void AbstractDisj::assign_texpr_array(
		ap_var_t* tvar,
		ap_texpr1_t* texpr,
		size_t size,
		ap_abstract1_t* dest
		) {
	for (Abstract * d : disj) {
		d->assign_texpr_array(tvar, texpr, size, dest);
	}
	main = disj[0]->main;
}

void AbstractDisj::assign_texpr_array(
		ap_var_t* tvar,
		ap_texpr1_t* texpr,
		size_t size,
		ap_abstract1_t* dest,
		int index
		) {
	SetNDisjunct(index);
	disj[index]->assign_texpr_array(tvar,texpr,size,dest);
	main = disj[0]->main;
}

//NOT IMPLEMENTED
void AbstractDisj::join_array(Environment * env, const std::vector<Abstract*> & X_pred) {
	(void) env;
	(void) X_pred;
}

void AbstractDisj::join_array(Environment * env, const std::vector<Abstract*> & X_pred, int index) {
	SetNDisjunct(index);
	disj[index]->join_array(env, X_pred);
	main = disj[0]->main;
}

//NOT IMPLEMENTED
void AbstractDisj::join_array_dpUcm(Environment *env, Abstract* n) {
	(void) env;
	(void) n;
}

void AbstractDisj::join_array_dpUcm(Environment *env, Abstract* n, int index) {
	SetNDisjunct(index);
	disj[index]->join_array_dpUcm(env, n);
	main = disj[0]->main;
}

void AbstractDisj::meet(Abstract* A) {
	//TODO
	(void) A;
}

//NOT CORRECT
ap_tcons1_array_t AbstractDisj::to_tcons_array() {
	return ap_abstract1_to_tcons_array(man,main);
}

ap_tcons1_array_t AbstractDisj::to_tcons_array(int index) {
	SetNDisjunct(index);
	return disj[index]->to_tcons_array();
}

//NOT CORRECT
ap_lincons1_array_t AbstractDisj::to_lincons_array() {
	return ap_abstract1_to_lincons_array(man,main);
}

ap_lincons1_array_t AbstractDisj::to_lincons_array(int index) {
	SetNDisjunct(index);
	return disj[index]->to_lincons_array();
}

void AbstractDisj::print() {
	*Out << *this;
}

void AbstractDisj::display(llvm::raw_ostream &stream, std::string * left) const {
	if (disj.size() == 1) {
		disj[0]->display(stream, left);
	} else {
		int count = 0;
		for (const Abstract * d : disj) {
			if (left != NULL) stream << *left;
			stream << "Disjunct " << count << "\n";
			d->display(stream, left);
			++count;
		}
	}
}
