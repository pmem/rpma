# **librpma roadmap [WIP]**

This document presents the librpma library roadmap as it shaped on 07/21/2020. It lists high-level features that are going to be released with the library release 1.0.
This roadmap is for informational purposes only, so you should not rely on this information for purchasing or planning decisions. Just like all projects, the items in the roadmap are subject to change or delay, and the continued development or release of a feature on the roadmap is at the sole discretion of the librpma development team.

Status|Title|Description|Expected
---|---|---|---
[x]|Connection|Be able to establish an RDMA connection with a remote node.|06/26/2020
[x]|Connection example|An example that demonstrates how to establish a connection using librpma.|06/26/2020
[x]|Remote read|Be able to read remote persistent memory.|07/10/2020
[x]|Read example|Example that demonstrates how to read remote persistent memory to local persistent memory.|07/10/2020
[x]|Connection data|Be able to exchange information during connection establishment procedure.|07/10/2020
[x]|Extended read example|An example that demonstrates how to use connection time data exchange do deliver a memory handle to the opposite side of a connection.|07/10/2020
[ ]|Remote write|Be able to write to remote persistent memory.|07/24/2020
[ ]|Remote flush|Be able to ensure data placement within the memory subsystem of a remote node.|07/24/2020
[ ]|Write example|An example that demonstrates how to write and flush data to remote persistent memory.|07/24/2020
[ ]|Fio engine|Build Fio engine for RPMEM on the top of librpma.|08/07/2020
[ ]|Fie example|An example that demonstrates how to Fio with librpma|08/07/2020
[ ]|Atomic write|Be able to execute atomic write via standard RDMA Write.|08/07/2020
[ ]|Atomic write example|An example that demonstrates how to use atomic write (with and flush) for remote atomic write operations.|08/07/2020
[ ]|Non-blocking events acquisition|Be able to poll in a non-blocking way for RDMA related operations events.|08/21/2020
[ ]|Non-blocking connection monitoring|Be able to poll for connection status changes in a non-blocking mode.|08/21/2020
[ ]|Scalable read/write example|An example that demonstrates how to use non-blocking features of the librpma API.|08/21/2020
[ ]|logging support|Be able to have all diagnostic/status information logged into syslog and/or on stderr.|08/21/2020
[ ]|fs-dax support|Be able to execute read/write operations on memory mounted as fs-dax (on selected RNICs).|09/04/2020
[ ]|fs-dax example|An example (+blog post) describing how to use PMem mounted as fs-dax for remote operations.|09/04/2020
[ ]|SoftRoCE support|An example of how to configure and use librpma on a single host loopback connection.|09/04/2020
[ ]|Messaging|Be able to exchange simple messages based on RDMA Send/Receive mechanism.|09/18/2020
[ ]|Messaging example|An example that demonstrates how to use messaging API.|09/18/2020 
[ ]|Enhanced atomic write|Be able to execute atomic write based on the sequence of RDMA.Read and RDMA.Compare&Swap.|10/02/2020
[ ]|Immediate data|Be able to use immediate data for data exchange.|10/02/2020
[ ]|Immediate data example| An example that demonstrates how to use immediate data with remote operations (e.g., write).|10/02/2020
[ ]|librpma1.0 release|The first public librpma release.|10/02/2020

## Contact Us

For more information about the library roadmap, please contact
Tomasz Gromadzki (tomasz.gromadzki@intel.com).

## More Info

Please read [README.md](README.md) for more information about the project.
