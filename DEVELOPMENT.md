# DEVELOPMENT ENVIRONMENT SETTINGS

## Configuring CMake options

CMake creates many cache files and subdirectories in the directory where it is run, so it is recommended to run all CMake-related commands (like `cmake` and `ccmake`)  in a separate newly created subdirectory usually called `build`:

```sh
[rpma]$ mkdir build
[rpma]$ cd build
[rpma/build]$ cmake ..
```

All examples listed below use the `build` subdirectory as the CMake build directory.

CMake options can be changed using the `-D` option e.g.:

```sh
[rpma/build]$ cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_DEVELOPER_MODE=ON ..
```

You can browse and edit the CMake options using `cmake-gui` or `ccmake` e.g.:

```sh
[rpma/build]$ ccmake ..
```

## CMake options of the librpma library

Here is a list of the most interesting CMake options of the librpma library:

| Name | Description | Values | Default |
| - | - | - | - |
| BUILD_DOC | Build the documentation | ON/OFF | ON |
| BUILD_TESTS | Build the tests | ON/OFF | ON |
| BUILD_EXAMPLES | Build the examples | ON/OFF | ON |
| BUILD_DEVELOPER_MODE | Enable developer checks | ON/OFF | OFF |
| TESTS_COVERAGE | Run coverage test | ON/OFF | OFF |
| TESTS_USE_FORCED_PMEM | Run tests with PMEM_IS_PMEM_FORCE=1 | ON/OFF | OFF |
| TESTS_USE_VALGRIND_PMEMCHECK | Enable tests with valgrind pmemcheck (if found)| ON/OFF | OFF |
| TESTS_PERF_TOOLS | Enable testing Python tools | ON/OFF | OFF |
| TESTS_RDMA_CONNECTION | Enable tests that require a configured RDMA-capable network interface (valgrind required) | ON/OFF | OFF |
| TESTS_VERBOSE_OUTPUT | More verbose test outputs | ON/OFF | OFF |
| DEBUG_LOG_TRACE | Enable logging functions' traces | ON/OFF | OFF |
| DEBUG_FAULT_INJECTION | Enable fault injection | ON/OFF | OFF |
| DEBUG_USE_ASAN | Enable AddressSanitizer | ON/OFF | OFF |
| DEBUG_USE_UBSAN | Enable UndefinedBehaviorSanitizer | ON/OFF | OFF |
| CMAKE_BUILD_TYPE | Choose the type of build | None/Debug/Release/RelWithDebInfo | Release |
| CMAKE_INSTALL_PREFIX | Install path prefix, prepended onto install directories | *dir path* | /usr/local |
| TEST_DIR | Working directory for tests | *dir path* | ./build/test |

The following command can be used to see all available CMake options:

```sh
[rpma/build]$ cmake -LAH ..
```

# Testing

This section describes how to prepare the environment for execution of all available kinds of tests:
- unit tests,
- multi-threaded (MT) tests and
- integration tests.

## Running only the unit tests

The unit tests are implemented using the [cmocka](https://cmocka.org/) framework. They do not need any RDMA-capable network interface. All unit tests are located in the `./tests/unit/` subfolder of the main directory.

In order to run **only** the unit tests (this is the default configuration):
1. Build the librpma library with the `TESTS_RDMA_CONNECTION` CMake variable set to `OFF` (it is good to set also the `CMAKE_BUILD_TYPE` CMake variable to `Debug` to be able to see the debug information in case of failures):

```sh
[rpma]$ cd build
[rpma/build]$ cmake -DCMAKE_BUILD_TYPE=Debug -DTESTS_RDMA_CONNECTION=OFF ..
[rpma/build]$ make -j$(nproc)
```

or just:

```sh
[rpma]$ cd build; rm -rf ./*
[rpma/build]$ cmake -DCMAKE_BUILD_TYPE=Debug ..
[rpma/build]$ make -j$(nproc)
```

in the empty `build` subfolder.

2. Run tests from the `build` subdirectory:

```sh
[rpma/build]$ make test
```

or:

```sh
[rpma/build]$ ctest --output-on-failure
```

to print out also the output of the failed tests.

## Running multi-threaded or integration tests on SoftRoCE or RDMA HW

### Preparing the environment

In order to run the multi-threaded or the integration tests:
1. Make sure you have all needed packages installed (they are listed in [Dockerfiles](./utils/docker/images/) (see the EXAMPLES_DEPS section)
2. Valgrind must be installed to run both the multi-threaded and the integration tests.
3. A correctly configured RDMA-capable network interface (SoftRoCE or RDMA HW) with an IP address assigned is required.
4. If SoftRoCE is to be used to run tests, it can be configured in the following two alternative ways (it also prints out the IP of the configured interface):

```sh
[rpma]$ ./tools/config_softroce.sh
```

or:

```sh
[rpma/build]$ cmake ..
[rpma/build]$ make config_softroce
```

5. Set the `RPMA_TESTING_IP` environment variable to an IP address of this interface:

```sh
$ export RPMA_TESTING_IP=192.168.0.1 # insert your own IP address here
```

### Building the librpma library for running multi-threaded or integration tests

1. In order to run the **multi-threaded tests** build the librpma library with the `TESTS_RDMA_CONNECTION` CMake variable set to `ON` (it is good to set also the `CMAKE_BUILD_TYPE` CMake variable to `Debug` to be able to see the debug information in case of failures):

```sh
[rpma]$ cd build
[rpma/build]$ cmake -DCMAKE_BUILD_TYPE=Debug -DTESTS_RDMA_CONNECTION=ON ..
[rpma/build]$ make -j$(nproc)
```

2. In order to run the **integration tests** build the librpma library with the `TESTS_RDMA_CONNECTION` and the `DEBUG_FAULT_INJECTION` CMake variables set to `ON` (it is good to set also the `CMAKE_BUILD_TYPE` CMake variable to `Debug` to be able to see the debug information in case of failures):

```sh
[rpma]$ cd build
[rpma/build]$ cmake -DCMAKE_BUILD_TYPE=Debug -DTESTS_RDMA_CONNECTION=ON -DDEBUG_FAULT_INJECTION=ON ..
[rpma/build]$ make -j$(nproc)
```

### Running unit and multi-threaded tests

In order to run both: unit and multi-threaded tests, run the following command:

```sh
[rpma/build]$ make test
```

or:

```sh
[rpma/build]$ ctest --output-on-failure
```

to print out also the output of the failed tests.

### Running only multi-threaded tests

In order to run **only** the multi-threaded tests, run the following command:

```sh
[rpma/build]$ ctest -R multithreaded --output-on-failure
```

### Running integration tests

The integration tests are implemented as examples run together with the fault injection mechanism. 

The integration tests can be started using one of the following commands:
1. From the build directory:

```sh
[rpma/build]$ make run_all_examples_with_fault_injection
```

or:

2. From the main directory of the librpma repository:

```sh
[rpma]$ ./examples/run-all-examples.sh ./build/examples/ --integration-tests
```

In order to run the integration tests on a PMem (a DAX device or a file on a file system DAX), an absolute path (starting with `/`) to this PMem has to be provided either via the `<pmem-path>` argument:

```sh
[rpma]$ ./examples/run-all-examples.sh ./build/examples/ --integration-tests <pmem-path>
```

or via the `RPMA_EXAMPLES_PMEM_PATH` environment variable. If both of them are set, the command line argument `<pmem-path>` will be used.

By default the integration tests do not stop on a failure. In order to stop on the first failure, the `RPMA_EXAMPLES_STOP_ON_FAILURE` environment variable has to be set to `ON` or the following command has to be run:

```sh
[rpma]$ ./examples/run-all-examples.sh ./build/examples/ --integration-tests --stop-on-failure
```

To see all available configuration options please take a look at the help message printed out by the following command:

```sh
[rpma]$ ./examples/run-all-examples.sh
```
