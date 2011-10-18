#ifndef ANALYZER_H
#define ANALYZER_H

#include <set>
#include <map>

#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/CFG.h"

enum SMT_Solver {
	Z3_MANAGER,
	YICES_MANAGER
};

enum Apron_Manager_Type {
	BOX,
	OCT,
	PK,
	PKEQ,
	PPL_POLY,
	PPL_GRID,
	PKGRID
};

enum Techniques {
	LOOKAHEAD_WIDENING,
	PATH_FOCUSING,
	LW_WITH_PF,
	SIMPLE,
	LW_WITH_PF_DISJ
};

std::string TechniquesToString(Techniques t);

std::string ApronManagerToString(Apron_Manager_Type D);

SMT_Solver getSMTSolver();

Techniques getTechnique();

bool compareTechniques();

bool compareDomain();

bool onlyOutputsRho();

bool useBagnaraWidening();

char* getFilename();

Apron_Manager_Type getApronManager();
Apron_Manager_Type getApronManager(int i);

extern llvm::raw_ostream *Out;

/// Functions ignored by Compare pass (because the analysis failed for
/// one technique)
extern std::set<llvm::Function*> ignoreFunction;


//extern std::map<Techniques,int> Passes;

#endif
