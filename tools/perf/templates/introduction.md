The Remote Persistent Memory Access (RPMA) library (librpma) uses Remote Direct Memory Access (RDMA) to provide easy to use interface for accessing Persistent Memory (PMem) on the remote system. It is a user-space library which may be used with all available RDMA implementations (InfiniBand&trade;, iWARP, RoCE, RoCEv2) from various providers as long as they support the standard RDMA user-space Linux interface namely the libiverbs library.

The RPMA-dedicated FIO engines are created as complementary pairs namely librpma\_apm\_client along with librpma\_apm\_server and librpma\_gpspm\_client along with librpma\_gpspm\_server. For the simplicity sake, both parts are implemented independently without any out-of-band communication or daemons allowing to prepare the target memory for I/O requests. The server engine prepares memory according to provided job file (either DRAM or PMem), registers it in the local RDMA-capable network interface (RNIC) and waits for the preconfigured (via the job file) number of incoming client's connections. The client engine establishes the preconfigured number of connections with the server. After these setup steps the client engine starts executing I/O requests against the memory exposed on the server side.

The RPMA library and any application using it (including FIO with dedicated engines) should work on all flavours of RDMA transport but it is currently tested against RoCEv2.

For the sake of the measurements for this report, the FIO process is limited to execute and allocate all its buffers from a single NUMA node to avoid costly cross-NUMA synchronizations (e.g. using Ultra Path Interconnect).

Figure 0 shows a high-level schematic of the systems used for testing in the rest of this report. The setup consists of two individual systems (one used as the initiator and one used as the target). The target system has six Intel&reg; Optane&trade; Persistent Memory devices connected to the respective NUMA node. Both the initiator and the target are equipped with 100Gbps Mellanox ConnectX-5 NICs. The initiator is connected to the target system directly using QSFP28 cables without any switches. From two ports available on NICs only one is in use during the measurements.

![](./Figure_000{{hl_ext}})
**Figure 0**: High-Level RPMA performance testing setup
