# BENCHMARKING

This document describes how to automate collection, processing and presentation of performance numbers (latency and bandwidth) from the RPMA-dedicated FIO engine.

As a baseline the generally accepted tools like `ib_read_lat`, `ib_read_bw` etc. are used which execution, processing and presentation are also automated.

## Requirements

To use the benchmarking tools (e.g. `ib_read.sh`, `rpma_fio_bench.sh`), you must have several components installed in your system:
 - python3
 - python3-pip
 - pandas
 - matplotlib
 - perftest (providing ib tools like `ib_read_lat`, `ib_read_bw` etc)
 - fio >= 3.27
 - numactl
 - sshpass
 - pciutils (for ddio.sh which may be invoked from `rpma_fio_bench.sh`)

*Note*: The newest features are available on the development branch: https://github.com/pmem/fio.git

```sh
$ sudo yum install python3 python3-pip numactl sshpass
$ pip3 install --user pandas
$ pip3 install --user matplotlib

$ sudo yum install perftest
or
$ git clone https://github.com/linux-rdma/perftest.git
$ cd perftest
$ ./autogen.sh
$ ./configure
$ make
$ sudo make install

$ git clone https://github.com/axboe/fio.git
$ cd fio
$ ./configure
$ make
$ sudo make install
```

To use the reporting tools (e.g. `csv_compare.py`, `report_bench.py`, `report_figures.py`, `report_create.py`), you must additionally install:
 - jinja2
 - markdown2
 - PIL

```sh
$ sudo yum install python3 python3-pip numactl sshpass
$ pip3 install --user jinja2
$ pip3 install --user markdown2
$ pip3 install --user PIL
```

*Note*: All of the scripts presented in the following sections must be run on the client side.

## Running workloads

There are a few tools that can be used for automatic running RPMA-related workloads:

- `ib_read.sh` - a tool using `ib_read_lat` and `ib_read_bw` to benchmark the baseline performance of RDMA read operation
- `rpma_fio_bench.sh` - a tool using librpma-dedicated FIO engines for benchmarking remote memory manipulation (reading, writing APM-style, writing GPSPM-style, mixed). These workloads can be run against PMem and DRAM as well.

### Example of `ib_read.sh` use

Generate the baseline latency numbers using `./ib_read.sh` tool:

```sh
$ export JOB_NUMA=0
$ export REMOTE_USER=user
$ export REMOTE_PASS=pass
$ export REMOTE_JOB_NUMA=0

$ ./ib_read.sh $SERVER_IP lat
```

To see all available configuration options please take a look at the help:

```sh
$ ./ib_read.sh
```

### Example of `rpma_fio_bench.sh` use

Generate latency numbers from the RPMA-dedicated FIO engine using `./rpma_fio_bench.sh`:

```sh
$ export JOB_NUMA=0
$ export REMOTE_USER=user
$ export REMOTE_PASS=pass
$ export REMOTE_JOB_NUMA=0
$ export REMOTE_JOB_MEM_PATH=/dev/dax1.0

$ ./rpma_fio_bench.sh $SERVER_IP apm read lat
```

To see all available configuration options please take a look at the help:

```sh
$ ./rpma_fio_bench.sh
```

## Analyzing the results

All of the benchmarking tools described above generate standardized CSV format output files, which can be further processed using `csv_compare.py` to generate comparative charts.

### Example of comparing the obtained results

To generate a chart comparing the obtained results you can feed them into the script as follows:

```sh
$ ./csv_compare.py --output_layout lat_avg ib_read_lat-21-01-31-072033.csv rpma_fio_apm_read_lat_th1_dp1_dev_dax1.0-21-01-31-073733.csv --output_with_tables
```

With the help of additional parameters, you can adjust various aspects of the output.

## Reporting

Instead of running all separate workloads you can run a comprehensive set of workloads and generate the RPMA performance report.

To generate a report, follow these steps:

### 1) Run all benchmarks required for the performance report:

Prepare config.json based file on config.json.example

```sh
$ ./report_bench.py run --config config.json --figure figures/read.json --result_dir results
```

To see all available configuration options please take a look at the help:

```sh
$ ./report_bench.py -h
```

### 2) Generate Figures charts for the performance report:

```sh
$ ./report_figures.py --bench results/bench.json
```

To see all available configuration options please take a look at the help:

```sh
$ ./report_figures.py -h
```

### 3) Generate the performance report:

Prepare report.json based file on report.json.example

```sh
$ ./report_create.py --bench results/bench.json --report report.json
```

To see all available configuration options please take a look at the help:

```sh
$ ./report_create.py -h
```
