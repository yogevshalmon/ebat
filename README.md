# EBat - Enumerating All Boolean Matches Tool

EBat solves the all Boolean matches problem, that is, given two (single-output combinational) circuits, it generates all the matches between the two circuits.

## Table of Contents
- [Project Description](#project-description)
- [Prerequisites](#prerequisites)
- [How to Build](#how-to-build)
- [Usage](#usage)
- [Reproducing Experiments](#reproducing-experiments)


## Project Description
EBat is a specialized tool designed to solve the all Boolean matches problem. It provides a comprehensive solution for generating matches between two single-output combinational circuits, offering researchers and engineers a powerful method for circuit comparison and analysis.

## Prerequisites
- Compilation requires g++ version 10.1.0 or higher
- CMake VERSION 3.8 or higher

## How to Build
To build the tool, follow these steps:

1. Unzip the project from the zip file
2. Enter the project directory
3. Run the build script:

```bash
./scripts/build_tool_from_zip.csh
```

This will:
- Create a new folder named "**build**"
- Compile the tool in release mode
- Generate the **boolmatch_tool** executable under the "**build**" folder

## Usage
After building, you can run the tool. For example, to see help options:

```bash
./boolmatch_tool -h
```

## Reproducing Experiments
This section details how to reproduce the experiments reported in our Table 1 of our paper submission.

### Benchmark Structure
We used 388 benchmarks across 4 benchmark families. Benchmark files are located in the `benchmarks` directory and are organized in a hierarchical structure:

```
benchmarks/
└── %BENCH_FAMILY_ORIG/
    └── %BENCH_BASED_ORIG/
        └── %BENCH/
            ├── src/
            │   └── src.aag
            └── trg/
                └── trg.aag
```

For example, `benchmarks/iscas85/iscas85_c6288/last_out-vs-or/` represents:
- Benchmark family: `iscas85`
- Base benchmark: `iscas85_c6288`
- Comparison: `last_out` (last output) vs `or` (disjunction of outputs)

Each benchmark contains two directories:
1. `src/`: Contains the source circuit AIGER file
2. `trg/`: Contains the target circuit AIGER file

Both directories may also include the original benchmark file in a different format.

This structure allows for easy organization and access to various circuit comparisons across different benchmark families.

### NP-Equivalence Table Result
First, please note that for NP-Equivalence you need to to allowed map with negated input with the parameter `/alg/allow_input_neg_map 1`.
We provide the default mode for the tool that achive the best result in the table for EbatC and EbatP algorithms, and how to remove\change part of the algorithm to reproduce the other lines in the table.

Available modes:
- `/mode EBatC_NP_best`: EbatC + default blocking + BTS&MUC + Strengthening
- `/mode EBatC_NP_best /alg/use_top_to_bot_sim 0`: EbatC + default blocking + FTS&MUC + Strengthening
- `/mode EbatP_NP_best`: EbatP + default blocking + BTS&MUC + Strengthening
- `/mode EBatC_NP_best /alg/use_ucore 0`: EbatC + default blocking + BTS + Strengthening
- `/mode EBatC_NP_best /alg/block/use_ucore_for_valid_match 0`: EbatC + default blocking + BTS&MUC + No_Strengthening
- `/mode BOOM_NP_base`: EBatS + default blocking + BTS + No_Strengthening