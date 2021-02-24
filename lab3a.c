// NAME: Aryaman Ladha, Akhil Vinta
// EMAIL: ladhaaryaman@ucla.edu, akhil.vinta@gmail.com
// ID: 805299802, 405288527
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

void exit_with_message(int x, char* message, int code){
    if(x < 0){
        fprintf(stderr, "%s Exiting with status %d.\n", message, code);
        exit(code);
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


void print_dirent_info(char* should_use, int inode, int index, unsigned int temp1, mode_t rec_len, unsigned char name_len, char* name){
    printf("%s,%d,%d,%d,%d,%d,'%s'\n", should_use, inode, index, temp1, rec_len, name_len, name);
}

void print_directory_entry(int inode, unsigned int data_block, int block_size, int fd){
    struct ext2_dir_entry entry;
    int data_block_location = 1024 + (data_block-1)*block_size;
    int index = 0;
    while(index < block_size){
        int x = pread(fd, &entry, sizeof(entry), data_block_location+index);
        exit_with_message(x,"Pread failed.", 2);
        if(entry.inode != 0){
            char file_name[EXT2_NAME_LEN+1];
            memcpy(file_name, entry.name, entry.name_len);
            file_name[entry.name_len] = 0; /* append null char to the file name */
            printf("DIRENT,%d,%d,%d,%d,%d,'%s'\n",
				inode, //parent inode number
				index, //logical byte offset
				entry.inode, //inode number of the referenced file
				entry.rec_len, //entry length
				entry.name_len, //name length
				entry.name //name, string, surrounded by single-quotes
			);
           //print_dirent_info("DIREN", inode, index, entry.inode, entry.rec_len, entry.name_len, entry.name);
        }
        index+= entry.rec_len;
    }
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


void print_indirect_1(int inode, unsigned int block, int block_size, int fd, char type, int depth){
    int block_location = 1024 + (block-1)*block_size;
    int * data_blocks = malloc(block_size);
    int num_entries = block_size/sizeof(int);
    int x = pread(fd, data_blocks, block_size, block_location);
    exit_with_message(x,"Pread failed.", 2);

    int offset;
    if(depth == 1) offset=12;
    else if(depth == 2) offset=num_entries+12;
    else if(depth == 3) offset=(num_entries*num_entries)+num_entries+12;
    else
        exit_with_message(-1, "Issue reading inode i_block array elements.", 2);

    for (int i = 0; i < num_entries; i++) {
		if (type == 'd') {
            if(data_blocks[i]!=0){
			    print_directory_entry(inode, data_blocks[i], block_size, fd);
            }
		}
        if(data_blocks[i]!=0){
            printf("INDIRECT,%d,%d,%d,%d,%d\n",
            inode, // I-node number of the owning file (decimal)
            1,     // (decimal) level of indirection for the block being scanned
            offset+i,  //logical block offset (decimal) represented by the referenced block.
            block, // block number of the (1, 2, 3) indirect block being scanned (decimal)
            data_blocks[i]// block number of the referenced block (decimal)
            );
        }
	}
    free(data_blocks);
}


void print_indirect_2(int inode, unsigned int block, int block_size, int fd, char type, int depth){
    int block_location = 1024 + (block-1)*block_size;
    int * data_blocks = malloc(block_size);
    int num_entries = block_size/sizeof(int);
    int x = pread(fd, data_blocks, block_size, block_location);
    exit_with_message(x,"Pread failed.", 2);
    //printf("block location = %d\n", block_location);
    int offset;
    if(depth == 2) offset=num_entries+12;
    else if(depth == 3) offset=(num_entries*num_entries)+num_entries+12;
    else
        exit_with_message(-1, "Wrong depth passed at some point in program.", 2);

    for(int i = 0; i < num_entries; i++){
        if(data_blocks[i] != 0){
            printf("INDIRECT,%d,%d,%d,%d,%d\n", inode, 2, offset+i, block, data_blocks[i]);
            print_indirect_1(inode, data_blocks[i], block_size, fd, type, depth);
        }
    }
}

void print_indirect_3(int inode, unsigned int block, int block_size, int fd, char type, int depth){
    int block_location = 1024 + (block-1)*block_size;
    int * data_blocks = malloc(block_size);
    int num_entries = block_size/sizeof(int);
    int x = pread(fd, data_blocks, block_size, block_location);
    exit_with_message(x,"Pread failed.", 2);
    //printf("block location = %d\n", block_location);
    for(int i = 0; i < num_entries; i++){
        if(data_blocks[i] != 0){
            printf("INDIRECT,%d,%d,%d,%d,%d\n", inode, 3, (num_entries*num_entries)+num_entries+12+i, block, data_blocks[i]);
        }
        print_indirect_2(inode, data_blocks[i], block_size, fd, type, depth);
    }
}




void read_inode(int index, int inode_table, int block_size, int fd){
    struct ext2_inode inode;
    int inode_data_location = 1024 + (inode_table-1)*block_size + (index-1)*sizeof(inode);
    int x = pread(fd, &inode, sizeof(inode), inode_data_location);
    exit_with_message(x,"Pread failed.", 2);
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

    //direct entries
	for (int i = 0; i < 12; i++) {
		if (type == 'd') {
            if(inode.i_block[i]!=0){
			    print_directory_entry(index, inode.i_block[i], block_size, fd);
            }
		}
	}

    //Indirect entry level 1
    if(inode.i_block[12]!=0)
        print_indirect_1(index,inode.i_block[12],block_size, fd, type, 1);
    if(inode.i_block[13]!=0)
        print_indirect_2(index,inode.i_block[13],block_size, fd, type, 2);
    if(inode.i_block[14]!=0)
        print_indirect_3(index,inode.i_block[14],block_size, fd, type, 3);


}

int main(int argc, char *argv[]){
    int fd = 0;
    errno = 0;
    unsigned int inodes_count = 0, blocks_count = 0;
    struct ext2_super_block super;
    if(argc > 2)
        exit_with_message(-1, "Parameter Error: Only valid parameters are executable and file system image file.", 1);
    if(argv[1]==NULL)
        exit_with_message(-1, "Parameter Error: Must specify file system image file for program to process.", 1);
    fd = open(argv[1], O_RDONLY);
    exit_with_message(fd, "Failed to open file.", 1);

    //Step1: Print out Super Block information. Always starts at byte 1024
    int x = pread(fd, &super, sizeof(super), 1024);
    exit_with_message(x,"Pread failed.", 2);
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
    exit_with_message(x,"Pread failed.", 2);
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
    exit_with_message(x,"Pread failed.", 2);
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
    exit_with_message(x,"Pread failed.", 2);
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