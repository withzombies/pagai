/**
 * \file Compare.h
 * \brief Declaration of the Compare pass
 * \author Julien Henry
 */
#ifndef COMPARE_H
#define COMPARE_H

#include "begin_3rdparty.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "end_3rdparty.h"

#include "SMTpass.h"
#include "Node.h"
#include "Abstract.h"
#include "Analyzer.h"
#include "Debug.h"

/**
 * \class CmpResult
 * \brief class that stores the results of the Compare class
 */
class CmpResults {

	public:
		int gt;
		int lt;
		int eq;
		int un;

		CmpResults(): gt(0), lt(0), eq(0), un(0) {};
};

/**
 * \class Compare
 * \brief Pass that compares abstract values computed by each AI pass
 */
class Compare : public llvm::ModulePass {

	public:
		static int compareAbstract(SMTpass * LSMT, Abstract * A, Abstract * B);

	private:
		std::vector<Techniques> ComparedTechniques;

	protected:
		SMTpass * LSMT;

		std::map<
			Techniques,
			std::map<Techniques,CmpResults>
			> results;

		std::map<Techniques, Duration> Time;
		std::map<Techniques, Duration> Time_SMT;

		// count the number of warnings emitted by each technique
		std::map<Techniques, int> Warnings;
		// count the number of safe properties emitted by each technique
		std::map<Techniques, int> Safe_properties;


		void compareTechniques(
			Node * n,
			Techniques t1,
			Techniques t2);

		void CompareTechniquesByPair(Node * n);
		void PrintResultsByPair();

		void ComputeTime(Techniques t, llvm::Function * F);
		void CountNumberOfWarnings(Techniques t, llvm::Function * F);
		void printTime(Techniques t);
		void printWarnings(Techniques t);
		void printSafeProperties(Techniques t);
		void printNumberSkipped(Techniques t);

		void printAllResults();
		void printResults(Techniques t1, Techniques t2);
	public:
		static char ID;

		Compare(std::vector<enum Techniques> & T);
		Compare();

		const char * getPassName() const;
		void getAnalysisUsage(llvm::AnalysisUsage &AU) const;
		bool runOnModule(llvm::Module &M);

};
#endif
