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

**Note**: To make sure you have all needed packages installed you can support yourself with [Dockerfiles](./utils/docker/images/) (see the BASE_DEPS and RPMA_DEPS sections)

### In order to build the documentation you also need:

- diff
- find
- groff
- txt2man == 1.7.0 (please see [our docker script](./utils/docker/images/install-txt2man.sh) to install the verified revision)

and optionally:

- pandoc

to generate the Markdown documentation.

**Note**: building the documentation can be turned off using the CMake `BUILD_DOC` option
(see [Configuring CMake options](DEVELOPMENT.md#configuring-cmake-options) and
[CMake options of the librpma library](DEVELOPMENT.md#cmake-options-of-the-librpma-library)).

### For some examples you also need:

- libpmem-dev(el) >= 1.6 or libpmem2-dev(el) >= 1.11 for examples: 3, 4, 5, 7, 9, 9s
- libprotobuf-c-dev(el) >= 1.0 for examples: 9, 9s

**Note**: the above versions of libraries are proven to work correctly.

**Note**: see [the list of the supported OSes](INSTALL.md#os-support).

**Note**: please be aware that the libpmem2-dev(el) package is not available on some distributions. Use [our script](./utils/docker/images/install-pmdk.sh) to install it manually from sources. You can check all needed additional packages in one of our [Dockerfiles](./utils/docker/images/), for example [here](./utils/docker/images/Dockerfile.archlinux-latest), in the variable called PMDK_DEPS.

**Note**: libprotobuf-c-dev(el) is needed to run examples: 9 and 9s

**Note**: Examples that use PMem (3, 4, 5, 7, 9, 9s) require only one of the following libraries to be run: libpmem or libpmem2. In case of having installed both of them libpmem2 will be used.

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
