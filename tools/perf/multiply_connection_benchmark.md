# RPMA benchmark in multilink configuration

The RPMA performance analysis runtime environment allows tests to be carried out in a configuration of multiple parallel connections between nodes
To execute such a benchmark, changes must be applied to the following configuration files:
- file fio.py
- librpma_apm-client.fio
- librpma_apm-server.fio
- librpma_gpspm-client.fio
- librpma_gpspm-server.fio
- config.json

Our tool is not adapted to `double-benchmarking` that's why we have to add one `IP address` (e.g. **192.168.101.4**) in the `config.json` file which is used by `*client.fio` and `*server.fio`. 
Moreover we have to hardcode **`another`** (e.g. **192.168.102.4**) `IP address` in these files.

For every `*client.fio` file from the folder `/rpma/tools/perf/fio_jobs` `serverip` from `[global]` section shall be moved to the `[client]` section:
```
[global]
...
# serverip=${serverip} #comment out or removed from [global] section
...
[client]
...
serverip=${serverip} # ${serverip} is used only by the first client
...
```
Additionally, we have to create a new separate client section with a statically set server IP:
```
[client]
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
serverip=192.168.102.4
```

For every file  `*server.fio` from the folder `rpma/tools/perf/fio_jobs` we add 2 `[server]` and remove `serverip` from `[global]`, `serverip` add to `[server]`, we need add `filenam1` and `filename2`
```sh
[server]
...
filename=${filename1}


[server]
...
filename=${filename2}
```
In the `/rpma/tools/perf/lib/benchmark/runner/fio.py` file we append env of `filename2` variable in the if condition.
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
Below is the code that we have based on
```sh
if pmem_path is None:
            args.extend(['--filename=malloc', '--create_on_open=1'])
        elif '/dev/dax' in pmem_path:
            # DeviceDAX
            args.append('--filename={}'.format(pmem_path))
        else:
            args.append('--filename_format={}.\\$jobnum'.format(pmem_path)
```
We recommend reducing the number of threads.
```sh
__THREADS_VALUES = [1, 2, 4, 8, 12, 16]
```
Orginal
```sh
__THREADS_VALUES = [1, 2, 4, 8, 12, 16, 32, 64]
```
We create 2 namespaces in 1 region in PMem.
Finally we have to add our `/dev/dax` in the `config.json` file.
```sh
"REMOTE_PMEM_PATH_2": "/dev/dax0.1"
```