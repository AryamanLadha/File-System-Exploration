# File-System-Exploration

A program that scans an EXT2 file system image, and produces CSV summaries of what it finds.

## Compilation

To compile the lab3a program, run:

 ```bash
 make clean
 make
 ```

## Usage

```bash
./lab3a IMAGE
```

Where `IMAGE ` is a file system image file.

## Output

There are six types of output lines produced, each one summarizing a different part of the filesystem:

### SUPERBLOCK

A single new-line terminated line, comprised of the following comma-separated fields, summarizing the key file system parameters:

1. SUPERBLOCK
2. total number of blocks (decimal)
3. total number of inodes (decimal)
4. block size (in bytes, decimal)
5. inode size (in bytes, decimal)
6. blocks per group (decimal)
7. inodes per group (decimal)
8. first non-reserved inode (decimal)

### GROUP SUMMARY

A single new-line terminated line for each group in the filesystem, summarizing its contents:

1. GROUP
2. group number (decimal, starting from zero)
3. total number of blocks in this group (decimal)
4. total number of inodes in this group (decimal)
5. number of free blocks (decimal)
6. number of free inodes (decimal)
7. block number of free block bitmap for this group (decimal)
8. block number of the free inode bitmap for this group (decimal)
9. block number of the first block of inodes in this group (decimal)

Note: This program only supports filesystems with one group

### Free Block Entries

For each free block in the block bitmap of a group, produce a new-line terminated line with the following:

1. 'BFREE'
2. The index of the free block (in the block bitmap)

### Free Inode Entries

For each free inode in the inode bitmap of a group, produce a new-line terminated line with the following:

1. 'IFREE'
2. The index of the free inode (in the inode bitmap)

### Inode Summary

For each allocated inode in the inode bitmap of a group, produce a new-line terminated line with the following:

1. INODE
2. The index of the inode (in the inode bitmap)
3. file type ('f' for file, 'd' for directory, 's' for symbolic link, '?" for anything else)
4. mode
5. owner (decimal)
6. group (decimal)
7. link count (decimal)
8. time of last I-node change (mm/dd/yy hh:mm:ss, GMT)
9. modification time (mm/dd/yy hh:mm:ss, GMT)
10. time of last access (mm/dd/yy hh:mm:ss, GMT)
11. file size (decimal)
12. number of (512 byte) blocks of disk space (decimal) taken up by this file

The number of blocks (field 12) contains the same value as the i_blocks field of the I-node.

For ordinary files (type 'f') and directories (type 'd'), the next fifteen fields are block addresses (decimal, 12 direct, one indirect, one double indirect, one triple indirect)

### Indirect Block References

For each file or directory inode, scan the single indirect blocks, and recursively scan the double and triple indirect blocks.
For each non-zero block pointer found, produce a new-line terminated line with the following:

1. INDIRECT
2. I-node number of the owning file (decimal)
3. (decimal) level of indirection for the block being scanned ... 1 for single indirect, 2 for double indirect, 3 for triple
4. logical block offset (decimal) represented by the referenced block. This can be thought of as an index number in the list of all data blocks for that file. If the referenced block is a single- or double-indirect block, this is the same as the logical offset of the first data block to which it refers.
5. block number of the (1, 2, 3) indirect block being scanned (decimal) . . . not the highest level block (in the recursive scan), but the lower-level block that contains the block reference reported by this entry.
6. block number of the referenced block (decimal)

### Directory Entries

For each directory inode, scan every data block. For each valid directory entry, produce a new-line terminated line with the following:

1. DIRENT
2. parent inode number (decimal) ... the I-node number of the directory that contains this entry
3. logical byte offset (decimal) of this entry within the directory
4. inode number of the referenced file (decimal)
5. entry length (decimal)
6. name length (decimal)
7. name (string, surrounded by single quotes).
