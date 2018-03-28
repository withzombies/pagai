/**
 * \file apron_MD.h
 * \brief Declares functions related to the Apron interface
 * \author Julien Henry
 */
#ifndef _APRON_MD_H 
#define _APRON_MD_H 

#include "begin_3rdparty.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/IR/IRBuilder.h"

#include "ap_global1.h"
#include "end_3rdparty.h"

#include "Analyzer.h"
#include "Node.h"

void ap_tcons1_t_to_MDNode(ap_tcons1_t & cons, llvm::Instruction * Inst, std::vector<METADATA_TYPE*> * met);

void coeff_to_MDNode(ap_coeff_t * a, llvm::Instruction * Inst, std::vector<METADATA_TYPE*> * met);
void ap_texpr1_t_to_MDNode(ap_texpr1_t & expr, llvm::Instruction * Inst, std::vector<METADATA_TYPE*> * met);

void texpr0_to_MDNode(ap_texpr0_t* a, ap_environment_t * env, llvm::Instruction * Inst, std::vector<METADATA_TYPE*> * met);
void texpr0_node_to_MDNode(ap_texpr0_node_t * a, ap_environment_t * env, llvm::Instruction * Inst, std::vector<METADATA_TYPE*> * met);

llvm::Value * ap_tcons1_to_LLVM(ap_tcons1_t & cons, llvm::IRBuilder<> * Builder);
llvm::Value * ap_texpr1_to_LLVM(ap_texpr1_t & expr, llvm::IRBuilder<> * Builder); 
llvm::Value * texpr0_to_LLVM(ap_texpr0_t* a, ap_environment_t * env, llvm::IRBuilder<> * Builder);
llvm::Value * texpr0_node_to_LLVM(ap_texpr0_node_t * a, ap_environment_t * env, llvm::IRBuilder<> * Builder);
llvm::Value * ap_scalar_to_LLVM(ap_scalar_t & scalar, llvm::IRBuilder<> * Builder);
llvm::Value * coeff_to_LLVM(ap_coeff_t * a, llvm::IRBuilder<> * Builder);

#endif
