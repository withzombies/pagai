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

std::map<params, std::map<llvm::Function*, llvm::sys::TimeValue *> > Total_time;
std::map<params, std::map<llvm::Function*, llvm::sys::TimeValue *> > Total_time_SMT;

std::map<params, std::map<llvm::Function*, int> > asc_iterations;
std::map<params, std::map<llvm::Function*, int> > desc_iterations;

std::map<params, std::set<llvm::Function*> > ignoreFunction;
std::map<llvm::Function*, int> numNarrowingSeedsInFunction;

void ReleaseTimeArray(std::map<params, std::map<llvm::Function*, llvm::sys::TimeValue *> > & T) {
	for (auto & map_entry : T) {
		for (auto & entry : map_entry.second) {
			delete entry.second;
		}
	}
}

void ReleaseTimingData() {
	ReleaseTimeArray(Total_time);
	ReleaseTimeArray(Total_time_SMT);
}

bool ignored(llvm::Function * F) {
	for (auto & entry : ignoreFunction) {
		if (entry.second.count(F)) return true;
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

llvm::sys::TimeValue TIMEOUT_LIMIT(3.);
llvm::sys::TimeValue start_timing;

