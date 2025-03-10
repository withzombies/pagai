#include <boost/format.hpp>

#include "begin_3rdparty.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/IRBuilder.h"
#include "end_3rdparty.h"

#include "RemoveUndet.h"

using namespace llvm;

Constant* RemoveUndet::getNondetFn (Module * M, Type * type) {
	Constant* res = undet_functions[type];
	if (res == NULL) {
		res = M->getOrInsertFunction
			(boost::str
			 (boost::format ("nondet_%d") % undet_functions.size ()), type, NULL);

		if (Function *f = dyn_cast<Function>(res)) {
			f->setAttributes(AttributeSet::get(getGlobalContext(),AttributeSet::FunctionIndex,Attribute::ReadNone));
			f->setAttributes(AttributeSet::get(getGlobalContext(),AttributeSet::FunctionIndex,Attribute::NoUnwind));
		}
		undet_functions[type] = res;
	}
	return res;
}

bool RemoveUndet::runOnModule(Module &M) {
	bool Changed = false;

	//Iterate over all functions, basic blocks and instructions.
	for (Module::iterator FI = M.begin(); FI != M.end(); ++FI) {
		for (Function::iterator b = FI->begin(); b != FI->end(); ++b) {
			BasicBlock::iterator i = b->begin();
			while (i != b->end()) {
				if (AllocaInst *AI = dyn_cast<AllocaInst>(i)) {
					Constant *fun = getNondetFn(&M, AI->getAllocatedType());

					IRBuilder<> Builder(FI->getContext());
					Builder.SetInsertPoint(&*(++i));
					Builder.CreateStore (Builder.CreateCall(fun), AI);
					Changed = true;
				} else {
					++i;
				}
			}
		}
	}

	undet_functions.clear();
	return Changed;
}

char RemoveUndet::ID = 0;
static RegisterPass<RemoveUndet> X("removeundet", "initialise alloca inst", false, false);
