/**
 * \file Debug.cc
 * \brief Implementation of some Debug utilities
 * \author Julien Henry
 */
#include <sys/time.h>

#include "Debug.h"
#include "Analyzer.h"

int n_paths;
int n_totalpaths;

std::map<params, std::map<llvm::Function*, Duration> > Total_time;
std::map<params, std::map<llvm::Function*, Duration> > Total_time_SMT;

std::map<params, std::map<llvm::Function*, int> > asc_iterations;
std::map<params, std::map<llvm::Function*, int> > desc_iterations;

std::map<params, std::set<llvm::Function*> > ignoreFunction;
std::map<llvm::Function*, int> numNarrowingSeedsInFunction;

bool ignored(llvm::Function * F) {
	for (auto & entry : ignoreFunction) {
		if (entry.second.count(F)) {
			return true;
		}
	}
	return false;
}

int nb_ignored() {
	std::set<llvm::Function*> ignored_funcs;
	for (auto & entry : ignoreFunction) {
		ignored_funcs.insert(entry.second.begin(), entry.second.end());
	}
	return ignored_funcs.size();
}

TimePoint time_now() {
	return Clock::now();
}

TimePoint start_timing;
Duration TIMEOUT_LIMIT_SEC(3.);
