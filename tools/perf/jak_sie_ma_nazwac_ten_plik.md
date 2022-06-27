# jak_sie_ma_nazwac_ten_plik

When we want to run the benchmark in Multi Coneection, we have to make changes to the cod:
- file fio.py
- librpma_aof_hw-client.fio
- librpma_aof_hw-server.fio
- librpma_aof_sw-client.fio
- librpma_aof_sw-server.fio
- librpma_apm-client.fio
- librpma_apm-server.fio
- librpma_gpspm-client.fio
- librpma_gpspm-server.fio
- config.json


For every file '*client.fio' from the folder '/rpma/tools/perf/fio_jobs' we add 2 client and remove 'serverip' from global, 'serverip' add to server,
Which 'global' we remote 'server ip' and we add it in  'client' nr 1 and 2

Examplary file librpma_aof_hw-client.fio
'''sh
[global]
ioengine=librpma_aof_client
create_serialize=0 # (required) forces specific initiation sequence
port=7204
thread
disable_clat=1
lat_percentiles=1
percentile_list=99.0:99.9:99.99:99.999

# The client will get a remote memory region description after establishing
# a connection.

[client]
serverip=192.168.102.4
mode=hw
sync=${sync}
numjobs=${numjobs}
group_reporting=1
iodepth=${iodepth}
readwrite=${readwrite}
blocksize=${blocksize}
ramp_time=${ramp_time}
time_based
runtime=${runtime}

[client]
serverip=${serverip}
mode=hw
sync=${sync}
numjobs=${numjobs}
group_reporting=1
iodepth=${iodepth}
readwrite=${readwrite}
blocksize=${blocksize}
ramp_time=${ramp_time}
time_based
runtime=${runtime}
'''

For every file  '*server.fio' from the folder 'rpma/tools/perf/fio_jobs' we add 2 server and remove 'serverip' from global, 'serverip' add to server, we need add 'filename2'
Example file librpma_aof_hw-server.fio

'''sh
[global]
ioengine=librpma_aof_server
create_serialize=0 # (required) forces specific initiation sequence
kb_base=1000 # turn on the straight units handling (non-compatibility mode)
port=7204
thread

# The server side spawns one thread for each expected connection from
# the client-side, opens and registers the range dedicated for this thread
# (a workspace) from the provided memory.
# Each of the server threads accepts a connection on the dedicated port
# (different for each and every working thread), accepts and executes flush
# requests, and sends back a flush response for each of the requests.
# When the client is done it sends the termination notice to the server's thread.

[server]
serverip=192.168.102.4
filename=${filename1}
mode=hw
# set to 1 (true) ONLY when Direct Write to PMem from the remote host is possible
# (https://pmem.io/rpma/documentation/basic-direct-write-to-pmem/)
direct_write_to_pmem=${direct_write_to_pmem}
numjobs=${numjobs}
size=100MiB

[server]
serverip=${serverip}
filename=${filename2}
mode=hw
# set to 1 (true) ONLY when Direct Write to PMem from the remote host is possible
# (https://pmem.io/rpma/documentation/basic-direct-write-to-pmem/)
direct_write_to_pmem=${direct_write_to_pmem}
numjobs=${numjobs}
size=100MiB
'''

In the file 'fio.py' we add to condition if - 'filename2' , which we added earlier
'''sh
 if pmem_path is None:
            env.append('filename1=malloc')
            env.append('filename2=malloc')
            args.extend(['--create_on_open=1'])
        elif '/dev/dax' in pmem_path:
            # DeviceDAX
            env.append('filename1={}'.format(pmem_path))
            env.append('filename2={}'.format(self.__config['REMOTE_PMEM_PATH_2']))
'''
We have to reduce the number of threads 
'''sh
__THREADS_VALUES = [1, 2, 4, 8, 12, 16]
'''

At pmem we create 2 namespaces in 1 region

Finally we have to add in the file 'config.json' our /dev/daxa
'''sh
 "REMOTE_PMEM_PATH_2": "/dev/dax0.1"
'''
