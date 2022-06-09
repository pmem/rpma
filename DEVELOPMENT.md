# DEVELOPMENT ENVIRONMENT SETTINGS

XXX

# Testing

This section describes how to prepare the environment for execution of all available kinds of tests:
- unit tests,
- multi-threaded (MT) tests and
- integration tests.

## Run unit tests

One of the basic principles of delivering a new functionality to a library is to deliver it simultaneously with the unit tests to ensure that the new features behave correctly.
You have to expect that code that degrades the test coverage will not pass the review process unless it is explicitly marked as a draft.

The unit tests are implemented using the [cmocka](https://cmocka.org/) framework.

All unit tests are located in the `./tests/unit/` directory.

To run unit tests you need to issue the following commands:

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

## Run integration tests on SoftRoCE

XXX

## Run integration tests on RDMA HW

XXX
