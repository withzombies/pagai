/**
 * \file recoverName.h
 * \brief Declaration of the recoverName class
 * \author Rahul Nanda, Julien Henry
 */
#ifndef _RECOVERNAME_H
#define _RECOVERNAME_H

#include <set>
#include <map>

#include "config.h"

#include "begin_3rdparty.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Support/Dwarf.h"
#if LLVM_VERSION_ATLEAST(3, 5)
#   include "llvm/IR/InstIterator.h"
#else
#   include "llvm/Support/InstIterator.h"
#endif
#include "end_3rdparty.h"

#include "Info.h"

// TODO DM
#define LLVM_DEBUG_VERSION (12 << 16)

/**
 * \class compare_Info
 * \brief class for comparing Info objects
 */
class compare_Info
{
	public:
		bool operator()(Info x,Info y) {
			return (x.Compare(y) < 0);
		}
};

/**
 * \class recoverName
 * \brief recover the names of the variables from the source code
 */
class recoverName {
	private :
		static void pass1(llvm::Function *F);
		
		static Info resolveMetDescriptor(llvm::MDNode* md);

		static void update_line_column(llvm::Instruction * I, unsigned & line, unsigned & column);
		static void print_set(std::set<Info, compare_Info> * s);

		static std::set<Info, compare_Info> getPossibleMappings(const llvm::Value * V, std::set<const llvm::Value *> * seen);
		
		static void fill_info_set(
				llvm::BasicBlock * b, 
				std::set<Info> * infos, 
				llvm::Value * val,
				std::set<llvm::BasicBlock*> * seen
				);

	public:
		static std::set<Info> getMDInfos_rec(llvm::Value* v,std::set<llvm::Value*> & seen);
		static Info getMDInfos(const llvm::Value* V);
		static int process(llvm::Function* F);
		static int getBasicBlockLineNo(llvm::BasicBlock* BB);
		static int getBasicBlockColumnNo(llvm::BasicBlock* BB);
		static std::string getSourceFileName(llvm::Function * F);
		static std::string getSourceFileDir(llvm::Function * F);
		static llvm::Instruction * getFirstMetadata(llvm::Function * F);
		static llvm::Instruction * getFirstMetadata(llvm::BasicBlock * b);
		static bool hasMetadata(llvm::Module * M);
		static bool hasMetadata(llvm::Function * F);
		static bool hasMetadata(llvm::BasicBlock * b);
		static bool is_readable(llvm::Function * F);
};

#endif
