# What do we have currently

- report_bench.sh (MANUAL)
    - rpma_fio_bench.sh -> JSON
        - fio_json2csv.py -> CSV
            - csv2standardized.py -> CSV
    - ib_read.sh -> CSV
        - csv2standardized.py -> CSV
- create_report_figures.sh <- CSVs
    - csv_compare.py -> PNG
- create_report.py <- PNG -> HTML

## Issues

- report_bench.sh has to be synchronized with create_report_figures.sh
- create_report_figures.sh is very complex
- create_report.py has to take into account naming convention of the output figures produced by create_report_figures.sh
- csv_compare.py sew up figures with selected subset of data whereas the data itself can be easily presented as HTML instead of putting it directly on the picture
- multiple figures per picture seems a little bit too crowded

# Reverse engineer what we are doing

1. We have multiple parts of report e.g. read, write, mix, aof, cpuload...
2. All these parts have more or less the same preamble
3. When a report is generated one can mix and match:
    - the preamble (opt-in)
    - various parts in any order
4. Each part have a set of pictures
5. Each picture is:
    - a set of data - where does it come from?
    - title
    - axis scale
6. A picture you want to make determines what benchmarks you have to run

# Entities

- part
    - variables[]
    - figures[]
        - description
        - data[]
            - benchmark

# Benchmarking



Output:

- report_XYZ/
    - ib_read_vs_rpma_read_dram_lat.json
        - includes data for lat_avg, lat_pctl_999, and lat_pctl_9999 figures
    - rpma_read_dram_vs_pmem_lat.json
        - ...
    - ib_read_bw_vs_rpma_read_dram_bw.json
        - includes data for bw_bs and bw_th figures
    - rpma_read_dram_vs_pmem_bw.json
        - ...

# File transformation sequence

## 1. Report figures definition

- figures/
    - read.json
    - ...

## 2. report_bench.py

### Input

- one or more figures .json e.g. `read.json` files and `config.json` OR
- `bench.json` in case of benchmarking interruption

### Output report_XYZ/

- Stage 1
    - `bench.json`
        - all untangled figures to be generated
        - all benchmark configurations to be run
        - the current status of execution `"done": true`
- Stage 2
    - `result_hash_01.json`
        - a complete result of a single benchmarking series
        - `hash_01` is an output of a hashing function taking into consideration all benchmark input parameters
        - `hash_01` execution is tracked by `bench.json` (`"done": true`)
        - `hash_01` is mentioned by one or more figures as data series source
    - ...
- Stage 3
    - `figures_ib_read_vs_rpma_read_dram_lat.json`
        - `ib_read_vs_rpma_read_dram_lat` is mentioned once or more by a report part
        - contains an object which keys are an result of `read.json` figures definition
        - keys are also mentioned in the report part

## 3. report_figures.py

### Input

- `bench.json`
    - contains a flat list of all figures for which data have been collected

### Output

- `figures_001_ib_read_vs_rpma_read_dram_lat_avg.png`
- `figures_001_ib_read_vs_rpma_read_dram_lat_avg.json`
    - data table with all the data used for any particular figure
- `figures_002_ib_read_vs_rpma_read_dram_lat_pctls_999.png`
- `figures_002_ib_read_vs_rpma_read_dram_lat_pctls_999.json`
- `figures_003_ib_read_vs_rpma_read_dram_lat_pctls_9999.png`
- `figures_003_ib_read_vs_rpma_read_dram_lat_pctls_9999.json`

## 4. report.py

### Input

- `bench.json` contains
    - list of all parts considered during benchmarking
    - contains a flat list of all figures for which data have been collected

### Output

- report_XYZ/
    - report.html

# What has to be done?

- a JSON defintions of already existing figures - 1day
- figure flattener (a part of report_bench.py or a separate lib/) - 2days
- constructing a command line and running a benchmark (*) - 1day
- generating JSON instead of CSV as a standardized output (csv2standardized.py) - 1day
- combaine all JSON series according to a figure definition into a single JSON file - 1day
- combine existing csv_compare.py parts into a lib/figuregen.py module dedicated to generating figures from JSON files (probably a lot simpler comparing to what there is now) - 2days
- create a module generating HTML tables with a data from the JSON file - 1day
- combine exising parts of report generation to make use of JSON input - 1day

**SUM**: 10 days
