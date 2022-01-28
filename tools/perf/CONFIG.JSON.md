# config.json parameters

Mandatory and optional parameters available to control the `report_bench.py` behaviour via configuration file (`--config`).

## Mandatory parameters

An example usage of mandatory parameters can be found in the [`config.json.example`](./config.json.example).

- "PLATFORM_GENERATION" - generation of the platform, supported values: "Cascade Lake" or "Ice Lake",
- "SERVER_IP" - an IP address of the remote node (the RDMA target) (for example: "192.168.0.1"),
- "JOB_NUMA" - a number of the NUMA node on which benchmarking tools will be run on the RPMA initiator,
- "REMOTE_JOB_NUMA" - a number of the NUMA node on which benchmarking tools will be run on the RPMA target,
- "REMOTE_DIRECT_WRITE_TO_PMEM" - a state of 'Direct Write to PMem' (true or false). It is not required in case of Cascade Lake platforms, when "REMOTE_SUDO_NOPASSWD" is set to true and "REMOTE_RNIC_PCIE_ROOT_PORT" is set correctly (for more details see ["Optional parameters"](#optional-parameters) below).
- "REMOTE_PMEM_PATH" - a path to a PMem on an RPMA target (Device DAX or File System DAX). In case of Device DAX it should be an absolute path to a DAX device (for example "/dev/dax0.0"). In case of File System DAX it should be an absolute path to a file (that may not exist) on this file system, which fio will use as a template to create separate files for each fio job (for example: "/mnt/pmem/file", where "/mnt/pmem/" is a directory where File System DAX is mounted on; fio will create files with names "/mnt/pmem/file.N", where N is the incremental number of the worker thread or process - see the [filename_format command line option][filename-format] for more details). This "REMOTE_PMEM_PATH" is not used and can be set to any value if 'filetype' is set to 'malloc' in a figure used in the benchmark.

For example:

```json
{
    "PLATFORM_GENERATION": "Ice Lake",
    "SERVER_IP": "192.168.0.1",
    "REMOTE_USER": "user",
    "REMOTE_PASS": "password",
    "JOB_NUMA": 0,
    "REMOTE_JOB_NUMA": 0,
    "REMOTE_DIRECT_WRITE_TO_PMEM": true,
    "REMOTE_PMEM_PATH": "/mnt/pmem/file"
}
```

[filename-format]: https://fio.readthedocs.io/en/latest/fio_man.html#cmdoption-arg-filename-format

## Optional parameters

An example usage of optional parameters can be also found in the [`config.json.example`](./config.json.example).

- "AUX_PARAMS" - auxiliary parameters for the local ib_read_* tools. Typically used to set IB device with -d xxx and use rdma_cm for connection setup (-R). See the manuals of ib_read_lat and ib_read_bw.
- "REMOTE_AUX_PARAMS" - auxiliary parameters for the remote ib_read_* tools. See AUX_PARAMS for additional information.
- "REMOTE_JOB_PATH" - an absolute path to the fio job file on the remote node. If not provided, then the fio job file is copied to the '/dev/shm/' directory.
- "IB_PATH" - an absolute path to the directory, where the ib_read_* binaries are located on the local node,
- "REMOTE_IB_PATH" - an absolute path to the directory, where the ib_read_* binaries are located on the remote node,
- "FIO_PATH" - an absolute path to the directory, where the fio binary is located on the local node,
- "REMOTE_FIO_PATH" - an absolute path to the directory, where the fio binary is located on the remote node.

```json
{
    "AUX_PARAMS": "-d rxe_eno1 -R",
    "REMOTE_AUX_PARAMS": "-d rxe_eno1 -R",
    "REMOTE_JOB_PATH": "/dev/shm/librpma_apm-server.fio",
    "IB_PATH": "/usr/local/bin/",
    "REMOTE_IB_PATH": "/usr/local/bin/",
    "FIO_PATH": "/usr/local/bin/",
    "REMOTE_FIO_PATH": "/usr/local/bin/"
}
```

For Cascade Lake platforms you can allow to set up DDIO automatically during the benchmark execution. To make it possible you have to allow passwordless sudo for the user you are using on the target system and provide a PCIe Root Port of the RNIC you are using on the target system. For details please read [Direct Write to PMem][direct-write]. e.g.:

```json
    "REMOTE_SUDO_NOPASSWD": true,
    "REMOTE_RNIC_PCIE_ROOT_PORT": "0000:17:00.0"
```

[direct-write]: https://pmem.io/rpma/documentation/basic-direct-write-to-pmem.html

## Pre and post commands

You can also set a special pre and post commands:
- "REMOTE_CMD_PRE" - a pre-command to be run before the server starts and
- "REMOTE_CMD_POST" - a post-command to be run after the server stops:
that can be used, for example, to collect additional data.

In the definition of those commands you can use existing variables or your own additional custom variables defined alongside:

```json
    "REMOTE_CMD_PRE": "date; echo ${REMOTE_AUX_PARAMS}",
    "REMOTE_CMD_POST": "date",
```

For more examples see ["Collecting SAR data"](#collecting-sar-data) and ["Collecting EMON data"](#collecting-emon-data) below.

### Collecting SAR data

```json
    "REMOTE_ANOTHER_NUMA": 1,
    "REMOTE_RESULTS_DIR": "path",
    "REMOTE_CMD_PRE": "rm -f ${REMOTE_RESULTS_DIR}sar.dat; numactl -N ${REMOTE_ANOTHER_NUMA} sar -u -P ${REMOTE_JOB_NUMA_CPULIST} -o ${REMOTE_RESULTS_DIR}sar.dat 5 > /dev/null",
    "REMOTE_CMD_POST": "sleep 10; killall -9 sar; sadf -d -- -u -P ${REMOTE_JOB_NUMA_CPULIST} ${REMOTE_RESULTS_DIR}sar.dat > ${REMOTE_RESULTS_DIR}sar_${RUN_NAME}.csv"
```

We use "REMOTE_ANOTHER_NUMA" here (different from "REMOTE_JOB_NUMA"), because we want an additional process to be run on another NUMA node.

### Collecting EMON data

```json
    "REMOTE_ANOTHER_NUMA": 1,
    "REMOTE_RESULTS_DIR": "path",
    "EVENT_LIST": "path",
    "REMOTE_CMD_PRE": "source /opt/intel/sep/sep_vars.sh; numactl -N ${REMOTE_ANOTHER_NUMA} emon -i ${EVENT_LIST} > ${REMOTE_RESULTS_DIR}${RUN_NAME}_emon.dat",
    "REMOTE_CMD_POST": "sleep 10; source /opt/intel/sep/sep_vars.sh; emon -stop"
```

### Debug options

There are listed all debug options below. Keep in mind that if you have any of them set, results of benchmarks will be INCORRECT!

For functional testing, you can run each benchmark only for a brief period of time.

```json
    "DEBUG_SHORT_RUNTIME": true
```

For debugging purposes, you can attach tracers to each or both ends of the connections e.g. `gdbserver`.

```json
    "DEBUG_TRACER": "gdbserver localhost:2345",
    "DEBUG_REMOTE_TRACER": "gdbserver localhost:2345"
```

During in-depth analysis of the benchmarking process itself, you may find useful these three debug options:
- `DEBUG_SKIP_RUNNING_TOOLS` allows running all the machinery but prevents the actual benchmarking binaries and the script setting DDIO ([tools/ddio.sh](./tools/ddio.sh)) from being run.
- `DEBUG_SKIP_REMOTE_CMDS` skips running all remote commands including copying files to the remote node.
- `DEBUG_DUMP_CMDS` allows dumping all commands running the benchmark binaries. You can use either or both of them.

```json
    "DEBUG_SKIP_RUNNING_TOOLS": true,
    "DEBUG_SKIP_REMOTE_CMDS": true,
    "DEBUG_DUMP_CMDS": true
```

### Other options

You can also set the timeout (in seconds) of a single run of the 'fio' tool (the default value is 5 minutes):

```json
    "TIMEOUT": 300
```
