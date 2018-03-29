/**
 * \file AIpf_incr.h
 * \brief Declaration of the AIpf_incr pass
 * \author Julien Henry
 */
#ifndef _AIPF_INCR_H
#define _AIPF_INCR_H

#include <queue>
#include <vector>

#include "AIpf.h"

/**
 * \class AIpf_incr
 * \brief Abstract Interpretation with Path Focusing algorithm (using SMT-solving) that uses the result of a previous analysis
 */
class AIpf_incr : public AIpf {

	public:
		static char ID;	

	public:

		AIpf_incr(char &_ID, Apron_Manager_Type _man, bool _NewNarrow, bool _Threshold) : AIpf(_ID,_man,_NewNarrow,_Threshold) {
			init();
		}

		AIpf_incr (): AIpf(ID) {
			init();
		}
		
		void init()
			{
				passID.T = PATH_FOCUSING_INCR;
			}

		~AIpf_incr () {}

		const char *getPassName() const;

		void getAnalysisUsage(llvm::AnalysisUsage &AU) const;

		void assert_properties(params P, llvm::Function * F);
		void intersect_with_known_properties(Abstract * Xtemp, Node * n, params P);

};

#endif
