# EBat - an all-Boolean-matching tool

EBat solves the all-Boolean-matching problem, that is, given two (single-output combinational) circuits, it generates all the matches between the two circuits (non-disjoint).

## Table of Contents
- [Project Description](#project-description)
- [Prerequisites](#prerequisites)
- [How to Build](#how-to-build)
- [Usage](#usage)
- [Reproducing Experiments](#reproducing-experiments)


## Project Description
EBat is a dedicated tool for all-Boolean-matching problem. It provides a comprehensive solution for generating matches between two single-output combinational circuits, offering researchers and engineers a powerful method for circuit comparison and analysis.

## Prerequisites
- Compilation requires g++ version 10.1.0 or higher
- CMake VERSION 3.8 or higher

## How to Build
To build the tool, follow these steps:

### Option 1: Clone the Repository
Clone the repository and run these commands (after entering the repository directory):

```
git submodule init
git submodule update
cmake -S . -B build
cd build
make
```

### Option 2: Download the Project as a Zip File

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
This section provides detailed instructions for reproducing the experimental results presented in Table 1 of our SAT'25 paper.

We conducted our experiments with a timeout of 60 seconds. Note that by default there is not timeout for the tool. You can adjust the timeout using the parameter `/general/timeout ....`.


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

### NP-Equivalence Table Result

To reproduce the NP-Equivalence results, you must enable mapping with negated inputs using the parameter `/alg/allow_input_neg_map 1`.

We provide several modes to reproduce different configurations reported in our paper. The mode (`EBatC_NP_best`) achieves the best results overall. Other modes allow you to modify or remove specific parts of the algorithm to reproduce other results in the table.

#### Available Modes

| Mode | Description | Command |
|------|-------------|---------|
| EbatC (Best) | EbatC + default blocking + BTS&MUC + Strengthening | `/mode EBatC_NP_best` |
| EbatC with FTS&MUC | EbatC + default blocking + FTS&MUC + Strengthening | `/mode EBatC_NP_best /alg/use_top_to_bot_sim 0` |
| EbatP | EbatP + default blocking + BTS&MUC + Strengthening | `/mode EbatP_NP_best` |
| EbatC without MUC | EbatC + default blocking + BTS + Strengthening | `/mode EBatC_NP_best /alg/use_ucore 0` |
| EbatC without Strengthening | EbatC + default blocking + BTS&MUC + No Strengthening | `/mode EBatC_NP_best /alg/block/use_ucore_for_valid_match 0` |
| BOOM (Baseline) | EbatS + default blocking + BTS + No Strengthening | `/mode BOOM_NP_base` |

To use a specific mode, append the corresponding command to the tool execution. For example:

```bash
./boolmatch_tool ../benchmarks/AND.aag ../benchmarks/AND.aag /general/timeout 60 /alg/allow_input_neg_map 1 /mode EBatC_NP_best [other_parameters]
```

### P-Equivalence Table Result

Similar to the NP-Equivalence Table Result, we provide several modes to reproduce different configurations reported in our paper.
The mode (`EBatC_P_best`) achieves the best results for the EbatC algorithm under P-Equivalence. Other modes allow you to modify or remove specific parts of the algorithm to reproduce other results in the table.

| Mode | Description | Command |
|------|-------------|---------|
| EbatC (Best) | EbatC + dynamic blocking + Alternation + Strengthening | `/mode EBatC_P_best` |
| EbatC with Enforce | EbatC + enforce blocking + Alternation + Strengthening | `/mode EBatC_P_best /alg/block/block_match_type 1` |
| EbatP | EbatP + dynamic blocking + Alternation + Strengthening | `/mode EBatP_P_best` |
| EbatC with Fix to 0 | EbatC + dynamic blocking + Fixed to 0 + Strengthening | `/mode EBatC_P_best /alg/use_adap_for_max_val_apprx_strat 0 /alg/max_val_apprx_strat_init_val 0` |
| EbatC with Fix to 1 | EbatC + dynamic blocking + Fixed to 1 + Strengthening | `/mode EBatC_P_best /alg/use_adap_for_max_val_apprx_strat 0 /alg/max_val_apprx_strat_init_val 1` |
| EbatC without Strengthening | EbatC + dynamic blocking + Alternation + No Strengthening | `/mode EBatC_P_best /alg/block/use_ucore_for_valid_match 0` |
| EbatC without Witness-Extension | EbatC + dynamic blocking + _ + Strengthening | `/mode EBatC_P_best /alg/use_max_val_apprx_strat 0` |
| BOOM (Baseline) | EbatS + enforce blocking + _ + No Strengthening | `/mode BOOM_P_base` |
