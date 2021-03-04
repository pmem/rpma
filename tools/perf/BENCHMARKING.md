# BENCHMARKING

This document describes how to automate collection, processing and presentation of performance numbers (latency and bandwidth) from the RPMA-dedicated FIO engine.

As a baseline the generally accepted tools like `ib\_read\_lat`, `ib\_read\_bw` etc. are used which execution, processing and presentation are also automated.

## Requirements

To use the benchmarking tools (e.g. ib\_read.sh, rpma\_fio\_bench.sh), you must have several components installed in your system:
 - python3
 - python3-pip
 - pandas
 - matplotlib
 - perftest (providing ib tools like `ib\_read\_lat`, `ib\_read\_bw` etc)
 - fio (supporting the librpma fio engine)
 - numactl
 - sshpass
 - pciutils (for ddio.sh which may be invoked from rpma\_fio\_bench.sh)

*Note*: Currently only https://github.com/pmem/fio.git supports the librpma fio engine.

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

$ git clone https://github.com/pmem/fio.git
$ cd fio
$ ./configure
$ make
$ sudo make install
```

To use the reporting tools (e.g. csv\_compare.py, report\_bench.sh, create\_report\_figures.sh, create\_report.py, fio-histogram.py), you must additionally install:
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

- `ib\_read.sh` - a tool using `ib\_read\_lat` and `ib\_read\_bw` to benchmark the baseline performance of RDMA read operation
- `rpma\_fio\_bench.sh` - a tool using librpma-dedicated FIO engines for benchmarking remote memory manipulation (reading, writing APM-style, writing GPSPM-style, mixed). These workloads can be run against PMem and DRAM as well.

### Example of `ib\_read.sh` use

Generate the baseline latency numbers using `./ib\_read.sh` tool:

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

### Example of `rpma\_fio\_bench.sh` use

Generate latency numbers from the RPMA-dedicated FIO engine using `./rpma\_fio\_bench.sh`:

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

All of the benchmarking tools described above generate standardized CSV format output files, which can be further processed using `csv\_compare.py` to generate comparative charts.

### Example of comparing the obtained results

To generate a chart comparing the obtained results you can feed them into the script as follows:

```sh
$ ./csv_compare.py --output_layout lat_avg ib_read_lat-21-01-31-072033.csv rpma_fio_apm_read_lat_th1_dp1_dev_dax1.0-21-01-31-073733.csv --output_with_tables
```

With the help of additional parameters, you can adjust various aspects of the output.

## Reporting

### To generate a report, follow these steps:

#### 1) Run all benchmarks required for the performance report:

```sh
$ export REMOTE_SUDO_NOPASSWD=1
$ export REMOTE_RNIC_PCIE_ROOT_PORT=$pcie_root_port
$ export REMOTE_JOB_MEM_PATH=/dev/dax1.0

$ ./reprt_bench.sh $SERVER_IP
```

#### 2) Generate Figures and Appendix charts for the performance report:

```sh
$ mkdir -p results/MACHINE_A
$ mv *.csv results/MACHINE_A
$ export DATA_PATH=/results/MACHINE_A

$ export READ_LAT_MACHINE=A
$ export READ_BW_MACHINE=A
$ export WRITE_LAT_MACHINE=A
$ export WRITE_BW_MACHINE=A
$ export MIX_BW_MACHINE=A
$ export MIX_LAT_MACHINE=A

$ export STAMP=xyz

# charts will be produced to the report_xyz directory
$ ./create_report_figures.sh report
$ ./create_report_figures.sh appendix
```

#### 3) Prepare custom report parts (optional):

```sh
$ cp templates/*.copyme report_xyz/
$ for f in report_xyz/*.copyme; do mv "$f" "${f/.copyme/}"; done
# edit the report_xyz/*.md files to represent details of your configuration
# prepare a view of the configuration high-level RPMA performance test (setup.png)
```

#### 4) Generate the performance report and appendices:

```sh
# will create a report_xyz/report.html report_xyz/appendices.html
$ ./create_report.py --report_dir report_xyz --release X.YZ report --test_date "Now" --high_level_setup_figure ./setup.png
$ ./create_report.py --report_dir report_xyz --release X.YZ appendices
```
