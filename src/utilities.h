/**
 * \file utilities.h
 * \brief Utility functions that do not require external dependencies.
 * \author Denis Becker
 *
 * The utility functions in this file can be unit-tested without needing
 * any extra-dependencies like LLVM or Apron.
 */
#ifndef _UTILITIES_H
#define _UTILITIES_H

#include <string>

namespace utilities {

/**
 * \brief Canonize the given line, assumed it is a linear inequation/equation.
 */
std::string canonize_line(const std::string & line);

}

#endif
