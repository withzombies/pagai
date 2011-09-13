#ifndef _AIGOPAN_H
#define _AIGOPAN_H

#include <queue>
#include <vector>

#include "llvm/Module.h"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/InstVisitor.h"
#include "llvm/Analysis/LoopInfo.h"

#include "ap_global1.h"

#include "apron.h"
#include "Node.h"
#include "Live.h"
#include "SMT.h"
#include "PathTree.h"
#include "AISimple.h"

using namespace llvm;

/// @brief Gopan&Reps Implementation.
///
/// This class is almost identical to AIClassic, the only difference
/// being the abstract domain (which uses a main value to decide which
/// paths are to be explored, and a pilot value to actually compute
/// the invariants).
class AIGopan : public AISimple {

	public:
		/// @brief Pass Identifier
		///
		/// It is crucial for LLVM's pass manager that
		/// this ID is different (in address) from a class to another,
		/// hence this cannot be factored in the base class.
		static char ID;	

	public:

		AIGopan ():
			AISimple(ID)
			{
				aman = new AbstractManGopan();
				//aman = new AbstractManClassic();
				passID = LOOKAHEAD_WIDENING;
				Passes[LOOKAHEAD_WIDENING] = passID;	
			}

		~AIGopan () {
			}

		const char *getPassName() const;
};

#endif
