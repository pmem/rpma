#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# __init__.py -- required for python imports
#

"""A set of entities dedicated to run benchmarks and process the results

On the input you have to provide:

- a system configuration (for details about the required and optional parameters
  please see https://github.com/pmem/rpma/blob/master/tools/perf/CONFIG.JSON.md;
  `lib.bench.Bench.config`)
  and
- one or more parts of the report you want to generate. You can think of a part
  as a few pages from a document containing one or more figures and some textual
  content. These things are provided by two means where:
    - figures are provided as a JSON file e.g.:
      https://github.com/pmem/rpma/tree/master/tools/perf/figures
    - the textual content is provided via pair of a Markdown and a JSON files
      both stored in the `templates/` directory:
      https://github.com/pmem/rpma/tree/master/tools/perf/templates

**Note**: All three files of a single part shares the same name. The
`report_bench`'s `--figures` argument accepts just part's figures JSON file(s).
The files under the `templates/` directory are matched based on the name e.g.

- `figures/write.json` (part's figures JSON file),
- `templates/part_write.json` and
- `templates/part_write.md`

A single figure (`lib.figure.Figure`) consist of one or more series of data
points each of which is a single benchmark run (`lib.benchmark.base.Benchmark`).
A series has to describe requirements it impose on the benchmarking environment
(`lib.Requirement.Requirement`) e.g.

```json
"requirements": {
    "direct_write_to_pmem": false
}
```

**Note**: `templates/part_*.*` files are optional. Especially when you decide to
prepare a custom part's figures file. But in this case you can not use
`report_create` to generate a report including your custom part. It will end up
with an error.
"""
