# RPMA benchmark in multilink configuration

When we want to run the benchmark in Multi Coneection, we have to make changes to the cod:
- file fio.py
- librpma_apm-client.fio
- librpma_apm-server.fio
- librpma_gpspm-client.fio
- librpma_gpspm-server.fio
- config.json

For every `*client.fio` file from the folder `/rpma/tools/perf/fio_jobs` we add 2nd client section `[client]` and remove `serverip` from `[global]`, `serverip` add it in `client` nr 1 and 2

Examplary file librpma_apm-client.fio

```shell
[global]
ioengine=librpma_apm_client
create_serialize=0 # (required) forces specific initiation sequence
port=7204
thread
disable_clat=1
lat_percentiles=1
percentile_list=99.0:99.9:99.99:99.999

[client]
serverip=192.168.102.4
sync=${sync}
numjobs=${numjobs}
group_reporting=1
iodepth=${iodepth}
readwrite=${readwrite}
rwmixread=70 # valid only for the 'rw' and 'randrw' readwrite modes
blocksize=${blocksize}
ramp_time=${ramp_time}
time_based
runtime=${runtime}

[client]
serverip=${serverip}
sync=${sync}
numjobs=${numjobs}
group_reporting=1
iodepth=${iodepth}
readwrite=${readwrite}
rwmixread=70 # valid only for the 'rw' and 'randrw' readwrite modes
blocksize=${blocksize}
ramp_time=${ramp_time}
time_based
runtime=${runtime}
```

For every file  `*server.fio` from the folder `rpma/tools/perf/fio_jobs` we add 2 `[server]` and remove `serverip` from `[global]`, `serverip` add to server, we need add `filenam1` and `filename2`
Example file or Exemplery file, I would choose one

```sh
[global]
ioengine=librpma_apm_server
create_serialize=0 # (required) forces specific initiation sequence
kb_base=1000 # turns on the straight units handling (non-compatibility mode)
port=7204
thread

[server]
serverip=192.168.102.4
filename=${filename1}
# set to 1 (true) ONLY when Direct Write to PMem from the remote host is possible
# (https://pmem.io/rpma/documentation/basic-direct-write-to-pmem/)
direct_write_to_pmem=${direct_write_to_pmem}
numjobs=${numjobs}
size=100MiB

[server]
serverip=${serverip}
filename=${filename2}
# set to 1 (true) ONLY when Direct Write to PMem from the remote host is possible
# (https://pmem.io/rpma/documentation/basic-direct-write-to-pmem/)
direct_write_to_pmem=${direct_write_to_pmem}
numjobs=${numjobs}
size=100MiB
```

In the `fio.py` file we append env of `filename2` variable in the if condition.

```sh
 if pmem_path is None:
            env.append('filename1=malloc')
            env.append('filename2=malloc')
            args.extend(['--create_on_open=1'])
         elif '/dev/dax' in pmem_path:
            # DeviceDAX
            env.append('filename1={}'.format(pmem_path))
            env.append('filename2={}'.format(self.__config['REMOTE_PMEM_PATH_2']))
```

We create 2 namespaces in 1 region in PMem.

```sh
__THREADS_VALUES = [1, 2, 4, 8, 12, 16]
```

At pmem we create 2 namespaces in 1 region
Finally we have to add our `/dev/dax` in the `config.json` file.

```sh
"REMOTE_PMEM_PATH_2": "/dev/dax0.1"
```
