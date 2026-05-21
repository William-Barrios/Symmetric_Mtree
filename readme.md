# Symmetric M-Tree

Project developed for the ADE (Advanced Data Structures) course.  
This repository implements a Symmetric M-Tree supporting range queries and exact match queries.

## Features

- Symmetric M-Tree implementation
- Range query support
- Exact match query support
- JSON dataset input
- Configurable maximum number of child nodes

## Build Instructions

Inside the project directory:

```bash
mkdir build
cd build
cmake ..
make
```

## Execution

Inside the `build` directory:

```bash
./MTree <path_to_json_file> <max_child_nodes>
```

Example:

```bash
./MTree /Users/williambarrios/Downloads/diabete_missing_values.json 5
```

## Query Examples

Examples of query syntax and usage can be found in:

```text
queries.txt
```

## References

This project is an independent educational implementation inspired by concepts presented in the original Symmetric M-Tree research paper.
