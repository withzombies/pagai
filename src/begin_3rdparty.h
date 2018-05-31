/**
 * \file begin_3rdparty.h
 *
 * Include this file before including 3rd-party header files.
 * This will disable warnings from them, to "unpollute" the compilation of PAGAI.
 *
 * It is not necessary to include it for standard header files as they are already warning-less.
 */
#ifdef __GNUC__ // actually recognized by both GCC and clang
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wunused-parameter"
#	pragma GCC diagnostic ignored "-Wstrict-aliasing"
#	pragma GCC diagnostic ignored "-Wpedantic"
#	ifdef __clang__
#		pragma GCC diagnostic ignored "-Winconsistent-missing-override"
#	endif
#endif
