#include "llvm/Module.h"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/LinkAllVMCore.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/Target/TargetData.h"
#include "llvm/CodeGen/LinkAllCodegenComponents.h"
#include "llvm/Transforms/Scalar.h"

#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/LoopInfo.h"

#include "AI.h"
#include "Node.h"
#include "Execute.h"
#include "Live.h"
#include "SMT.h"

using namespace llvm;

void execute::exec(std::string InputFilename, std::string OutputFilename) {

	Module *M = NULL;
	raw_fd_ostream *FDOut = NULL;
	
	LLVMContext & Context = getGlobalContext();

	std::string ErrorMessage;

	if (MemoryBuffer * Buffer
			= MemoryBuffer::getFileOrSTDIN(InputFilename, &ErrorMessage)) {
		M = ParseBitcodeFile(Buffer, Context, &ErrorMessage);
		delete Buffer;
	} else {
		ferrs() << "Not able to initialize module from bitcode\n";
		return;
	}

	if (OutputFilename != "") {

		std::string error;
		FDOut = new raw_fd_ostream(OutputFilename.c_str(), error);
		if (!error.empty()) {
			errs() << error << '\n';
			delete FDOut;
			return;
		}
		Out = new formatted_raw_ostream(*FDOut, formatted_raw_ostream::DELETE_STREAM);

		// Make sure that the Output file gets unlinked from the disk if we get a
		// SIGINT
		sys::RemoveFileOnSignal(sys::Path(OutputFilename));
	}

	// Build up all of the passes that we want to do to the module.
	PassManager Passes;

	ModulePass *AIPass = new AI();
	FunctionPass *LoopInfoPass = new LoopInfo();

	Passes.add(new TargetData(M));
	Passes.add(createVerifierPass());
	Passes.add(createGCLoweringPass());
	//Passes.add(createLowerInvokePass());
	Passes.add(createPromoteMemoryToRegisterPass());
	Passes.add(createLoopSimplifyPass());	
	Passes.add(LoopInfoPass);

	// this pass converts SwitchInst instructions into a sequence of
	// chained binary branch instructions, much easier to deal with
	Passes.add(createLowerSwitchPass());	
	
	Passes.add(new Live());
	Passes.add(new SMT());
	Passes.add(AIPass);

	Passes.run(*M);

	//Out->flush();
	//delete FDOut;
	//delete Out;
	//delete AIPass;
	//delete LoopInfoPass;
}
