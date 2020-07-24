# How to build the librpma library from source

## Requirements

### Common requirements

In order to build librpma, you need to have installed several components:

- C compiler
- [CMake](http://www.cmake.org) >= 3.3
- libibverbs-dev(el)
- librdmacm-dev(el)
- groff
- cmocka-dev(el) == 1.1.5-26-g672c5ce (please see https://github.com/pmem/rpma/blob/master/utils/docker/images/install-cmocka.sh to install the verified revision)
- txt2man == 1.7.0 (please see https://github.com/pmem/rpma/blob/master/utils/docker/images/install-txt2man.sh to install the verified revision)

### For some examples you also need:

- libpmem-dev(el) >= 1.6

Note: the above revisions are proven to work correctly.

## Building
First, you need to configure the compilation, using CMake. Go inside the
`build` dir. Create it if it doesn't exist.

```shell
$ mkdir build && cd build
$ cmake ..
$ make -j
```

### CMake standard options
Here is a list of the most interesting options
provided out of the box by CMake.

| Name | Description | Values |
| --- | --- | --- |
| BUILD_DOC | Build the documentation | ON/OFF |
| BUILD_EXAMPLES | Build the examples | ON/OFF |
| BUILD_TESTS | Build the tests | ON/OFF |
| CHECK_CSTYLE | Check code style of C sources | ON/OFF |
| CMAKE_BUILD_TYPE | Choose the type of build | None/Debug/Release/RelWithDebInfo |
| CMAKE_INSTALL_PREFIX | Install path prefix, prepended onto install directories | /usr/local |
| COVERAGE | Run coverage test | ON/OFF |
| DEVELOPER_MODE | Enable developer checks | ON/OFF |
| GROFF | Path to a program | /usr/bin/groff |
| TESTS_LONG | Enable long running tests | ON/OFF |
| TESTS_USE_FAULT_INJECTION | Run tests with fault injection | ON/OFF |
| TESTS_USE_FORCED_PMEM | Run tests with PMEM_IS_PMEM_FORCE=1 | ON/OFF |
| TESTS_USE_VALGRIND | Enable tests with valgrind | ON/OFF |
| TEST_DIR | Working directory for tests | /rpma/build/test |
| TRACE_TESTS | More verbose test outputs | ON/OFF |
| TXT2MAN | Path to a program | /usr/local/bin/txt2man |
| USE_ASAN | Enable AddressSanitizer | ON/OFF |
| USE_UBSAN | Enable UndefinedBehaviorSanitizer | ON/OFF |

### Configuring CMake options

CMake options can be changed with ```-D``` option e.g.:

```shell
$ cmake -DBUILD_DOC=ON -DTEST_LONG=ON -DTEST_DIR=/rpma/build/test ..
```

You can browse and edit CMake options using ```cmake-gui``` or ```ccmake``` e.g.:

```shell
$ ccmake .
```

## Installing

When you want to install librpma after compiling it:

```shell
$ sudo make install
```

## Running

The librpma library can be found in the `build/src` directory.
Additionally, you can run the binaries in `build/examples/*` which are examples of using the library.

## Testing

As mentioned above you can turn on the unit tests and make it possible to easily
execute them:

```shell
$ cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON ..
$ make test
$ ctest
```
