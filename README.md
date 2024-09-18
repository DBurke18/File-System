# File-System

## Overview

This project involves developing a complete file system driver, handling core file operations, efficient data caching, and network communication. It showcases advanced C programming techniques, including array manipulation, mathematical transformations, filesystem operations, and client-server interactions over a network.

## Features

- **Data Management**: The system processes floating-point data with specific transformations, rounds the results to integers, and performs operations such as calculating the sum and greatest common divisors (GCD) of adjacent array elements. It also includes functionality to graph mathematical functions like sine using ASCII art.

- **File System Driver**: A user-space file system driver is implemented to handle basic file operations such as mounting, unmounting, opening, reading, writing, seeking, and closing files. The driver interfaces with a simulated disk controller, translating high-level file operations into low-level disk commands using a command block structure.

- **LRU Caching**: To optimize performance, a Least Recently Used (LRU) cache is incorporated, reducing the frequency of disk access by caching sectors in memory. The cache operates in a write-through mode, ensuring that data remains synchronized between memory and disk. Detailed performance metrics, including cache hits and misses, are logged.

- **Network Communication**: The project extends the file system driver to function over a network, where all file operations are transmitted to a remote disk controller. The client communicates with the server via socket connections, enabling the driver to handle file operations across different machines efficiently.

- **Workload Testing**: The file system is rigorously tested with different workload sizes, ranging from small to large, simulating thousands of file operations under various conditions to ensure scalability and robustness.

## Compilation and Testing

To compile the project, use the following commands:

```bash
make clean
make
```

To test the program, use the following commands:

```bash
./fs3_client -v -l fs3_client_log_small.txt assign4-small-workload.txt
