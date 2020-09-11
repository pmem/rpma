# How to build the librpma library from source

## Requirements

### Common requirements

In order to build librpma, you need to have installed several components:

- C compiler
- [CMake](http://www.cmake.org) >= 3.3
- libibverbs-dev(el)
- librdmacm-dev(el)
- groff
- libcmocka-dev(el) == 1.1.5-26-g672c5ce (please see https://github.com/pmem/rpma/blob/master/utils/docker/images/install-cmocka.sh to install the verified revision)
- txt2man == 1.7.0 (please see https://github.com/pmem/rpma/blob/master/utils/docker/images/install-txt2man.sh to install the verified revision)

### For some examples you also need:

- libpmem-dev(el) >= 1.6
- libprotobuf-c-dev(el) >= 1.0

**Note**: the above revisions are proven to work correctly.

## Building

First, you have to create a `build` directory.
From there you have to prepare the compilation using CMake.
The final build step is just a `make` command.

```shell
$ mkdir build && cd build
$ cmake ..
$ make -j
```

### CMake standard options

Here is a list of the most interesting options
provided out of the box by CMake.

| Name | Description | Values | Default |
| - | - | - | - |
| BUILD_DOC | Build the documentation | ON/OFF | ON |
| BUILD_EXAMPLES | Build the examples | ON/OFF | ON |
| BUILD_TESTS | Build the tests | ON/OFF | ON |
| CHECK_CSTYLE | Check code style of C sources | ON/OFF | OFF |
| CMAKE_BUILD_TYPE | Choose the type of build | None/Debug/Release/RelWithDebInfo | Debug |
| CMAKE_INSTALL_PREFIX | Install path prefix, prepended onto install directories | *dir path* | /usr/local |
| COVERAGE | Run coverage test | ON/OFF | OFF |
| DEVELOPER_MODE | Enable developer checks | ON/OFF | OFF |
| TESTS_LONG | Enable long running tests | ON/OFF | OFF |
| TESTS_USE_FAULT_INJECTION | Run tests with fault injection | ON/OFF | OFF |
| TESTS_USE_FORCED_PMEM | Run tests with PMEM_IS_PMEM_FORCE=1 | ON/OFF | OFF |
| TESTS_USE_VALGRIND | Enable tests with valgrind | ON/OFF | ON |
| TEST_DIR | Working directory for tests | *dir path* | ./build/test |
| TRACE_TESTS | More verbose test outputs | ON/OFF | OFF |
| USE_ASAN | Enable AddressSanitizer | ON/OFF | OFF |
| USE_UBSAN | Enable UndefinedBehaviorSanitizer | ON/OFF | OFF |

### Configuring CMake options

CMake options can be changed with `-D` option e.g.:

```shell
$ cmake -DBUILD_DOC=ON -DTEST_LONG=ON -DTEST_DIR=/rpma/build/test ..
```

You can browse and edit CMake options using `cmake-gui` or `ccmake` e.g.:

```shell
$ ccmake .
```

### Building packages

In order to build 'rpm' or 'deb' packages you should issue the following commands:

```shell
cmake .. -DCPACK_GENERATOR="$GEN" -DCMAKE_INSTALL_PREFIX=/usr
make package
```

where $GEN is a type of package generator: RPM or DEB.

CMAKE_INSTALL_PREFIX must be set to a destination were packages will be installed

## Installing

After compiling the library, you can install it:

```shell
$ sudo make install
```

## Running

The librpma library can be found in the `build/src` directory.
After the compilation, the librpma library binaries can be found in the `build/src` directory.
Additionally, you can run the examples in `build/examples/*`.

## Testing

As mentioned above you can turn on building the unit tests and run them easily:

```shell
$ cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON ..
$ make test
$ ctest
```
