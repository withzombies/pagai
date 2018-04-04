# Tests

## Running Tests

To run all kinds of tests, run `make test` or `ctest`.
The pagai executable and the test executables **must have been built before**, it is not part of the test target.

To build everything required for testing: `make && make build_tests`.

Three types of tests can be run:

- non-regression tests using `ctest -R nonreg`
- assertion-based tests using `ctest -R asserts`
- reproduce known bugs using `ctest -R reproduce_known_bugs`

The last item (reproduce known bugs) can be used to check if it is possible to reproduce a known bug.
If a known bug cannot be reproduced, it may have been corrected (further verification should be done).

## Test Types

### Non-Regression Tests

Non-regression tests : the output is compared to a previously recorded output (= expected).

    ./non_regression/

If the source file is named "file.c" the expected output should be named "file.expected.c".
If multiple outputs are considered valid, they can be named "file.expected.1.c", "file.expected.2.c", etc.

### Assertion-Based Tests

Tests based on assertions (all assertions in the test file must be proved to complete the test).

    ./asserts/

### Command Line Tests

Tests to check command line options.

    ./command_line/

### Known Bugs

Tests that exhibit known bugs (the corresponding ticket number is given for the bug).

    ./known_bugs/

