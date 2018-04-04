#include <iostream>

#include "utilities.h"

void test(const std::string & line_in, const std::string & line_exp)
{
    std::string line_out = utilities::canonize_line(line_in);
    if (line_out != line_exp) {
        std::cerr << "output: \"" << line_out << "\" | expected: \"" << line_exp << "\"" << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

int main()
{
    // note: the sign of terms is included in the lexico comparison "+4" >= "-5" (lexicographically)

    test("1+2 <= 0", "1+2 <= 0"); // do not change if order is already OK
    test("2+1 >= 0", "1+2 >= 0"); // swap order (lexico)
    test("a+b+d-c-e >= 0", "a+b+d-c-e >= 0"); // do not change order if already OK
    test("-a+e-d-c-b >= 0", "e-a-b-c-d >= 0"); // swap order (lexico) (with letters)
    test("-3*a+4*b+c-58 >= 0", "4*b+c-3*a-58 >= 0"); // swap order (lexico) (linear terms)
    test("a+b+c = 0", "a+b+c = 0"); // do not change order if OK (equality)
    test("-589*i-98*c-d = 0", "589*i+98*c+d = 0"); // first sign positive by convention (equality)
    test("3*x.1+2*a-89 >= 0", "2*a+3*x.1-89 >= 0"); // variable of type "x.1"

    return EXIT_SUCCESS;
}
