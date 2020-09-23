# **The librpma roadmap [09/21/2020]**
This document presents the current state of the library as of 09/21/2020. It lists high-level features that are going to be released with the library release 0.9 and features that are planned to be developed later.
This roadmap is for informational purposes only, so you should not rely on this information for purchasing or planning decisions. Just like all projects, the items in the roadmap are subject to change or delay, and the continued development or release of a feature on the roadmap is at the sole discretion of the librpma development team.

Status|Title|Description|Expected
---|---|---|---
:ballot_box_with_check:|Connection|Be able to establish an RDMA connection with a remote node.|06/26/2020
:ballot_box_with_check:|Connection example|An example that demonstrates how to establish a connection using librpma.|06/26/2020
:ballot_box_with_check:|Remote read|Be able to read remote persistent memory.|07/10/2020
:ballot_box_with_check:|Read example|An example that demonstrates how to read remote persistent memory to local persistent memory.|07/10/2020
:ballot_box_with_check:|Connection data|Be able to exchange information during the connection establishment procedure.|07/10/2020
:ballot_box_with_check:|Extended read example|An example that demonstrates the use of PMem in the librpma application which performs a read operation.|07/10/2020
:ballot_box_with_check:|Remote write|Be able to write to remote persistent memory.|07/24/2020
:ballot_box_with_check:|Write example|An example that demonstrates how to write data to remote persistent memory and ensure persistence by *RDMA Read*.|07/24/2020
:ballot_box_with_check:|Remote flush|Be able to ensure data placement within the memory subsystem of a remote node.|08/07/2020
:ballot_box_with_check:|Write-flush example|An example that demonstrates how to write and flush data to remote persistent memory.|08/07/2020
:ballot_box_with_check:|Non-blocking completion acquisition|Be able to poll, in a non-blocking way, for RDMA operations completions.|08/07/2020
:ballot_box_with_check:|Non-blocking connection monitoring|Be able to poll for connection status changes in a non-blocking mode.|08/07/2020
:ballot_box_with_check:|Scalable read/write example|An example(s) that demonstrates how to use non-blocking features of the librpma API.|08/07/2020
:ballot_box_with_check:|Atomic write|Be able to execute an atomic write via standard aligned *RDMA Write*.|08/21/2020
:ballot_box_with_check:|Atomic write example|An example that demonstrates how to use atomic write (with a flush) for remote atomic write operations.|09/04/2020
:ballot_box_with_check:|logging support|Be able to have all diagnostic/status information logged into syslog and/or on stderr.|08/21/2020
:ballot_box_with_check:|fs-dax support|Be able to execute read/write operations on memory mmapped as fs-dax (on selected RNICs).|08/21/2020
:ballot_box_with_check:|fs-dax example|An example (+blog post) that demonstrates how to use PMem mmapped as fs-dax for remote operations. **Note**: All the existing PMem examples work fine with fs-dax. No special treatment required so nothing to describe in the blog post.|08/21/2020
:ballot_box_with_check:|SoftRoCE support|An example to demonstrate how to configure and use librpma on an emulated RDMA device. **Note**: We have introduced an easy way to run all of our examples on any system on SoftRoce. Please refer to [the examples README](examples/README) for details.|08/21/2020
:ballot_box_with_check:|Messaging|Be able to exchange simple messages based on *RDMA Send/Receive* mechanism.|09/04/2020
:ballot_box_with_check:|Messaging example|An example that demonstrates how to use messaging API.|09/04/2020
:ballot_box_with_check:|Peer configuration|Peer configuration support|09/18/2020
:ballot_box_with_check:|Legacy OS support|Build system support for legacy Linux distributions|09/18/2020
:ballot_box_with_check:|GPSPM example|An example that demonstrates how to implement GPSPM flushing method on the top of the librpma library|09/18/2020
:black_square_button:|**librpma0.9 release**|**The first public stable librpma version.**|**09/30/2020**
:black_square_button:|Log prefix|Enable a custom, user-defined prefix in log messages|Q1'21
:black_square_button:|Immediate data|Be able to use immediate data for data exchange.|Q1'21
:black_square_button:|Immediate data example| An example that demonstrates how to use immediate data with remote operations (e.g., write).|Q1'21
:black_square_button:|Enhanced atomic write|Be able to execute an atomic write based on the sequence of *RDMA Read* and *RDMA Compare&Swap*.|Q1'21
:black_square_button:|SRQ support|Shared received queue support.|Q1'21
:black_square_button:|AOF replication example|Append-only-file replication example.|Q1'21
:black_square_button:|Verify|Introduce verify operation to the librpma API.|Q1'21
:black_square_button:|Performance tests|Extend tests setup/implementation to cover complex performance scenarios.|Q1'21
:black_square_button:|librpma1.0 release|The librpma release 1.0.|Q1'21

# **The librpma fio engine roadmap [09/21/2020]**
The librpma fio engine is distinguished as a separate value built on the top of the librpma library with its own roadmap that will not affect the librpma release cycle.
Status|Title|Description|Expected
---|---|---|---
:black_square_button:|Fio engine for writing|Introduce Fio engine example for librpma.|10/02/2020
:black_square_button:|Fio engine for reading|Support for RPMem read operation.|Q4'20
:black_square_button:|Fio instruction|An instruction that demonstrates how to benchmark librpma using Fio|Q4'20
:black_square_button:|Fio engine for pull replication method|Fio engine support for pull replication method|Q4'20
:black_square_button:|Fio engine for GPSPM|Fio engine support for GPSPM flush.|Q4'20

## Contact Us

For more information about both roadmaps, please contact
Tomasz Gromadzki (tomasz.gromadzki@intel.com).

## More Info

Please read [README.md](README.md) for more information about the project.
