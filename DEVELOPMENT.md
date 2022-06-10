# DEVELOPMENT ENVIRONMENT SETTINGS
## CMake standard options

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
## Configuring CMake options

CMake options can be changed with `-D` option e.g.:

```shell
$ cmake -DBUILD_DOC=ON -DTEST_LONG=ON -DTEST_DIR=/rpma/build/test ..
```

You can browse and edit CMake options using `cmake-gui` or `ccmake` e.g.:

```shell
$ ccmake .
```

# Testing

This section describes how to prepare the environment for execution of all available kinds of tests:
- unit tests,
- multi-threaded (MT) tests and
- integration tests.

The librpma library can be found in the `build/src` directory.
After the compilation, the librpma library binaries can be found in the `build/src` directory.
Additionally, you can run the examples in `build/examples/*`.
## Run unit and multi-threaded tests on SoftRoCE

One of the basic principles of delivering a new functionality to a library is to deliver it simultaneously with the unit tests to ensure that the new features behave correctly.
You have to expect that code that degrades the test coverage will not pass the review process unless it is explicitly marked as a draft.

The unit tests are implemented using the [cmocka](https://cmocka.org/) framework.

All unit tests are located in the `./tests/unit/` directory.

To run unit tests you need to issue the following commands:

```sh
$ cd build
$ cmake ..
$ make -j${nproc}
$ make test
```
The `ctest` command can be used instead of `make test`.

## Run multi-threaded tests

To run all multi-threaded tests a configured RDMA-capable network interface is needed.

### Run multi-threaded tests on SoftRoCE:
- configure SoftRoCE using the tools/config_softroce.sh script or `make config_softroce`
- set the `RPMA_TESTING_IP` environment variable
```sh
$ export RPMA_TESTING_IP=192.168.0.1 # insert your own IP address here
```
- turn on building the multi-threaded tests and run them
```sh
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug -DTESTS_RDMA_CONNECTION=ON -DTESTS_USE_VALGRIND=ON ..
$ make -j${nproc}
$ make test
```
The `ctest` command can be used instead of `make test`.

### Run multi-threaded tests on RDMA HW:
- configure RDMA HW
- set the `RPMA_TESTING_IP` environment variable
```sh
$ export RPMA_TESTING_IP=192.168.0.1 # insert your own IP address here
```
- turn on building the multi-threaded tests and run them
```sh
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug -DTESTS_RDMA_CONNECTION=ON -DTESTS_USE_VALGRIND=ON ..
$ make -j${nproc}
$ make test
```
`ctest` command can be used instead of `make test`

## Run integration tests on SoftRoCE

XXX

## Run integration tests on RDMA HW

XXX
## Contributing

If you want to contribute to our repository please first checkout [CONTRIBUTING.md file](CONTRIBUTING.md).
