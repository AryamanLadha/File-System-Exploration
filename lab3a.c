#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "ext2_fs.h"
int main(){
    int fd = 0;
    unsigned int inodes_count = 0, blocks_count = 0; //log_block_size = 0;
    struct ext2_super_block super;
    fd = open("EXT2_test.img", O_RDONLY);
    //Note: Need to check the return value of pread to make sure it reads the
    //specified size.
    int x = pread(fd, &super, sizeof(super), 1024);
    if(x == -1){
        fprintf(stderr, "Pread failed\n");
        exit(1);
    }
    inodes_count = super.s_inodes_count;
    blocks_count = super.s_blocks_count;
    int block_size = 1024 << super.s_log_block_size; /* calculate block size in bytes */
    int inode_size = super.s_inode_size;
    int first_inode = super.s_first_ino;
    int blocks_per_group = super.s_blocks_per_group;
    int inodes_per_group = super.s_inodes_per_group;
    char*field = "SUPERBLOCK";
    int num_groups = blocks_count/blocks_per_group;
    if(num_groups == 0)
        num_groups = 1;
    printf("Number of groups is: %d\n", num_groups);
    printf("%s,%d,%d,%d,%d,%d,%d,%d\n",field,blocks_count,inodes_count,block_size,inode_size,blocks_per_group,
    inodes_per_group,first_inode);
    int group_block_desc_location = 2048;
    if(block_size > 1024)
        group_block_desc_location  = 1024;
    struct ext2_group_desc descriptor;
    pread(fd, &descriptor, sizeof(descriptor), group_block_desc_location);
    int blocks = blocks_count;
    char*field2 = "GROUP";
    int group_number = 0;
    int inodes = inodes_count;
    int free_blocks = descriptor.bg_free_blocks_count;
    int free_inodes = descriptor.bg_free_inodes_count;
    int block_bitmap = descriptor.bg_block_bitmap;
    int inode_bitmap = descriptor.bg_inode_bitmap;
    int first_inode_block = descriptor.bg_inode_table;
    printf("%s,%d,%d,%d,%d,%d,%d,%d,%d\n",field2,group_number,blocks,inodes,free_blocks,free_inodes,block_bitmap,
    inode_bitmap,first_inode_block);
    exit(0);
}