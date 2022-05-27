The branch is used to test the compiler support of C++20 modules, explore the
practice with C++20 modules and compare the coding style between header based
and named module based.

The non-module version lives in the async_simple directory and the module
version lives in async_simple_module and third_party_module directory. We could
find the usage of named modules in async_simple_module and we could find the
methods to wrap a existing library into a named module in third_party_module
directory.

Due to the lack of modules support for C++20 modules in linux environments,
the build script is written in Makefile.

This could be compiled by an internal branch of clang and we're going to
contirbute it.
