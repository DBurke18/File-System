# File-System

## Overview

This project involves developing a fully functional file system driver, handling essential file operations, implementing efficient data caching, and supporting network communication. It showcases key aspects of C programming, such as file management, array manipulation, caching, and socket-based network communication between a client and server.

## Features

- **File System Driver**: Implements core file operations such as opening, reading, writing, seeking, and closing files. These operations translate high-level commands into disk-level operations, which are communicated with a simulated disk controller over a network.
  
- **Network Communication**: The project includes a network layer where all file operations (like reading and writing data) are transmitted to a remote disk controller via TCP sockets. The client establishes a connection, sends command blocks, and receives responses for each file system request.

- **Caching (LRU)**: An efficient Least Recently Used (LRU) cache reduces the need for frequent network access. Recently accessed disk sectors are stored in the cache, and if a sector is found in the cache, the driver retrieves it without contacting the disk controller.

- **Data Management**: The system processes floating-point data and performs file operations by managing arrays and buffers for both reading and writing. The project includes handling file position pointers and ensuring the integrity of file data through sequential operations.

- **Workload Simulation**: The file system is extensively tested using workload files, which simulate real-world file operations. The simulator reads the workload, performs file operations, and validates the results by comparing the contents of the files to expected values.

## Compilation and Testing

To compile the project, use the following commands:

```bash
make clean
make
```

To test the program, use the following commands:

```bash
./fs3_client -v -l fs3_client_log_small.txt assign4-small-workload.txt
