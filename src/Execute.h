/**
 * \file Execute.h
 * \brief Declaration of the Execute class
 * \author Julien Henry
 */
#ifndef _EXECUTE_H
#define _EXECUTE_H

#include <string>
#include <vector>

#include "llvm/Support/FormattedStream.h"

/**
 * \class execute
 * \brief class that creates and runs the llvm passes
 */
class execute {
public :
	void exec(std::string InputFilename, std::string OutputFilename, std::vector<std::string> IncludePaths);
};

#endif
