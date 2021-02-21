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
    printf("Inodes: %d\nBlocks: %d\nBlock Size: %d\n", inodes_count,blocks_count,block_size);
    exit(0);
}