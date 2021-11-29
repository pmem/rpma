# How to build the librpma library from source

## Requirements

### Common requirements

In order to build librpma, you need to have installed several components:

- C compiler
- [CMake](http://www.cmake.org) >= 3.3
- pkg-config
- libibverbs-dev(el)
- librdmacm-dev(el)
- libcmocka-dev(el) == 1.1.5-26-g672c5ce (please see [our docker script](./utils/docker/images/install-cmocka.sh) to install the verified revision)

### In order to build the documentation you also need:

- diff
- find
- groff
- txt2man == 1.7.0 (please see [our docker script](./utils/docker/images/install-txt2man.sh) to install the verified revision)

and optionally:

- pandoc

to generate the Markdown documentation.

**Note**: building the documentation can be turned off using the CMake 'BUILD_DOC' option
(see [CMake standard options](INSTALL.md#cmake-standard-options) and
[Configuring CMake options](INSTALL.md#configuring-cmake-options)).

### In order to test the Python tools you also need:

- pylint

**Note**: testing the Python tools can be turned off using the CMake 'TEST_PYTHON_TOOLS' option
(see [CMake standard options](INSTALL.md#cmake-standard-options) and
[Configuring CMake options](INSTALL.md#configuring-cmake-options)).

### For some examples you also need:

- libpmem-dev(el) >= 1.6
- libprotobuf-c-dev(el) >= 1.0

**Note**: the above revisions are proven to work correctly.

**Note**: see [the list of the supported OSes](INSTALL.md#os-support).

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
| TEST_PYTHON_TOOLS | Enable testing Python tools | ON/OFF | ON |
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

## OS support

Known supported OSes:

- CentOS >= 7
- Debian >= 9
- Fedora >= 27
- Ubuntu >= 18.04

Known unsupported OSes:

- Debian 8:     too old version of CMake (v3.0.2)
- Ubuntu 14.04: too old version of CMake (v2.8.12.2)
