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
$ export REMOTE_JOB_MEM_PATH=/dev/dax0.1

$ ./rpma_fio_bench.sh $SERVER_IP apm read lat
```

To see all available configuration options please take a look at the help:

```sh
$ ./rpma_fio_bench.sh
```

## Analyzing the results

All of the benchmarking tools described above generates a standardized CSV format output files. Which can be further processed using `csv_compare.py` to generate comparative charts.

### Example of comparing the obtained results

To generate a chart comparing the obtained results you can feed them into the script as follows:

```sh
$ ./csv_compare.py --output_layout lat_avg ib_read_lat-21-01-31-072033.csv rpma_fio_apm_read_lat_th1_dp1_dev_dax0.1-21-01-31-073733.csv --output_with_tables
```

With the help of additional parameters, you can adjust various aspects of the output.

## Reporting

Instead of running all separate workloads you can run a comprehensive set of workloads using the following set of commands and generate the RPMA performance report.

Run all benchmarks required for the performance report:

```sh
$ export REMOTE_SUDO_NOPASSWD=1
$ export REMOTE_RNIC_PCIE_ROOT_PORT=$pcie_root_port
$ export REMOTE_JOB_MEM_PATH=/dev/dax0.1

$ ./reprt_bench.sh $SERVER_IP
```

Generate Figures and Appendix charts for the performance report:

```sh
$ mkdir -p results/MACHINE_A
$ mv *.csv results/MACHINE_A
$ export DATA_PATH=/results/MACHINE_A

$ export READ_LAT_MACHINE=MACHINE_A
$ export READ_BW_MACHINE=MACHINE_A
$ export WRITE_LAT_MACHINE=MACHINE_A
$ export WRITE_BW_MACHINE=MACHINE_A
$ export MIX_BW_MACHINE=MACHINE_A
$ export MIX_LAT_MACHINE=MACHINE_A

$ export STAMP=xyz

# charts will be produced to the report_xyz directory
$ ./create_report_figures.sh report
$ ./create_report_figures.sh appendix
```

Generate the performance report and appendices:

```sh
# will create a report_xyz/report.html report_xyz/appendices.html
$ ./create_report.py --report_dir report_xyz --release X.YZ report --test_date "Now" --high_level_setup_figure ./setup.png
$ ./create_report.py --report_dir report_xyz --release X.YZ appendices
```
