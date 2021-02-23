#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "ext2_fs.h"
#include <time.h>
#include <errno.h>
#include <string.h>
void check(int x){
    if(x == -1){
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);
    }
}
int is_block_used(int bno, char * bitmap)
{
    int index = 0, offset = 0;
    if (bno == 0)
        return 1;
    index = (bno-1)/8; //which byte within the bitmap stores the info of this block#
    offset = (bno-1)%8;
    return ((bitmap[index] & (1 << offset)) );
}

void print_convert_time(time_t* time){
    struct tm *converted_time = gmtime(time);
    int month = converted_time->tm_mon+1;
    //printf("Month is: %d\n", month);
    int day = converted_time->tm_mday;
    int year = converted_time->tm_year;
    year = (1900 + year)%100;
    int hour = converted_time->tm_hour;
    int minute = converted_time->tm_min;
    int second = converted_time->tm_sec;
    printf("%02d/%02d/%02d %02d:%02d:%02d,", month,day,year,hour,minute,second); 
    // 08/07/17 17:58:47

}
void read_inode(int index , int inode_table, int block_size, int fd){
    struct ext2_inode inode;
    int inode_data_location = 1024 + (inode_table-1)*block_size + (index-1)*sizeof(inode);
    pread(fd, &inode, sizeof(inode), inode_data_location);
    if(inode.i_mode == 0 || inode.i_links_count == 0)
        return;
    char type = '?';
    if(S_ISDIR(inode.i_mode))
        type = 'd';
    else if(S_ISREG(inode.i_mode))
        type = 'f';
    else if(S_ISLNK(inode.i_mode))
        type = 's';
    int mode = inode.i_mode & 0xFFF;
    printf("INODE,%d,%c,%o,%d,%d,%d,",index,type,mode,inode.i_uid, inode.i_gid, inode.i_links_count);
    time_t c = inode.i_ctime;
    print_convert_time(&c);
    time_t m = inode.i_mtime;
    print_convert_time(&m);
    time_t a = inode.i_atime;
    print_convert_time(&a);
    printf("%d,%d", inode.i_size, inode.i_blocks); //TODO Add one more field here
    if(!((type =='s') && (inode.i_size < 60))){
        printf(",");
        for(unsigned int i = 0; i<15; i++)
            if(i==14)
                printf("%d",inode.i_block[i]);
            else
            printf("%d,",inode.i_block[i]);
    }
    printf("\n");

}
int main(){
    int fd = 0;
    errno = 0;
    unsigned int inodes_count = 0, blocks_count = 0;
    struct ext2_super_block super;
    fd = open("trivial.img", O_RDONLY);


    //Step1: Print out Super Block information. Always starts at byte 1024
    int x = pread(fd, &super, sizeof(super), 1024);
    check(x);
    inodes_count = super.s_inodes_count;
    blocks_count = super.s_blocks_count;
    int block_size = 1024 << super.s_log_block_size; /* calculate block size in bytes */
    int inode_size = super.s_inode_size;
    int first_inode = super.s_first_ino;
    int blocks_per_group = super.s_blocks_per_group;
    int inodes_per_group = super.s_inodes_per_group;
    int num_groups = blocks_count/blocks_per_group;
    if(num_groups == 0)
        num_groups = 1;
    printf("SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n",
        blocks_count,
        inodes_count,
        block_size,
        inode_size,
        blocks_per_group,
        inodes_per_group,
        first_inode
    );


    //Step 2: Print out Group Block Description. Variable start length depending on block size
    int group_block_desc_location = 2*block_size;
    if(block_size > 1024)
        group_block_desc_location  = 1*block_size;
    struct ext2_group_desc descriptor;
    x = pread(fd, &descriptor, sizeof(descriptor), group_block_desc_location);
    check(x);
    int free_blocks = descriptor.bg_free_blocks_count;
    int free_inodes = descriptor.bg_free_inodes_count;
    int block_bitmap = descriptor.bg_block_bitmap;
    int inode_bitmap = descriptor.bg_inode_bitmap;
    int first_inode_block = descriptor.bg_inode_table;
    printf("GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n",0,
        blocks_count,
        inodes_count,
        free_blocks,
        free_inodes,
        block_bitmap,
        inode_bitmap,
        first_inode_block
    );

    //Step 3: Free block entries
    int block_bitmap_location = 1024 + (block_bitmap-1)*block_size;
    char*bitmap = malloc(block_size);
    x = pread(fd, bitmap, block_size, block_bitmap_location);
    check(x);
    for(unsigned int i = 1; i<=blocks_count; i++){
        if(!is_block_used(i,bitmap)){
            printf("BFREE,%d\n",i);
        }
    }
    free(bitmap);

    //Step 4: Free inode entries
    int inode_bitmap_location = 1024 + (inode_bitmap-1)*block_size;
    bitmap = malloc(block_size);
    x = pread(fd, bitmap, block_size, inode_bitmap_location);
    check(x);
    for(unsigned int i = 1; i <= inodes_count; i++){
        if(!is_block_used(i,bitmap)){
            printf("IFREE,%d\n",i);
        }
        else{
            read_inode(i,descriptor.bg_inode_table, block_size, fd);
        }
    }
    free(bitmap);
    exit(0);
}