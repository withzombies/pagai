#!/usr/bin/env bash

if [ -z "$PARALLEL_MAKE_OPTS" ]
then
	PARALLEL_MAKE_OPTS=-j8
fi

mkdir -p build_cov
cd build_cov

cmake -DCONFIG=llvm3.6 -DENABLE_COVERAGE=ON ..

# We suppose dependencies are already installed for this configuration

make $PARALLEL_MAKE_OPTS
make build_tests $PARALLEL_MAKE_OPTS
ctest

# Coverage

make coverage

echo
echo "Code coverage directory successfully built"
echo "To view the results:"
echo
echo "    firefox build_cov/coverage/index.html"
echo
