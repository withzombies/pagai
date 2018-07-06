/**
 * \file Debug.h
 * \brief Declares some Debug functions and variables
 * \author Julien Henry
 */
#ifndef DEBUG_H
#define DEBUG_H

// DM 2014-11-20 experimental
#define NAIVE_TRUNC

// Don't use LLVM's DEBUG statement
#undef DEBUG

// #define PRINT_DEBUG

#ifdef PRINT_DEBUG
#   define DEBUG(X) do {X} while (0)
#else
#   define DEBUG(X) do { } while (0)
#endif

#ifdef PRINT_DEBUG_SMT
#   define DEBUG_SMT(X) do {X} while (0)
#else
#   define DEBUG_SMT(X) do { } while (0)
#endif

#ifdef HAS_Z3
#   define Z3(X)  X
#else
#   define Z3(X)
#endif

#include <ctime>
#include <chrono>
#include <map>

#include "Node.h"

extern int n_paths;
extern int n_totalpaths;

typedef std::chrono::high_resolution_clock			Clock;
typedef std::chrono::duration<double>				Duration;
typedef std::chrono::time_point<Clock, Duration>	TimePoint;

extern std::map<params, std::map<llvm::Function*, Duration> > Total_time;
extern std::map<params, std::map<llvm::Function*, Duration> > Total_time_SMT;

/**
 * \brief count the number of ascending iterations
 */
extern std::map<params,std::map<llvm::Function*,int> > asc_iterations;

/**
 * \brief count the number of descending iterations
 */
extern std::map<params,std::map<llvm::Function*,int> > desc_iterations;

/**
 * \brief Functions ignored by Compare pass (because the analysis failed for
 * one technique)
 */
extern std::map<params,std::set<llvm::Function*> > ignoreFunction;
extern std::map<llvm::Function*,int> numNarrowingSeedsInFunction;

extern bool ignored(llvm::Function * F);
extern int nb_ignored();
extern TimePoint time_now();

extern TimePoint start_timing;
extern Duration TIMEOUT_LIMIT_SEC;

#define START() do { start_timing = time_now(); } while (0)
#define TIMEOUT_COND() (time_now() - start_timing > TIMEOUT_LIMIT_SEC)

#define TIMEOUT(X) do { if (hasTimeout() && (TIMEOUT_COND())) { *Out << "ERROR: TIMEOUT\n"; X; } } while (0)

#endif
