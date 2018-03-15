#!/usr/bin/env bash

if [ -z "$PARALLEL_MAKE_OPTS" ]
then
    PARALLEL_MAKE_OPTS=-j8
fi

mkdir -p build
cd build

# Choose configuration

cmake -DCONFIG=llvm3.6 ..

# Build external dependencies

make dep_install $PARALLEL_MAKE_OPTS
cmake ..

# Build PAGAI

make $PARALLEL_MAKE_OPTS

echo
echo "PAGAI successfully built"
echo "To run the executable:"
echo
echo "    ./build/pagai --help"
echo
