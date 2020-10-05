# **The librpma roadmap [10/01/2020]**
This document presents the current state of the library as of 10/01/2020. 
It lists high-level features that are going to be released with the library after the release 0.9.
This roadmap is for informational purposes only, so you should not rely on this information for purchasing or planning decisions. Just like all projects, the items in the roadmap are subject to change or delay, and the continued development or release of a feature on the roadmap is at the sole discretion of the librpma development team.

Status|Title|Description|Expected
---|---|---|---
:black_square_button:|Immediate data|Be able to use immediate data for data exchange.|Q1'21
:black_square_button:|Immediate data example| An example that demonstrates how to use immediate data with remote operations (e.g., write).|Q1'21
:black_square_button:|Enhanced atomic write|Be able to execute an atomic write based on the sequence of *RDMA Read* and *RDMA Compare&Swap*.|Q1'21
:black_square_button:|SRQ support|Shared received queue support.|Q1'21
:black_square_button:|AOF replication example|Append-only-file replication example.|Q1'21
:black_square_button:|Verify|Introduce verify operation to the librpma API.|Q1'21
:black_square_button:|Performance tests|Extend tests setup/implementation to cover complex performance scenarios.|Q1'21
:black_square_button:|Multi-threading tests|Extend tests setup/implementation to cover complex multi-threading scenarios.|Q1'21
:black_square_button:|librpma1.0 release|The librpma release 1.0.|Q1'21

# **The librpma fio engine roadmap [10/01/2020]**
The librpma fio engine is distinguished as a separate value built on the top of the librpma library with its own roadmap that will not affect the librpma release cycle.
Status|Title|Description|Expected
---|---|---|---
:black_square_button:|Fio engine for writing|Introduce Fio engine example for librpma.|Q4'20
:black_square_button:|Fio engine for reading|Support for RPMem read operation.|Q4'20
:black_square_button:|Fio instruction|An instruction that demonstrates how to benchmark librpma using Fio|Q4'20
:black_square_button:|Fio engine for pull replication method|Fio engine support for pull replication method|Q4'20
:black_square_button:|Fio engine for GPSPM|Fio engine support for GPSPM flush.|Q4'20

## Contact Us

For more information about both roadmaps, please contact
Tomasz Gromadzki (tomasz.gromadzki@intel.com).

## More Info

Please read [README.md](README.md) for more information about the project.
