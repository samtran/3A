#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "ext2_fs.h"
#include <unistd.h>
#include <string.h>


//create struct to hold group info
struct my_group{
  int blocks_per_group;
  int num_free_blocks;
  int num_free_inodes;
  int free_block_bitmap;
  int free_inode_bitmap;
  int first_inode;
};
int main(int argc, char *argv[]){
  //check arg
  if(argc != 2){
   fprintf(stderr, "invalid argument. Required file");
    exit(1);
  }
  int fileFD = open(argv[1], O_RDONLY); //open file given                                                                                   
  if(fileFD == -1){
    fprintf(stderr, "Error opening file.");
    exit(2);
  }
  //SUPERBLOCK PARSING                                                                                                                               
  //get block count                                                                                                                                  
  int block_count;
  pread(fileFD, &block_count, 4, 1024 + 4);
  //get inodes count                                                                                                                                
  int inodes_count;
  pread(fileFD, &inodes_count, 4, 1024);
  //get block size manually                                                                                                                         
  int block_size;
  pread(fileFD, &block_size, 4, 1024 + 24);
  block_size = 1024 << block_size;
  //get inode_size                                                                                                                                  
  int inode_size;
  pread(fileFD, &inode_size, 2, 1024 + 88);
  //block_per_group                                                                                                                                 
  int block_per_group;
  pread(fileFD, &block_per_group, 4, 1024 + 32);
  //inodes_per_group                                                                                                                                 
  int inodes_per_group;
  pread(fileFD, &inodes_per_group, 4, 1024 + 40);
  int first_inode;
  pread(fileFD, &first_inode, 4, 1024 + 84);
  //Write CSV for superblock                                                                                                                         
  fprintf(stdout, "SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n", block_count, inodes_count, block_size, inode_size, block_per_group, inodes_per_group, first_inode);
  
  //GROUP PARSING
  //calculate total number of groups by: total blocks/blocks per group
  int tot_groups = block_count/block_per_group;
  struct my_group *group = malloc(tot_groups * sizeof(struct my_group));
  int i = 0;
  for( i = 0; i <= tot_groups; i++){
    if(tot_groups != 0 && i == tot_groups)
      break;
    int groupOffset = 1024 + 1024; //offset from superblock and size of superblock
    groupOffset = groupOffset + (i * 32);//32 is the size of each group
    //get free blocks
    int freeblocks;
    pread(fileFD, &freeblocks, 2, groupOffset + 12);
    group[i].num_free_blocks = freeblocks;
    //get free inodes
    int freeinodes;
    pread(fileFD, &freeinodes, 2, groupOffset + 14);
    group[i].num_free_inodes = freeinodes;
    //get block num of free block bitmap
    int freeBlockBitmap;
    pread(fileFD, &freeBlockBitmap, 4, groupOffset);
    group[i].free_block_bitmap = freeBlockBitmap;
    //get free inode bitmap 
    int freeInodeBitmap;
    pread(fileFD, &freeInodeBitmap, 4, groupOffset + 4);
    group[i].free_inode_bitmap = freeInodeBitmap;
    //get first block of inodes
    int firstInode;
    pread(fileFD, &firstInode, 4, groupOffset + 8);
    group[i].first_inode = firstInode;;
    //Write CSV for group                                                                                                                        
    //if its not the last group, its constant blocks_per_group from super block
    if(i == (tot_groups -1)){
      group[i].blocks_per_group = block_count;
      fprintf(stdout, "GROUP,%i,%d,%d,%d,%d,%d,%d,%d\n", i, block_count, inodes_per_group, freeblocks, freeinodes, freeBlockBitmap, freeInodeBitmap, firstInode);      
    }
    else{
      group[i].blocks_per_group = block_count% block_per_group;
      fprintf(stdout, "GROUP,%i,%d,%d,%d,%d,%d,%d,%d\n", i, group[i].blocks_per_group, inodes_per_group, freeblocks, freeinodes, freeBlockBitmap, freeInodeBitmap, firstInode);
    }      
  }
  
  //declare variables for BFREE AND IFREE
  int val;
  int j;
  int l;
  int bitmask;
  int isFree;
  //BFREE parsing (free block entries)
  for(i = 0; i <= tot_groups; i++){//for each group, analyze free bitmap
    if(tot_groups != 0 && i == tot_groups)
      break;
    int j = 0;
    for(j = 0; j < block_size; j++){ //check free or not free status of each block using this bitmap
      pread(fileFD, &isFree, 1, (block_size*group[i].free_block_bitmap)+j);
      //go through each bit of the byte
      bitmask = 1; //to get 1 bit at a time
      for(l = 0; l < 8; l++){
	val = isFree & bitmask;
	if(val == 0)//0 means free, so do output
	  fprintf(stdout, "BFREE,%d\n", (j*8) + l + (i*block_per_group) + 1);
        bitmask = bitmask << 1; //shift mask to get the next bit
      }
     }
  }

  //IFREE parsing (free inode entries)
  for(i = 0; i <=tot_groups; i++){//each group contains a inode bitmap
    if(tot_groups != 0 && i == tot_groups)
      break;
    for(j = 0; j<block_size; j++){
      pread(fileFD, &isFree, 1, (block_size*group[i].free_inode_bitmap)+j);
      //go through each bit of the byte
      bitmask = 1;
      for(l = 0; l<8; l++){
	val = isFree & bitmask;
	if(val == 0)//0 means free so output
	  fprintf(stdout, "IFREE,%d\n",(j*8 + l + (i*block_per_group) + 1));
        bitmask = bitmask << 1;
      }
    }
  }
  exit(0); 
}
