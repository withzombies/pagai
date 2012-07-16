#include <llvm/Pass.h>
#include <llvm/Function.h>
#include <llvm/Instructions.h>
#include <llvm/Support/InstVisitor.h>

using namespace llvm;

class BoolSimpl : public BasicBlockPass,
  public InstVisitor<BoolSimpl, void> {
 public:
  static char ID;
  BoolSimpl() : BasicBlockPass(ID) {}

  void visitICmpInst(ICmpInst &I);

  virtual bool runOnBasicBlock(BasicBlock &F);
};
