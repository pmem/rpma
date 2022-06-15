# DEVELOPMENT ENVIRONMENT SETTINGS

## CMake standard options

Here is a list of the most interesting options provided out of the box by CMake:

| Name | Description | Values | Default |
| - | - | - | - |
| BUILD_DOC | Build the documentation | ON/OFF | ON |
| BUILD_TESTS | Build the tests | ON/OFF | ON |
| BUILD_EXAMPLES | Build the examples | ON/OFF | OFF |
| BUILD_CHECK_CSTYLE | Check code style of C sources | ON/OFF | OFF |
| BUILD_DEVELOPER_MODE | Enable developer checks | ON/OFF | OFF |
| TESTS_COVERAGE | Run coverage test | ON/OFF | OFF |
| TESTS_USE_FORCED_PMEM | Run tests with PMEM_IS_PMEM_FORCE=1 | ON/OFF | OFF |
| TESTS_USE_VALGRIND | Enable tests with valgrind | ON/OFF | ON |
| TESTS_PERF_TOOLS | Enable testing Python tools | ON/OFF | OFF |
| TESTS_VERBOSE_OUTPUT | More verbose test outputs | ON/OFF | OFF |
| DEBUG_LOG_TRACE | Enable logging functions' traces | ON/OFF | OFF |
| DEBUG_FAULT_INJECTION | Enable fault injection | ON/OFF | OFF |
| DEBUG_USE_ASAN | Enable AddressSanitizer | ON/OFF | OFF |
| DEBUG_USE_UBSAN | Enable UndefinedBehaviorSanitizer | ON/OFF | OFF |
| CMAKE_BUILD_TYPE | Choose the type of build | None/Debug/Release/RelWithDebInfo | Release |
| CMAKE_INSTALL_PREFIX | Install path prefix, prepended onto install directories | *dir path* | /usr/local |
| TEST_DIR | Working directory for tests | *dir path* | ./build/test |

You can use the following command to see all available options:

```sh
$ cmake -LAH ..
```

## Configuring CMake options

CMake creates many cache files and subdirectories in the directory where it is run, so it is recommended to run all CMake-related commands (like `cmake` and `ccmake`)  in a separate newly created subdirectory usually called `build`:

```sh
$ mkdir build
$ cd build
$ cmake ..
```

CMake options can be changed with `-D` option e.g.:

```sh
$ cmake -DBUILD_DOC=ON -DTEST_LONG=ON -DTEST_DIR=/rpma/build/test ..
```

You can browse and edit CMake options using `cmake-gui` or `ccmake` e.g.:

```sh
$ ccmake ..
```

# Testing

This section describes how to prepare the environment for execution of all available kinds of tests:
- unit tests,
- multi-threaded (MT) tests and
- integration tests.

## Run unit tests

The unit tests are implemented using the [cmocka](https://cmocka.org/) framework.

All unit tests are located in the `./tests/unit/` subfolder of the main directory.

To run unit tests you need to issue the following commands in the main directory of the librpma repository:

```sh
$ cd build
$ cmake ..
$ make -j$(nproc)
$ make test
```
The `ctest` command can be used instead of `make test`.

## Run multi-threaded tests

To run all multi-threaded tests a configured RDMA-capable network interface is needed.

### Run multi-threaded tests on SoftRoCE:
- configure SoftRoCE using the `tools/config_softroce.sh` script or `make config_softroce`
- set the `RPMA_TESTING_IP` environment variable:
```sh
$ export RPMA_TESTING_IP=192.168.0.1 # insert your own IP address here
```
- turn on building the multi-threaded tests with valgrind support enabled and run them:
```sh
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug -DTESTS_RDMA_CONNECTION=ON -DTESTS_USE_VALGRIND=ON ..
$ make -j$(nproc)
$ make test
```
The `ctest` command can be used instead of `make test`.

### Run multi-threaded tests on RDMA HW:
- configure RDMA HW
- set the `RPMA_TESTING_IP` environment variable:
```sh
$ export RPMA_TESTING_IP=192.168.0.1 # insert your own IP address here
```
- turn on building the multi-threaded tests with valgrind support enabled and run them:
```sh
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug -DTESTS_RDMA_CONNECTION=ON -DTESTS_USE_VALGRIND=ON ..
$ make -j$(nproc)
$ make test
```
`ctest` command can be used instead of `make test`

## Run integration tests on SoftRoCE/RDMA HW

The integration tests are implemented as examples run together with the fault injection mechanism.

In order to run the integration tests on SoftRoCE/RDMA HW, the `RPMA_TESTING_IP` environment
variable has to be set to an IP address of a configured RDMA-capable network interface
and then they can be started using one of the following commands:

```sh
$ make run_all_examples_with_fault_injection
```

from the build directory or

```sh
$ ./examples/run-all-examples.sh ./build/examples/ --fault-injection
```

from the main directory of the librpma repository.

In order to run the examples on a PMem (a DAX device or a file on a file system DAX),
an absolute path (starting with `/`) to this PMem has to be provided
either via the `<pmem-path>` argument:

```sh
$ ./examples/run-all-examples.sh ./build/examples/ --fault-injection <pmem-path>
```

or via the `RPMA_EXAMPLES_PMEM_PATH` environment variable. If both of them are set,
the command line argument `<pmem-path>` will be used.

By default the integration tests do not stop on a failure. In order to stop on the first failure,
the `RPMA_EXAMPLES_STOP_ON_FAILURE` environment variable has to be set to `ON`
or the following command has to be run:

```sh
$ ./examples/run-all-examples.sh ./build/examples/ --fault-injection --stop-on-failure
```

from the main directory of the librpma repository.
