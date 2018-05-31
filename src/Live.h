/**
 * \file Live.h
 * \brief Declaration of the Live class
 * \author Julien Henry
 */
#ifndef LIVE_H
#define LIVE_H

#include "begin_3rdparty.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/LoopInfo.h"
#include "end_3rdparty.h"

/**
 * \class Live
 * \brief Liveness analysis
 *
 * Analysis that provides liveness information for
 * LLVM IR Values.
 */
class Live : public llvm::FunctionPass {

	private:

		//LoopInfo *LI;

		/**
		 * \brief A bunch of state to be associated with a value.
		 */
		struct Memo {
			/**
			 * \brief The set of blocks which contain a use of the value.
			 */
			llvm::SmallPtrSet< llvm::BasicBlock *, 4> Used;
			llvm::SmallPtrSet< llvm::BasicBlock *, 4> UsedPHI;

			/**
			 * \brief A conservative approximation of the set of blocks in
			 * which the value is live-through, meaning blocks dominated
			 * by the definition, and from which blocks containing uses of the
			 * value are reachable.
			 */
			llvm::SmallPtrSet< llvm::BasicBlock *, 4> LiveThrough;
			llvm::SmallPtrSet< llvm::BasicBlock *, 4> LiveThroughPHI;
		};

		/**
		 * \brief Remembers the Memo for each Value. This is populated on
		 * demand.
		 */
		llvm::DenseMap< llvm::Value *, Memo> Memos;

		/**
		 * \brief Retrieve an existing Memo for the given value if one
		 * is available, otherwise compute a new one.
		 */
		Memo &getMemo( llvm::Value *V);

		/**
		 * \brief Compute a new Memo for the given value.
		 */
		Memo &compute( llvm::Value *V);

	public:
		static char ID;
		Live();

		const char * getPassName() const;
		virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const;
		virtual bool runOnFunction(llvm::Function &F);
		virtual void releaseMemory();

		/**
		 * \brief Test if the given value is used in the given block.
		 */
		bool isUsedInBlock( llvm::Value *V, llvm::BasicBlock *BB);
		bool isUsedInPHIBlock( llvm::Value *V, llvm::BasicBlock *BB);

		bool isLiveByLinearityInBlock(llvm::Value *V, llvm::BasicBlock *BB, bool PHIblock);

		/**
		 * \brief Test if the given value is known to be
		 * live-through the given block
		 *
		 * Live through means that the block is properly
		 * dominated by the value's definition, and there exists a block
		 * reachable from it that contains a use.
		 */
		bool isLiveThroughBlock( llvm::Value *V, llvm::BasicBlock *BB, bool PHIblock);
};


#endif

