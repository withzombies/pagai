# PAGAI Static Analyzer

## Installation

Shorter: on Ubuntu
sudo apt-get install libmpfr-dev libgmp3-dev libboost-all-dev cmake cmake-gui curl

PAGAI needs:

- GMP and MPFR. On Debian systems, install packages `libmpfr-dev` and `libgmp3-dev`

- Boost. On Debian systems, install package `libboost-all-dev`

- CMake, for the compilation. On Debian systems, install package `cmake` (you may want to install `cmake-gui` too for a graphical configuration tool)

- Yices, Z3, CUDD, LLVM, Apron and the PPL (can be automatically installed for you). See the top of external/Makefile for required versions of these tools. In particular, PAGAI won't work if you don't have the exact version of LLVM.

- curl or wget, if you want automatic dependencies installation.

The simplest way to compile PAGAI is to run:

    ./autoinstall.sh

from the toplevel. This will download and compile most dependencies
for you (it takes time, and ~550Mb of disk space).

    PARALLEL_MAKE_OPTS=-j4 ./autoinstall.sh

will compile dependencies trying to use 4 cores (adjust as needed).

If you don't want the dependencies to be installed for you, run:

    cmake .
    make

from the toplevel.

Alternatively, one can use an out-of-tree build with:

    mkdir build
    ../autoinstall-out-of-tree.sh

or, without the external dependency installation:

    mkdir build
    cd build
    cmake ..
    make

Variables (path to libraries, build options, ...) can be set for each
build with e.g.:

    cmake -DSTATIC_LINK=ON .
    cmake -i .
    cmake-gui .

To run PAGAI, you should make sure your `$PATH` variable contains the
directory of the SMT solver you want to use (e.g. Z3, CVC4, etc.)

On MacOS, please compile with clang and not g++: there are strange
template instantiations errors with g++, leading to duplicate symbols
when linking.


### Optional APRON extensions

PAGAI can use APRON linked with
- the [Parma Polyhedra Library](http://bugseng.com/products/ppl/) with `-DENABLE_PPL`
- [OptOptagons](https://github.com/eth-srl/OptOctagon) from ETHZ with `-DENABLE_OPT_OCT`

## Usage 

`pagai -h` should give you the list of arguments.

### With a C input

If you want to use PAGAI with a C input, you first need to setup a `pagai.conf` file in
the same directory as the PAGAI executable. This conf file contains only one variable so
far, named `ResourceDir`. This variable should be set to the path of your LLVM/Clang
installation. In particular, make sure you set `ResourceDir` so that the path
`$(ResourceDir)/lib/clang/3.4/include/` exists and contains e.g. `float.h`.
	
The `pagai.conf` file should have the form:

    "ResourceDir <dir>"

Example:

    ~$ cat pagai.conf
	ResourceDir /Users/julien/work/pagai/external/llvm

### With a LLVM `.bc` input

The `pagai.conf` file is only used for compiling C/C++ code into LLVM bitcode. If you
directly give to PAGAI a bitcode file, it will process it. Note that you will need to
compile into bitcode with clang and the `-g` and `-emit-llvm` arguments.

## Example 

    ~$ ./pagai ../ex/wcet_bicycle2.c

```c
// ResourceDir is /Users/julien/work/pagai/external/llvm/lib/clang/3.4
// analysis: AIopt
/* processing Function bicycle */
#include "../pagai_assert.h"

void bicycle() {
    int /* reachable */
        count=0, phase=0;
    for(int i=0; i<10000; // safe
                          i++) {
        /* invariant:
        -2*count+phase+3*i = 0
        14998-count+phase >= 0
        1-phase >= 0
        phase >= 0
        count-2*phase >= 0
        */
        if (phase == 0) {
            // safe
            count += 2; phase = 1;
        } else if (phase == 1) {
            // safe
            count += 1; phase = 0;
        }
    }
    /* assert OK */
    assert(count <= 15000);
    /* reachable */
}
```
