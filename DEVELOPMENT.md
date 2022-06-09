# DEVELOPMENT ENVIRONMENT SETTINGS

XXX

# Testing

This section describes how to prepare the environment for execution of all available kinds of tests:
- unit tests,
- multi-threaded (MT) tests and
- integration tests.

## Run unit tests

You can turn on building the unit tests and run them easily:

```shell
$ cd build
$ rm -rf
$ cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON ..
$ make
$ make test
```
``ctest`` command can be used instead of ``make test``

## Run multi-threaded tests

To run multithreaded tests we need configured RDMA-capable network interface.

### Run multi-threaded tests on SoftRoCE:
- configure SoftRoCE with tools/config_softroce.sh script
- set the environment variable RPMA_TESTING_IP
```shell
$ export RPMA_TESTING_IP=127.0.0.0
```
- turn on building the multi-threaded tests and run them
```shell
$ cd build
$ rm -rf
$ cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -DTESTS_RDMA_CONNECTION=ON ..
$ make
$ make test
```
``ctest`` command can be used instead of ``make test``

### Run multi-threaded tests on RDMA HW:
- configure RDMA HW
- set the environment variable RPMA_TESTING_IP
```shell
$ export RPMA_TESTING_IP=127.0.0.0
```
- turn on building the multi-threaded tests and run them
```shell
$ cd build
$ rm -rf
$ cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -DTESTS_RDMA_CONNECTION=ON ..
$ make
$ make test
```
``ctest`` command can be used instead of ``make test``

## Run integration tests on SoftRoCE

XXX

## Run integration tests on RDMA HW

XXX
