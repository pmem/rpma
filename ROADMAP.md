# **librpma roadmap [WIP]**

This is the roadmap of librpma library. It consists of high-level features that are going to be released with the library release 1.0.
This roadmap is for informational purposes only, so you should not rely on this information for purchasing or planning purposes. Just like all projects, the items in the roadmap are subject to change or delay, and the continued development or release of a feature on the roadmap is at the sole discretion of the librpma development team.
The roadmap is 

Status|Title|Description|Expected
---|---|---|---
[x]|Connection|Be able to establish RDMA connection with a remote node|ww26'20
[x]|Connection example|Example that demonstrates how to establish connection using librpma.|ww26'20
[x]|Remote read|Be able to read remote persistent memory|ww28'20
[x]|Read example|Example that demonstrates how to read remote persistent memory to local persistent memory.|ww28'20
[x]|Connection data|Be able to exchange information during connection establish procedure|ww28'20
[x]|Read example extended|An example that demonstrates how to use connection time data exchange do deliver a memory handle to the opposite side of a connection|ww28'20
[ ]|Remote write|Be able to write to remote persistent memory|ww30'20
[ ]|Remote flush|Be able to ensure data securely stored in a remote node|ww30'20
[ ]|Write example|An example that demonstrates how to write and flush data to a remote persistent memory|ww30'20
[ ]|Fio engine|Build Fio engine for RPMEM on the top of librpma|ww32'20
[ ]|Fie example|An example that demonstrates how to Fio with librpma|ww34'20
[ ]|Atomic write|Be able to execute atomic write via standard RDMA Write|ww32'20
[ ]|Atomic write example|An example that demonstrates how to use atomic write (with and flush) for remote atomic write operations|ww30'20
[ ]|Non-blocking events acquisition|Be able to poll in a non-blocking way for RDMA related operations events|ww34'20
[ ]|Non-blocking connection monitoring|Be able to poll for connection status changes in a non-blocking mode|ww34'20
[ ]|Scalable read/write example|An example that demonstrates how to use non-blocking features of the librpma API|ww34'20
[ ]|logging support|Be able to have all diagnostic/status information logged into syslog and/or on stderr|ww34'20
[ ]|fs-dax support|Be able to execute read/write operations on memory mounted as fs-dax (on selected RNICs)|ww36'20
[ ]|fs-dax example|AN example (+blog post) describing how to use PMem mounted as fs-dax for remote operations.|ww36'20
[ ]|SoftRoCE support|An example how to configure and use librpma on a single host loopback conection|ww36'20
[ ]|Messaging|Be able to exchange simple messages basded on RDMA Send/Receive mechanism|ww38'20
[ ]|Messaging example|An example that demonstrates how to use messaging API|ww38'20 
[ ]|Atomic write Ext.|Be able to execute atomic write based on the sequence of RDMA.Read and RDMA.Compare&Swap|ww40'20
[ ]|Immediate data|Be able to use immediate data for data exchange|ww40'20
[ ]|Immediate data example| An example that demonstrates how to use immediate data with different remote operations like e.g.  write|ww40'20
[ ]|librpma1.0 release|First public librpma release|ww40'20



## Contact Us

For more information about the library roadmap, missing features,sugested extensions, contact
Tomasz Gromadzki (tomasz.gromadzki@intel.com).

## More Info

Please read [README.md](README.md) for more information about the project.
