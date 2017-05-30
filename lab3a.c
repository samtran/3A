#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "ext2_fs.h"
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]){
  //check arg
  if(argc != 2){
   fprintf(stderr, "invalid argument. Required file");
    exit(1);
  }
  int fileFD = open(argv[1], O_RDONLY); //open file given                                                                                   
  if(fileFD == -1){
    fprintf(stderr, "Error opening file.");
    exit(1);
  }
  //Superblock parsing                                                                                                                               
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
  exit(0); 
}
