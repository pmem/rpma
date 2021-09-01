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

To use the reporting tools (e.g. `report_bench.py`, `report_figures.py`, `report_create.py`), you must additionally install:
 - jinja2
 - markdown2
 - PIL

```sh
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

## Reporting

Instead of running all separate workloads you can run a comprehensive set of workloads and generate the RPMA performance report.

To generate a report, follow these steps:

### 1) Prepare a config.json file

Make a copy of config.json.example (config.json) and adjust it to describe your configuration. For details on all available options please see [CONFIG.JSON.md](CONFIG.JSON.md).

### 2) Run benchmarks

You can choose from few predefined sets of benchmarks covering different aspects. The predefined benchmarks are stored in the `./figures/*.json` files. You can run one or more of them at once e.g.:

```sh
$ ./report_bench.py run --config config.json --figures figures/read.json figures/write.json --result_dir results
```

To see all available configuration options please take a look at the help:

```sh
$ ./report_bench.py -h
```

### 3) Generate figures

```sh
$ ./report_figures.py --bench results/bench.json
```

To see all available configuration options please take a look at the help:

```sh
$ ./report_figures.py -h
```

### 4) Generate the performance report

Make a copy of report.json.example (report.json) and adjust it to describe your configuration.

```sh
$ ./report_create.py --bench results/bench.json --report report.json
```

To see all available configuration options please take a look at the help:

```sh
$ ./report_create.py -h
```
