# EBat - Enumerating All Boolean Matches Tool

EBat solves the all Boolean matches problem, that is, given two (single-output combinational) circuits, it generates all the matches between the two ciruits.


## How to build

Please consider the following before continuing: 
- Compilation requires g++ version 10.1.0 or higher.
- We use CMake for building the tool, please verify that you have CMake VERSION 3.8 or higher.

To build the tool, first we assume the projet was unzip from the zip file, and the you enter the directory.

After that, run the following script:

```
./scripts/build_tool_from_zip.csh
```

This will create new folder named "**build**" and will compile the tool in release mode.

Inside the new **build** folder the tool **boolmatch_tool** should be generated.

After building the tool in the "build" directory, you should be able to run the tool, for example ask for help:

```
./boolmatch_tool -h
```

## Reproducing the experiments in our SAT'24 submission

This section lays out how to reproduce the experiments, reported in Tables 1 and 2 in our paper submission.

### Benchmarks

In total we used 388 benchmarks, consist of 4 benchmark families.
Each benchmark exist under the benchmarks directory in the form `%BENCH_FAMILY_ORIG/%BENCH_BASED_ORIG/%BENCH/(src|trg)/(src|trg).aag`

For example `benchmarks/iscas85/iscas85_c6288/last_out-vs-or` mean we consider  the `iscas85` family where we take the benchmark `iscas85_c6288` and compare `last_out` (taking the last output) vs  `or` (the disjunction of the outputs).
Under each benchmark we have two directories, `src`, and `trg` that contain the actual AIGER file (it may contain also the original benchmark file in different format).


### Reproducing NP-Equivalence Table Result

First, please note that for NP-Equivalence you need to to allowed map with negated input with the parameter `/alg/allow_input_neg_map 1`.
We provide the default mode for the tool that achive the best result in the table for EbatC and EbatP algorithms, and how to remove\change part of the algorithm to reproduce the other lines in the table.

- `/mode EBatC_NP_best`: EbatC + default blocking + BTS&MUC + Strengthening
- `/mode EBatC_NP_best /alg/use_top_to_bot_sim 0`: EbatC + default blocking + **FTS&MUC** + Strengthening
- `/mode EBatP_NP_best`: **EbatP** + default blocking + BTS&MUC + Strengthening
- `/mode EBatC_NP_best /alg/use_ucore 0`: EbatC + default blocking + **BTS** + Strengthening
- `/mode EBatC_NP_best /alg/block/use_ucore_for_valid_match 0`: EbatC + default blocking + BTS&MUC + **No_Strengthening**
- `/mode BOOM_NP_base`: EBatS + default blocking + BTS + No_Strengthening