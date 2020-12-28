# The librpma roadmap [12/28/2020]
This document presents the current state of the library as of 12/28/2020. 
It lists high-level features that are going to be released with the library after the release 0.9.
This roadmap is for informational purposes only, so you should not rely on this information for purchasing or planning decisions. Just like all projects, the items in the roadmap are subject to change or delay, and the continued development or release of a feature on the roadmap is at the sole discretion of the librpma development team.

Please let us know if you find any of the items listed below interesting for you; either you expect this item to be added to the library, or you would like to deliver a source code solution for it.

**Note**: Please take a look at [CONTRIBUTING.md](CONTRIBUTING.md) before you start developing.

**Note**: Please notice that work has been already started for items marked with :construction: status.

Status|Title|Description|Expected
---|---|---|---
:black_square_button:|Immediate data|Be able to use immediate data for data exchange.|Q1'21
:black_square_button:|Immediate data example| An example that demonstrates how to use immediate data with remote operations (e.g., write).|Q1'21
:construction:|Enhanced atomic write|Be able to execute an atomic write based on the sequence of *RDMA Read* and *RDMA Compare&Swap*.|Q2'21
:black_square_button:|SRQ support|Shared received queue support.|Q1'21
:construction:|Performance tests|Extend tests setup/implementation to cover complex performance scenarios.|Q1'21
:construction:|Multi-threading tests|Extend tests setup/implementation to cover complex multi-threading scenarios.|Q1'21
:black_square_button:|librpma1.0 release|The librpma release 1.0.|Q2'21
:construction:|Verify|Introduce verify operation to the librpma API.|Q3'21

:black_square_button: - planned, :construction: - under development, :ballot_box_with_check: - done

# The librpma Fio engine roadmap [12/28/2020]
The librpma fio engine is distinguished as a separate value built on the top of the librpma library with its own roadmap that will not affect the librpma release cycle.

You can follow the actual development of the Fio engine on [github.com/pmem/fio](https://github.com/pmem/fio) repo. 

The final version will be upstreamed to the main Fio repository.

Status|Title|Description|Expected
---|---|---|---
:ballot_box_with_check:|Fio engine for reading|Support for RPMA read benchmarking.|Q4'20
:construction:|Fio engine for writing|Support for RPMA write (APM) benchmarking.|Q1'21
:construction:|Fio engine for mix-mode|Support for RPMA read/write (APM) benchmarking.|Q1'21
:construction:|RPMA benchmarking HOWTO|An instruction that demonstrates how to benchmark RPMem using Fio|Q1'21
:construction:|Fio engine for GPSPM|Fio engine support for GPSPM flush.|Q1'21

:black_square_button: - planned, :construction: - under development, :ballot_box_with_check: - done

# Contact Us

For more information about both roadmaps, please contact
Tomasz Gromadzki (tomasz.gromadzki@intel.com).

# More Info

Please read [README.md](README.md) for more information about the project.
