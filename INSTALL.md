# How to build from source

## Requirements

### Common requirements

In order to build librpma, you need to install several components:

- A C compiler
- [CMake](http://www.cmake.org) >= 3.3
- libibverbs-dev
- librdmacm-dev
- libuwind-dev
- groff
- cmocka -> `rpma/utils/docker/images/install-cmocka.sh`
- txt2man -> `rpma/utils/docker/images/install-txt2man.sh`

For some examples:

- libpmem >= 1.6

Note that these version numbers are version we know works correctly.

## Building
First, you need to configure the compilation, using CMake. Go inside the
`build` dir. Create it if it doesn't exist.

	cmake ..
	make

### CMake standard options
Here is a list of the most interesting options provided out of the box by
CMake.

- CMAKE_BUILD_TYPE:     The type of build (can be Debug Release MinSizeRel
						RelWithDebInfo)
- CMAKE_INSTALL_PREFIX: The prefix to use when running make install (Default
						to /usr/local)


### CMake options defined for librpma

Options are defined in the following files:

- rpma/CMake.txt

They can be changed with the -D option:

`cmake -DBUILD_DOC=ON -DTEST_LONG=ON -DTEST_DIR=/rpma/build/test ..`

### Browsing/editing CMake options

In addition to passing options on the command line, you can browse and edit
CMake options using `cmake-gui` or `ccmake`.

- Go to the build dir
- run `ccmake .`

## Installing

If you want to install librpma after compilation run:

	make install

## Running

The librpma library can be found in the `build/src` directory.
You can run the binaries in `build/examples/*` which is a
are example tests.

## Testing

As mention above you can turn on the unit tests and make it possible to easily
execute them:

`cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON ..`

After that you can simply call `make test` in the build directory or if you
want more output simply call `ctest`.
