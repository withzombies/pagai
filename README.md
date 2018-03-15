# PAGAI Static Analyzer

## Installation

Shorter: on Ubuntu
sudo apt-get install libmpfr-dev libgmp3-dev libboost-all-dev cmake cmake-gui curl

PAGAI needs:

- GMP and MPFR. On Debian systems, install packages `libmpfr-dev` and `libgmp3-dev`
- Boost. On Debian systems, install package `libboost-all-dev`
- CMake, for the compilation. On Debian systems, install package `cmake` (you may want to install `cmake-gui` too for a graphical configuration tool)
- Yices, Z3, CUDD, LLVM, Apron and the PPL (can be automatically installed for you). See the file `cmake/external_projects_config.cmake` to see the exact versions required for each configuration.
  The easiest way is probably to go for a local install of all dependencies for the configuration you want.

To compile PAGAI, run the following commands:

    mkdir build
    cd build
    cmake [OPTIONS] ..  # The list of options is documented below
    make -j4            # Adjust "-j4" depending on the number
                        # of parallel tasks you want to run

The `cmake` command will very likely output an error saying that some dependency could not be found on your system.
Indeed, we require specific versions of libraries that are generally not available using package managers.
You can ask for an automatic download/install of those dependencies using the following command (after having run
`cmake` once):

    make -j4 dep_install    # This can take some time (some dozens of minutes)

When this is done, you can finally build PAGAI:

    cmake ..            # The configuration is kept in cache files, so
                        # you don't have to specify all the arguments again
    make -j4

After this, you won't have to run `cmake` again, even if you modify PAGAI's source files.
Just run `make` again. The only reason you would want to run `cmake` again would be if you
want to change one of the options you gave to cmake.

### Compiler Choice

CMake chooses the compiler based on `CC` and `CXX` environment variables.
If you want to force the use of a specific compiler (and that your `CC`
or `CXX` variables are not set) use something like:

    CC=clang CXX=clang++ cmake [OPTIONS] ..

On MacOS, please compile with clang and not g++: there are strange
template instantiations errors with g++, leading to duplicate symbols
when linking.

### CMake Options

The CMake script can take various options in the form `-DOPTION_NAME=VALUE`.
The list of available options is:

- `CONFIG`=`CONFIG_NAME`    Use a specific set of versions for dependencies.
                            Each configuration corresponds to a specific set of versions that
                            are known to be supported by PAGAI. The list of configurations
                            can either be seen by looking at the main `CMakeLists.txt`, or by
                            typing `cmake ..` without anything else (from an empty sub-directory).
- `ENABLE_PPL`=`ON/OFF`     Compile and link APRON with [Parma Polyhedra Library](http://bugseng.com/products/ppl/).
                            Defaults to `OFF` (i.e. uses NewPolka).
- `ENABLE_OPT_OCT`=`ON/OFF` Compile and link APRON with [OptOptagons](https://github.com/eth-srl/OptOctagon)
                            from ETHZ. Defaults to `OFF`.

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
