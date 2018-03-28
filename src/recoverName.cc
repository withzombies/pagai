/**
 * \file recoverName.cc
 * \brief Implementation of the recoverName class
 * \author Rahul Nanda, Julien Henry
 */
#include <algorithm>
#include <fstream>

#include "config.h"

#include "begin_3rdparty.h"
#if LLVM_VERSION_ATLEAST(3, 5)
#	include "llvm/IR/Dominators.h"
#	include "llvm/IR/DebugInfo.h"
#else
#	include "llvm/DebugInfo.h"
#	include "llvm/Analysis/Dominators.h"
#endif
#include "llvm/Support/Dwarf.h"
#include "end_3rdparty.h"

#include "recoverName.h"
#include "Debug.h"
#include "SMTpass.h"

#define MAX 0xFFFFFFFF

using namespace llvm;

// map M1 for pass1
std::multimap<const Value*,Info> M1;

//Basic Block Mapping: 
//Block_line maps a basic block to starting line no. in original source code
//Block_column maps a basic block to column no. in original code.
std::map<BasicBlock*,int> Block_line,Block_column; 

std::map<Value*,Info> computed_mappings;

void recoverName::fill_info_set(BasicBlock * b, std::set<Info> * infos, Value * val, std::set<BasicBlock*> * seen) {
	seen->insert(b);
	for (BasicBlock::iterator I = b->begin(), E = b->end(); I != E; ++I) {
		if (const DbgValueInst *DVI=dyn_cast<DbgValueInst>(I)) {
			if (DVI->getValue() == val) {
				//*Out << "match found between " << *val << " " << *DVI->getValue() << "\n";
				MDNode * MD = DVI->getVariable(); 
				Info varInfo = resolveMetDescriptor(MD);
				infos->insert(varInfo);
			}
		}
	}
	for (pred_iterator PI = pred_begin(b), E = pred_end(b); PI != E; ++PI) {
		BasicBlock *pred = *PI;
		if (seen->count(pred) == 0)
			fill_info_set(pred,infos,val, seen);
	}
}

std::set<Info> recoverName::getMDInfos_rec(Value* v,std::set<Value*> & seen) {
	std::set<Info> res_infos;
	PHINode * phi = dyn_cast<PHINode>(v);
	if (phi == NULL) {
		std::pair<std::multimap<const Value*,Info>::iterator,std::multimap<const Value*,Info>::iterator> in_Map;
		in_Map=M1.equal_range(v);
		std::multimap<const Value*,Info>::iterator itt;
		for (itt = in_Map.first; itt!=in_Map.second; itt++) {
			res_infos.insert(itt->second);
		}
		return res_infos;
	}

	for (unsigned i = 0; i < phi->getNumIncomingValues(); i++) {
		Value * val = phi->getIncomingValue(i);
		if (isa<UndefValue>(val)) continue;
		BasicBlock * b = phi->getIncomingBlock(i);
		std::set<Info> infos;
		std::set<BasicBlock*> block_seen;
		fill_info_set(b,&infos,val,&block_seen);
		if (infos.size() == 0 && !seen.count(val) && isa<PHINode>(val)) {
		//if (infos.size() == 0 && !seen.count(val)) {
			std::set<Value*> s;
			s.insert(seen.begin(),seen.end());
			s.insert(val);
			std::set<Info> res = getMDInfos_rec(val,s);
			infos.insert(res.begin(),res.end());
		}
		if (res_infos.empty()) {
			res_infos.insert(infos.begin(),infos.end());
		} else if (infos.size() > 0) {
			std::set<Info> new_res_infos;
			std::set<Info>::iterator it = res_infos.begin(), et = res_infos.end();
			for (; it != et; it++) {
				if (infos.count(*it)) new_res_infos.insert(*it);
			}
			res_infos.clear();
			res_infos.insert(new_res_infos.begin(),new_res_infos.end());
		}
	}
	return res_infos;
}

Info recoverName::getMDInfos(const Value* V) {
	Value * v = const_cast<Value*>(V);
	if (computed_mappings.count(v)) return computed_mappings[v];
	
	if (PHINode * phi = dyn_cast<PHINode>(v)) {
		//*Out << "getMD " << *phi << "\n";
		std::set<Value*> seen;
		seen.insert(phi);
		std::set<Info> s = getMDInfos_rec(v,seen);
		if (s.empty()) {
			computed_mappings[v] = Info(SMTpass::getVarName(v),-1,"unknown");
		} else if (s.size() > 1) {
			// there are several choices...
			// we choose one depending on teh syntactic name of the LLVM
			// variable
			std::string syntactic_name = SMTpass::getVarName(v);
			std::set<Info>::iterator it = s.begin(), et = s.end();
			bool found = false;
			for (; it != et; it++) {
				if (it->getName().compare(syntactic_name)) {
					computed_mappings[v] = *it;
					found = true;
				}
			}
			if (!found)
				computed_mappings[v] = Info(SMTpass::getVarName(v),-1,"unknown");
		} else {
			computed_mappings[v] = *s.begin();
		}
		return computed_mappings[v];
	}
	std::pair<std::multimap<const Value*,Info>::iterator,std::multimap<const Value*,Info>::iterator> ret1;
	ret1=M1.equal_range(V);
	std::multimap<const Value*,Info>::iterator it;

	for (it = ret1.first; it != ret1.second; it++) {
		computed_mappings[v] = (it)->second;
		if (isa<Argument>(v) && !(it)->second.IsArg()) continue;
		if (isa<GlobalValue>(v) && !(it)->second.IsGlobal()) continue;
		computed_mappings[v] = it->second;
		DEBUG(
		*Out << "for " << *V << " " ;
		it->second.display();
		*Out <<  "\n";
		);
		return (it)->second;
	}

	std::set<const Value*> seen;
	seen.insert(V);
	std::set<Info,compare_Info> possible_mappings = getPossibleMappings(V,&seen);

	if (possible_mappings.begin() == possible_mappings.end()) {
		DEBUG(
		*Out << "no possible mappings for " << *V << "\n...";
		);
		return Info(SMTpass::getVarName(v),-1,"unknown");
	}
	computed_mappings[v] = Info(*possible_mappings.begin());
	return Info(*possible_mappings.begin());
}

// for debugging purpose...
void recoverName::print_set(std::set<Info,compare_Info> * s) {
	std::set<Info>::iterator it = s->begin(), et = s->end();
	for (; it != et; it++) {
		(*it).display();
	}
	*Out << "\n";
}

std::set<Info,compare_Info> recoverName::getPossibleMappings(const Value * V, std::set<const Value *> * seen) {
	std::set<Info,compare_Info> res;
	std::pair<std::multimap<const Value*,Info>::iterator,std::multimap<const Value*,Info>::iterator> in_Map;
	in_Map=M1.equal_range(V);
	std::multimap<const Value*,Info>::iterator it;

	bool empty = true;
	for (it = in_Map.first; it!=in_Map.second; it++) {
		empty = false;
		res.insert(it->second);
	}
	if (empty) {
		// V should be a PHINode
		if (! isa<PHINode>(V)) return res; // can occur for instance when there is an undef as PHINode argument
		const PHINode * phi = dyn_cast<PHINode>(V);
		seen->insert(V);
		for (unsigned i = 0; i < phi->getNumIncomingValues(); i++) {
			Value * v = phi->getIncomingValue(i);
			if (seen->count(v) > 0) continue;
			std::set<Info,compare_Info> s = getPossibleMappings(v,seen);
			if (res.empty()) {
				res.swap(s);
			} else {
				// computing the intersection
				std::set<Info,compare_Info>::iterator it1 = res.begin();
				std::set<Info,compare_Info>::iterator it2 = s.begin();
				while ( (it1 != res.end()) && (it2 != s.end()) ) {
					if (*it1 < *it2) {
						res.erase(it1++);
					} else if (*it2 < *it1) {
						++it2;
					} else { // **it1 == **it2
						++it1;
						++it2;
					}
				}
				// Anything left in set_1 from here on did not appear in set_2,
				// so we remove it.
				res.erase(it1, res.end());	
			}

		}
		seen->erase(V);
	}
	return res;
}

//process function involves calling two functions 'pass1' and then 'pass2', passing the argument Function*
//pass1 and pass2 create maps M1 and M2 for all const Value* present in Function* passed to process function.
int recoverName::process(Function *F) { 
	pass1(F);	
	std::multimap<const Value*,Info>::iterator itt;
	DEBUG(
		*Out<<"MAPPING OF VARIABLES ...\nMap1\n";
		for ( itt=M1.begin() ; itt != M1.end(); itt++ ) {
			*Out<< *(*itt).first << " => ";
			(*itt).second.display();
			*Out<<"\n";
		}
	);
	return 1;
}

int recoverName::getBasicBlockLineNo(BasicBlock* BB) {
	std::map<BasicBlock*,int>::iterator it;
	it=Block_line.find(BB);
	if(it!=Block_line.end())
	{
		return it->second;
	}
	else
		return -1;
}

int recoverName::getBasicBlockColumnNo(BasicBlock* BB) {
	std::map<BasicBlock*,int>::iterator it;
	it=Block_column.find(BB);
	if(it!=Block_column.end())
	{
		return it->second;
	}
	else
		return -1;
}

/*
 * This function collects required information about a variable from a MDNode*, creates the corresponding object
 * of type Info and returns it.
 */
Info recoverName::resolveMetDescriptor(MDNode* md) {
	DIVariable descriptor(md);
	std::string name;
	std::string type;
	int lineNo;
	auto tag = descriptor.getTag();
	
	switch(tag) {
		case dwarf::DW_TAG_auto_variable:
		case dwarf::DW_TAG_arg_variable:
		case dwarf::DW_TAG_variable:
			lineNo = descriptor.getLineNumber();
			name = descriptor.getName();
			type = "unknown";
			return Info(name, lineNo, type, true, false, false, false);
	}	
	assert(false);
}

void recoverName::update_line_column(Instruction * I, unsigned & line, unsigned & column) {
	MDNode *BlockMD = I->getMetadata("dbg");
	if (BlockMD == NULL) return;

// MDLocation was introduced after LLVM 3.4
#if LLVM_VERSION_ATLEAST(3, 5)
	MDLocation *location = dyn_cast<MDLocation>(BlockMD);
	if (location == NULL) {
		return;
	}

	auto l = location->getLine();
	auto c = location->getColumn();
#else
	unsigned l = 0;
	unsigned c = 0;
	if (const ConstantInt *BBLineNo = dyn_cast<ConstantInt>(BlockMD->getOperand(0))) {
		l = BBLineNo->getZExtValue();
	}
	if (const ConstantInt *BBColumnNo = dyn_cast<ConstantInt>(BlockMD->getOperand(1))) {
		c = BBColumnNo->getZExtValue();
	}
#endif

	if (l < line) {
		line = l;
		column = c;
	} else if (l == line && c < column) {
		column = c;
	}
}

//'pass1' reads llvm.dbg.declare and llvm.dbg.value instructions, maps bitcode variables(of type Value*)
//to original source variable(of type Info*) and stores them in multimap M1
void recoverName::pass1(Function *F) {
	MDNode * MD; 
	const Value* val;
	for (Function::iterator bb = F->begin(), e = F->end(); bb != e; ++bb) {
		unsigned bbline=MAX,bbcolumn=MAX;
		for (BasicBlock::iterator I = bb->begin(), E = bb->end(); I != E; ++I) {
			//now check if the instruction is of type llvm.dbg.value or llvm.dbg.declare
			bool dbgInstFlag=false;
			if(const DbgValueInst *DVI=dyn_cast<DbgValueInst>(I)) {
				val=DVI->getValue();
				MD = DVI->getVariable(); 
				dbgInstFlag=true;
			} else if(const DbgDeclareInst *DDI=dyn_cast<DbgDeclareInst>(I)) {	
				val=DDI->getAddress();
				MD = DDI->getVariable(); 
				dbgInstFlag=true;
			}

			/*
			 * We do not take llvm.dbg.* instructions into account
			 * for line/columns computations since some of them
			 * point to function parameters, resulting in annoying
			 * "reachable" annotations inside the parameters list
			 * if we keep them.
			 */
			if (!dbgInstFlag) {
				update_line_column(I, bbline, bbcolumn);
			} else {
				Info varInfo=resolveMetDescriptor(MD);
				if(!varInfo.empty()) {
					std::pair<std::multimap<const Value*,Info>::iterator,std::multimap<const Value*,Info>::iterator> ret1;
					ret1=M1.equal_range(val);

					bool AlreadyMapped=false;
					//this is to check and avoid duplicate entries in the map M1 
					for(std::multimap<const Value*,Info>::iterator it=ret1.first;it!=ret1.second;it++) {
						if(varInfo == (it->second)) {
							AlreadyMapped=true;
							break;
						} 
					}
					if(!AlreadyMapped) {
						std::pair<const Value*,Info>hi=std::make_pair(val,varInfo);
						M1.insert(hi);
					}
				}
			}
		}
		Block_line.insert(std::make_pair(bb,bbline));
		// bbcolumn is set to 1 at least, since new versions of LLVM always set it to 0
		// instead of the correct column number...
		// if we keep 0, invariants won't be printed correctly
		Block_column.insert(std::make_pair(bb,(1>bbcolumn)?1:bbcolumn));
	}	
}

Instruction * recoverName::getFirstMetadata(Function * F) {
	Function::iterator it = F->begin(), et = F->end();
	for (;it!=et;it++) {
		Instruction * res = getFirstMetadata(it);
		if (res != NULL) return res;
	}
	return NULL;
}

Instruction * recoverName::getFirstMetadata(BasicBlock * b) {
	BasicBlock::iterator it = b->begin(), et = b->end();
	for (;it!=et;it++) {
		if (it->hasMetadata()) return it;
	}
	return NULL;
}

std::string recoverName::getSourceFileName(Function * F) {
	const Instruction * I = getFirstMetadata(F);
	if (I == NULL) return "";
	const MDNode * dbg = I->getMetadata("dbg");
	if (dbg == NULL) return "";
	const DILocation location = DILocation(dbg);
	return location.getFilename();
}

std::string recoverName::getSourceFileDir(Function * F) {
	const Instruction * I = getFirstMetadata(F);
	if (I == NULL) return "";
	const MDNode * dbg = I->getMetadata("dbg");
	if (dbg == NULL) return "";
	const DILocation location = DILocation(dbg);
	return location.getDirectory();
}

bool recoverName::is_readable(Function * F) { 
	std::string dir = getSourceFileDir(F);
	std::string name = getSourceFileName(F);
	if (dir.size() == 0 || name.size() == 0) return false;
	std::string dirname;
	if (name[0] == '/') {
		dirname = name;
	} else {
		dirname = dir + "/" + name;
	}
	std::ifstream File(dirname.c_str()); 
	return !File.fail(); 
}

bool recoverName::hasMetadata(Module * M) {
	Module::iterator it = M->begin(), et = M->end();
	for (;it!=et;it++) {
		if (hasMetadata(it)) return true;
	}
	return false;
}

bool recoverName::hasMetadata(Function * F) {
	Function::iterator it = F->begin(), et = F->end();
	for (;it!=et;it++) {
		if (hasMetadata(it)) return true;
	}
	return false;
}

bool recoverName::hasMetadata(BasicBlock * b) {
	BasicBlock::iterator it = b->begin(), et = b->end();
	for (;it!=et;it++) {
		if (it->hasMetadata()) return true;
	}
	return false;
}

