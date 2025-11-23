# Ext-File-System Simulation

## Developer: Keese Phillips

## About:
A robust, userspace implementation of a Unix-like file system simulation written in C. This project simulates a virtual disk and manages file allocation, directory structures, and inode tracking, providing a shell-like interface for file system interaction. This project implements a simplified version of the Ext2 file system architecture. It operates on a "virtual disk" (a binary file on the host machine) and manages storage blocks, inodes, and file metadata manually. It bridges the gap between raw disk blocks and high-level file operations.

**The system is designed to handle:** 
- Virtual Disk Management: Creation, mounting, and unmounting of virtual disk files
- Inode-based Allocation: Efficient file tracking using indexed nodes
- Hierarchical Directories: Support for nested directories and file organization
- Shell Simulation: A command-line interface (fs_sim) to interact with the file system interactively
- Features Persistence: Data persists in a virtual disk file between executions
- File Operations: Full support for open, read, write, seek, and close
- Directory Management: mkdir (make directory), ls (list contents), and cd (change directory).Hard Links: Ability to link multiple filenames to a single inode (ln)
- Metadata Handling: Maintenance of Superblocks, Inode Bitmaps, and Data Block Bitmaps

## Installation & Compilation:
**Prerequisites:**
- GCC Compiler (or any standard C compiler)
- Make (optional, but recommended)
- Linux/Unix environment (Ubuntu, MacOS, WSL)

**Build:** To compile the project, run the included Makefile:
```bash
make
```
**Clean:** To clean up build artifacts:
```bash
make clean
``` 
**Run:**
To start the file system simulator, run the executable. 
```bash
./fs_sim
```

## Command Description s
- mkfs: Format the virtual disk and initialize the file system
- mount: Mount the virtual disk for operations.
- touch: Create a new empty file
- mkdir: Create a new directory Automatically creates hard links to . (current) and .. (parent)
- ls: List files and directories in the current path
- cd: Change the current working directory by reading the target Dentry
- cat: Print the contents of a file to the command prompt
- cp: Copy a file, reads source and writes to destination (creates destination if it doesn't exist)
- rm: Remove a file, frees the associated data block and inode
- rmdir:Remove a directory. *Note: Only works if directory is empty (contains only . and ..)*
- ln: Create a hard link. Associates a new filename with an existing inode
- exit: Unmount the disk and exit the simulator

## File Structure
The project is modularized into the following components:
- fs_sim.c: The main entry point. Contains the simulation loop and creates the interactive shell
- file.c: Handles core file logic (cat, cp, rm, ln). Manages file descriptors and read/write pointers
- directory.c: Manages directory entries (mkdir, rmdir, cd), path parsing, and folder listings
- fs.c / fs.h: The core file system logic. Handles inode allocation, block mapping, and superblock management
- disk.c / disk.h: The hardware abstraction layer. Simulates reading/writing raw sectors to the disk file
- API.h / API.c: Defines the public interface for the file system
- fs_util.c: Helper utility functions for bit manipulation and string parsing

## Technical Details
- Max File Size: Files can contain up to 73,216 bytes
- Block Structure: The system utilizes both direct blocks and indirect blocks to manage file data
- Inode Structure: Each file is represented by an inode containing permissions, size, and pointers to data block
- Allocation Strategy: Uses a Free Block Bitmap to quickly identify and allocate available storage sectors