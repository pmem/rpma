{% if menu is true %}
<a class="pure-menu-heading release" href="#main">{{release}}</a>
<a class="pure-menu-heading" href="#test-setup">Test Setup</a>
<ul class="pure-menu-list">
    <li class="pure-menu-item"><a href="#common-cfg" class="pure-menu-link">Common Configuration</a></li>
    <li class="pure-menu-item"><a href="#target-cfg" class="pure-menu-link">Target Configuration</a></li>
    <li class="pure-menu-item"><a href="#bios-settings" class="pure-menu-link">BIOS Settings</a></li>
    <li class="pure-menu-item"><a href="#security" class="pure-menu-link">Security</a></li>
    <li class="pure-menu-item"><a href="#benchmarking" class="pure-menu-link">Benchmarking</a></li>
</ul>
<a class="pure-menu-heading" href="#introduction">Introduction</a>
{% else %}

**Testing Date**: {{test_date}}

{% if authors %}
**Performed by**:

{{authors}}
{% endif %}

## Audience and Purpose

This report is intended for people who are interested in evaluating RPMA performance.

The purpose of the report is not to imply a single "correct" approach, but rather to provide a baseline of well-tested configurations and procedures that produce repeatable results. This report can also be viewed as information regarding best-known methods/practices when testing the RPMA performance.

## Disclaimer

Performance varies by use, configuration and other factors. Learn more at [www.Intel.com/PerformanceIndex​​](http://www.intel.com/PerformanceIndex).

Performance results are based on testing as of dates shown in configurations and may not reflect all publicly available ​updates. See backup for configuration details. No product or component can be absolutely secure.

Intel technologies may require enabled hardware, software or service activation.

Your costs and results may vary.

Intel does not control or audit third-party data.  You should consult other sources to evaluate accuracy.

&copy; Intel Corporation. Intel, the Intel logo, and other Intel marks are trademarks of Intel Corporation or its subsidiaries.  Other names and brands may be claimed as the property of others.

<h2 id="test-setup">Test Setup</h2>

<h3 id="common-cfg">Common Configuration (both initiator and target)</h3>

{{configuration.common}}

<h3 id="target-cfg">Target Configuration</h3>

{{configuration.target.description}}

{{configuration.target.details}}

<h3 id="bios-settings">BIOS Settings</h3>

{{configuration.bios.settings}}

Excerpt:

{{configuration.bios.excerpt}}

<h3 id="security">Kernel & BIOS spectre-meltdown information</h3>

{{configuration.security}}

<h3 id="benchmarking">Benchmarking process</h3>

To learn how the collection, processing, and presentation of the data was conducted please see [https://github.com/pmem/rpma/blob/master/tools/perf/BENCHMARKING.md​​](https://github.com/pmem/rpma/blob/master/tools/perf/BENCHMARKING.md).

<h2 id="introduction">Introduction to RPMA</h2>

The Remote Persistent Memory Access (RPMA) library (librpma) uses Remote Direct Memory Access (RDMA) to provide easy to use interface for accessing Persistent Memory (PMem) on the remote system. It is a user-space library which may be used with all available RDMA implementations (InfiniBand&trade;, iWARP, RoCE, RoCEv2) from various providers as long as they support the standard RDMA user-space Linux interface namely the libiverbs library.

The RPMA-dedicated FIO engines are created as complementary pairs namely librpma\_apm\_client along with librpma\_apm\_server and librpma\_gpspm\_client along with librpma\_gpspm\_server. For the simplicity sake, both parts are implemented independently without any out-of-band communication or daemons allowing to prepare the target memory for I/O requests. The server engine prepares memory according to provided job file (either DRAM or PMem), registers it in the local RDMA-capable network interface (RNIC) and waits for the preconfigured (via the job file) number of incoming client's connections. The client engine establishes the preconfigured number of connections with the server. After these setup steps the client engine starts executing I/O requests against the memory exposed on the server side.

The RPMA library and any application using it (including FIO with dedicated engines) should work on all flavours of RDMA transport but it is currently tested against RoCEv2.

The FIO should be configured in a way that guarantees running all its threads and allocating all its buffers from a single NUMA node, the same the used RDMA interface is attached to, to avoid costly cross-NUMA synchronizations (e.g. using Ultra Path Interconnect).

Please see a high-level schematic of the systems used for testing in the rest of this report. {{configuration.description}}

{% if configuration.schematic %}<img src="{{configuration.schematic}}"/>{% endif %}

{% endif %}
