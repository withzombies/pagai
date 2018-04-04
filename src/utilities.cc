#include <vector>
#include <sstream>
#include <algorithm>

#include "utilities.h"

namespace utilities {

void custom_split_terms(const std::string & str, const std::string & delims, std::vector<std::string> & terms)
{
	size_t last_i = 0;
	for (size_t i = 0; i < str.length(); ++i) {
		if (delims.find(str[i]) != std::string::npos && last_i < i) {
			terms.emplace_back(str.substr(last_i, i - last_i));
			last_i = i;
		}
	}
	terms.emplace_back(str.substr(last_i));

	// add '+' to first term if it has no explicit minus sign
	if (terms[0][0] != '-') {
		terms[0] = "+" + terms[0];
	}
}

std::string canonize_line(const std::string & line)
{
	std::ostringstream canonized;

	size_t lindex = line.find(' '); // there is no space in the "left" part (before the >= or =)
	std::string left_part = line.substr(0, lindex);
	std::string right_part = line.substr(lindex);

	// reorder main sum
	std::vector<std::string> terms;
	custom_split_terms(left_part, "+-", terms);
	std::sort(terms.begin(), terms.end());

	// by convention, for equalities, we force the first term to be positive (otherwise it may change from run to run)
	if (right_part == " = 0") {
		if (terms[0][0] == '-') {
			// invert sign of each term
			for (size_t i = 0; i < terms.size(); ++i) {
				terms[i][0] = (terms[i][0] == '+' ? '-' : '+');
			}
		}
	}

	// remove first '+' if any
	if (terms[0][0] == '+') {
		terms[0] = terms[0].substr(1);
	}

	for (const std::string & term : terms) {
		canonized << term;
	}
	canonized << right_part;

	return canonized.str();
}

} // end namespace utilities
