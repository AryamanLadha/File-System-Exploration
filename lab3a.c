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
    printf("%s,%d,%d,%d,%d,%d,%d,%d\n",field,blocks_count,inodes_count,block_size,inode_size,blocks_per_group,inodes_per_group,first_inode);
    exit(0);
}