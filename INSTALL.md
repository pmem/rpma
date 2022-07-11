# How to build the librpma library from source

## Requirements

### Common requirements
**Note**: To make sure you have all the components you need, you can use our [docker scripts](./utils/docker/images/)

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

**Note**: testing the Python tools can be turned off using the CMake 'TESTS_PERF_TOOLS' option
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
The final build step is just the `make` command:

```sh
[rpma]$ mkdir build
[rpma]$ cd build
[rpma/build]$ cmake ..
[rpma/build]$ make -j
```

When the librpma library has been successfully built, the binaries of the library can be found in the `build/src` directory.
Additionally, you can run the examples in `build/examples/*`.

To check how to set environment settings please checkout [DEVELOPMENT.md file](DEVELOPMENT.md).

### Building packages

In order to build 'rpm' or 'deb' packages you should issue the following commands:

```sh
[rpma/build]$ cmake .. -DCPACK_GENERATOR="$GEN" -DCMAKE_INSTALL_PREFIX=/usr
[rpma/build]$ make package
```

where $GEN is a type of package generator: RPM or DEB.

CMAKE_INSTALL_PREFIX must be set to a destination were packages will be installed

It is recommended to run all unit test before the installation. In order to do it you can issue the following command.
```sh
[rpma/build]$ make test
```
The `ctest` command can be used instead of `make test`.

## Installing

After compiling the library, you can install it:

```sh
[rpma/build]$ sudo make install
```

## Testing

In order to learn how to run all tests and set environment settings, please see [Testing](DEVELOPMENT.md#Testing) section in [DEVELOPMENT.md](DEVELOPMENT.md) file.

## OS support

Supported Linux distributions include:

- CentOS 7
- Debian >= 10
- Fedora >= 32
- Ubuntu >= 20.04
