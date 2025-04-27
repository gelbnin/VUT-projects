# Cluster Analysis (Project 2 - cluster.c)

## Description

This C program (`cluster.c`) implements a simple single-linkage hierarchical clustering algorithm for 2D objects. It reads object data (ID and X, Y coordinates) from a file. The program iteratively merges the two closest clusters based on the minimum distance between any pair of objects from the two clusters, until a specified (or default) number of clusters remains. The final clusters are printed to standard output. The project involves completing functions within a provided code skeleton (`cluster.c`).

## Compilation

```bash
gcc -std=c99 -Wall -Wextra -Werror -DNDEBUG cluster.c -o cluster -lm
```

## Usage

* FILENAME (Required): The path to the input file containing object data.
* [N] (Optional): The target number of clusters to produce. Must be an integer greater than 0. If omitted, the default value is 1 (all objects merged into a single cluster).

```bash
./cluster FILENAME [N]
```

## Examples

```bash
$ ./cluster objekty 20
Clusters:
cluster 0: 40[86,663]
cluster 1: 43[747,938]
cluster 2: 47[285,973]
cluster 3: 49[548,422]
cluster 4: 52[741,541]
cluster 5: 56[44,854]
cluster 6: 57[795,59]
cluster 7: 61[267,375]
cluster 8: 62[85,874]
cluster 9: 66[125,211]
cluster 10: 68[80,770]
cluster 11: 72[277,272]
cluster 12: 74[222,444]
cluster 13: 75[28,603]
cluster 14: 79[926,463]
cluster 15: 83[603,68]
cluster 16: 86[238,650]
cluster 17: 87[149,304]
cluster 18: 89[749,190]
cluster 19: 93[944,835]
```

```bash
$ ./cluster objekty 8
Clusters:
cluster 0: 40[86,663] 56[44,854] 62[85,874] 68[80,770] 75[28,603] 86[238,650]
cluster 1: 43[747,938]
cluster 2: 47[285,973]
cluster 3: 49[548,422]
cluster 4: 52[741,541] 79[926,463]
cluster 5: 57[795,59] 83[603,68] 89[749,190]
cluster 6: 61[267,375] 66[125,211] 72[277,272] 74[222,444] 87[149,304]
cluster 7: 93[944,835]
```

## Evaluation 

10.35/14