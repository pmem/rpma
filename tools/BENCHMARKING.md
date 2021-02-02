# BENCHMARKING

This document describes how to automate collection, processing and presentation of performance numbers (latency and bandwidth) from the RPMA-dedicated FIO engine.

As a baseline the generally accepted tools like `ib_read_lat`, `ib_read_bw` etc. are used which execution, processing and presentation are also automated.

## Requirements

To use these tools, you must have several components installed in your system:
 - python3
 - python3-pip
 - pandas
 - matplotlib
 - perftest (providing ib tools like `ib_read_lat`, `ib_read_bw` etc)
 - fio (supporting the librpma fio engine)
 - numactl
 - sshpass

*Note*: Currently only https://github.com/pmem/fio.git supports the librpma fio engine.

```sh
$ sudo yum install python3 python3-pip numactl sshpass
$ pip3 install --user pandas
$ pip3 install --user matplotlib

$ git clone https://github.com/linux-rdma/perftest.git
$ cd perftest
$ ./autogen.sh
$ ./configure
$ make
$ sudo make install
or
$ sudo yum install perftest

$ git clone https://github.com/pmem/fio.git
$ cd fio
$ ./configure
$ make
$ sudo make install
```

*Note*: All of the scripts presented in the following sections must be run on the client side.

## Running workloads

You can choose from few tools with automate running RPMA related workloads:

### Example of `ib_read.sh` use

Generate the baseline latency or bandwidth numbers using `./ib_read.sh` tool:

```sh
$ export JOB_NUMA=0

# -d <IB device> -R (enable rdma_cm QP)
$ export AUX_PARAMS='-d mlx5_0 -R'
$ export REMOTE_USER=user
$ export REMOTE_PASS=pass
$ export REMOTE_JOB_NUMA=0
$ export REMOTE_AUX_PARAMS='-d mlx5_0 -R'

$ ./ib_read.sh <SERVER_IP> lat
```

To see all available configuration workloads please take a look at the help:

```sh
$ ./ib_read.sh -h
```

### Example of `rpma_fio_bench.sh` use

Generate latency or bandwidth numbers from the RPMA-dedicated FIO engine using `./rpma_fio_bench.sh`:

```sh
$ export JOB_NUMA=0
$ export FIO_PATH=/custom/fio/path
$ export REMOTE_USER=user
$ export REMOTE_PASS=pass
$ export REMOTE_JOB_NUMA=0
$ export REMOTE_SUDO_NOPASSWD=0/1
$ export REMOTE_RNIC_PCIE_ROOT_PORT=<pcie_root_port>
$ export REMOTE_DIRECT_WRITE_TO_PMEM=0/1 (https://pmem.io/rpma/documentation/basic-direct-write-to-pmem.html)
$ export FORCE_REMOTE_DIRECT_WRITE_TO_PMEM=0/1 (forces setting REMOTE_DIRECT_WRITE_TO_PMEM to this value)
$ export REMOTE_FIO_PATH=/custom/fio/path

# optional, by default: /dev/shm/librpma-server-${TIMESTAMP}.fio
$ export REMOTE_JOB_PATH=/custom/jobs/path

# optional, by default: malloc
$ export REMOTE_JOB_MEM_PATH=/path/to/mem

$ ./rpma_fio_bench.sh <SERVER_IP> apm read lat
```

To see all available configuration workloads please take a look at the help:

```sh
$ ./rpma_fio_bench.sh -h
```

## Analyzing the results

All of the benchmarking tools described above generates a standardized CSV format output files. Which can be further processed using `csv_compare.py` to generate a comparative charts.

### Example of comparing the obtained results

```sh
$ ./csv_compare.py --output_layout [lat/bw] file-1.csv [... file-n.csv]
# generates a png chart using python + pandas + matplotlib
```

With the help of additional parameters, we can also adjust various aspects of the output.

## Reporting

Instead of running all separate workloads you can run a comprehensive set of workloads using the following set of commands and generate the RPMA performance report.

Run all benchmarks required for the performance report:

```sh
$ export REMOTE_SUDO_NOPASSWD=1
$ export REMOTE_RNIC_PCIE_ROOT_PORT=<pcie_root_port>
$ export REMOTE_JOB_MEM_PATH=/path/to/mem

$ ./reprt_bench.sh
```

Generate Figures and Appendix charts for a performance report:

```sh
$ export DATA_PATH=/custom/data/path

$ ./create_report_figures.sh report|appendix
```

Generate the performance report and appendices:

```sh
$ ./create_report.py
```
