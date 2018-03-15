# PAGAI Static Analyzer

## Installation

Shorter: on Ubuntu
sudo apt-get install libmpfr-dev libgmp3-dev libboost-all-dev cmake cmake-gui curl

PAGAI needs:

- GMP and MPFR. On Debian systems, install packages `libmpfr-dev` and `libgmp3-dev`
- Boost. On Debian systems, install package `libboost-all-dev`
- CMake, for the compilation. On Debian systems, install package `cmake` (you may want to install `cmake-gui` too for a graphical configuration tool)
- Yices, Z3, CUDD, LLVM, Apron and the PPL (can be automatically installed for you). See the file `CMakeLists.txt` to see the exact versions supported for each configuration.
  The easiest way is probably to go for a local install of all dependencies for the configuration you want.

### Default Build

We provide a script called `default_build.sh` which creates a build directory that, after some compilation time,
contains the PAGAI executable with a default set of options. If you want to change one of those options, use
a custom build.

To clean up the build directory built with `default_build.sh`, run `default_clean.sh`.

### Custom Build

1. Create and initialize a build directory

    ```
    mkdir build && cd build
    cmake <Path to CMakeLists.txt> # e.g. "cmake .."
    ```

    CMake will ask you to choose a configuration (it is the only mandatory option).
    If you don't know which one to use, go for the one with the most recent version of LLVM.
    Also, specify now the other build options you want (see below). For example:

    ```
    cmake -DCONFIG=llvm3.6 [OTHER_OPTIONS] ..
    ```

2. Local install of dependencies

    Most dependencies can be installed locally, which we recommend so that the supported
    versions are used. To install PAGAI's dependencies locally (the destination is the `external/install`
    directory in PAGAI's root dir) run this command from the build directory:

    ```
    make -j4 dep_install
    ```

    This can take some time (dozens of minutes to hours) especially the installation of LLVM.

3. Refresh cmake

    When this is done, refresh one last time the cmake configuration:

    ```
    cmake ..
    ```

    There is no need to specify the options to cmake again, since they are recorded from the
    first time you run the cmake command.

4. Build PAGAI executable

    Finally, you can build the PAGAI executable using `make`.

    After this, you won't have to run `cmake` again, even if you modify PAGAI's source files.
    Just run `make`. The only reason you would want to run `cmake` again would be if you
    want to change one of the options you gave to cmake.

### CMake Options

The CMake script can take various options in the form `-DOPTION_NAME=VALUE`.

The list of available options is visible in the top of the main `CMakeLists.txt` file,
as well as their action on the build system and their default values.

### Compiler Choice

CMake chooses the compiler based on `CC` and `CXX` environment variables.
If you want to force the use of a specific compiler, other than the default
one, use something like:

    export CC=clang
    export CXX=clang++
    cmake [OPTIONS] ..

On MacOS, please compile with clang and not g++: there are strange
template instantiations errors with g++, leading to duplicate symbols
when linking.

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
