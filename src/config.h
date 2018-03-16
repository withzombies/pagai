/**
 * \file config.h
 * \brief Declares some utility macros to ease configuration checking
 */
#ifndef CONFIG_UTIL_PAGAI_H
#define CONFIG_UTIL_PAGAI_H

#include "llvm/Config/llvm-config.h"

// Ensure LLVM_VERSION is at least <maj>.<min>
#define LLVM_VERSION_ATLEAST(maj, min) (LLVM_VERSION_MAJOR > (maj) || (LLVM_VERSION_MAJOR == (maj) && LLVM_VERSION_MINOR >= (min)))

// class Metadata has been introduced only after LLVM 3.4
#if LLVM_VERSION_ATLEAST(3, 5)
#   define METADATA_TYPE llvm::Metadata
#else
#   define METADATA_TYPE llvm::Value
#endif

#endif
