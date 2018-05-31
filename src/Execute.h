/**
 * \file Execute.h
 * \brief Declaration of the Execute class
 * \author Julien Henry
 */
#ifndef _EXECUTE_H
#define _EXECUTE_H

#include <string>
#include <vector>

/**
 * \class execute
 * \brief class that creates and runs the llvm passes
 */
class execute {

	public:
		void exec(const std::string & InputFilename, const std::string & OutputFilename, const std::vector<std::string> & IncludePaths);

};

#endif
