# RPMA benchmark in multilink configuration

When we want to run the benchmark in Multi Connection, we have to make changes to the cod:
- file fio.py
- librpma_apm-client.fio
- librpma_apm-server.fio
- librpma_gpspm-client.fio
- librpma_gpspm-server.fio
- config.json

Our tool is not adapted to `double-benchmarking` that's why we have to add one `IP address` in the `config.json` (e.g. **192.168.101.4**) file which is used by `*client.fio` and `*server.fio`. 
Moreover we have to hardcode **`another`** (e.g. **192.168.102.4**) `IP address` in these files.

For every `*client.fio` file from the folder `/rpma/tools/perf/fio_jobs` we add 2nd client section `[client]` and remove `serverip` from `[global]`, `serverip` add it in `[client]` nr 1 and 2nd
```sh
[global]
...
serverip=${serverip}
...
```
```sh
[client] 
...
serverip=192.168.102.4

[client]
...
serverip=${serverip}
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
We have to reduce the number of threads.
```sh
__THREADS_VALUES = [1, 2, 4, 8, 12, 16]
```
We create 2 namespaces in 1 region in PMem.
Finally we have to add our `/dev/dax` in the `config.json` file.
```sh
"REMOTE_PMEM_PATH_2": "/dev/dax0.1"
```