#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "ext2_fs.h"
#include <unistd.h>
#include <string.h>

// Code to ask Sam about is denoted with "ASK SAM"
// Code to ask Apurva about is denoted with "ASK APURVA"

#define SUPERBLOCK_OFFSET 1024
#define GROUP_OFFSET SUPERBLOCK_OFFSET + 1024
#define GROUP_SIZE 32
// Number of groups in file system
__u16 n_groups;
int fileFD;

struct ext2_super_block* superblock;
struct ext2_group_desc* groups;
struct ext2_inode* inodes;

void p_superblock() {
  // Superblock is located at a byte offset 1024 from the beginning of the file
  superblock = malloc(sizeof(struct ext2_super_block));
  // total number of blocks
  pread(fileFD, &(superblock->s_blocks_count), 4, SUPERBLOCK_OFFSET + 4);
  // total number of inodes
  pread(fileFD, &(superblock->s_inodes_count), 4, SUPERBLOCK_OFFSET);
  // block size
  __u32 block_size;
  pread(fileFD, &block_size, 4, SUPERBLOCK_OFFSET + 24);
  superblock->s_log_block_size = SUPERBLOCK_OFFSET << block_size;
  // inode size
  pread(fileFD, &(superblock->s_inode_size), 2, SUPERBLOCK_OFFSET + 88);
  // blocks per group
  pread(fileFD, &(superblock->s_blocks_per_group), 4, SUPERBLOCK_OFFSET + 32);
  // inodes per group
  pread(fileFD, &(superblock->s_inodes_per_group), 4, SUPERBLOCK_OFFSET + 40);
  // first non-reserved inodes
  pread(fileFD, &(superblock->s_first_ino), 4, SUPERBLOCK_OFFSET + 84);
  // Now print to stdout
  fprintf(stdout, "SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n", superblock->s_blocks_count,
  	  superblock->s_inodes_count, superblock->s_log_block_size, superblock->s_inode_size, 
  	  superblock->s_blocks_per_group, superblock->s_inodes_per_group, superblock->s_first_ino);
}

void p_group() {
  n_groups = superblock->s_blocks_count / superblock->s_blocks_per_group;
  fprintf(stderr, "N groups: %d\n", n_groups);
  groups = malloc(sizeof(struct ext2_group_desc) * n_groups);
  for (int i = 0; i <= n_groups; i++) {	
	if (n_groups != 0 && i == n_groups)
	  break;
	// group number
  	__u32 group_number = i;
	// total number of free blocks in group
	pread(fileFD, &groups[i].bg_free_blocks_count, 2, GROUP_OFFSET + (i * GROUP_SIZE) + 12);
	// total number of free inodes in group
	pread(fileFD, &groups[i].bg_free_inodes_count, 2, GROUP_OFFSET + (i * GROUP_SIZE) + 14);
	// block number of free block bitmap for group
	pread(fileFD, &groups[i].bg_block_bitmap, 4, GROUP_OFFSET + (i * GROUP_SIZE));
	// block number of free inode bitmap for group
	pread(fileFD, &groups[i].bg_inode_bitmap, 4, GROUP_OFFSET + (i * GROUP_SIZE) + 4);
	// block number of first block of inodes for group
	pread(fileFD, &groups[i].bg_inode_table, 4, GROUP_OFFSET + (i * GROUP_SIZE) + 8);
	// If it is not in the last group
	fprintf(stdout, "GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n", group_number, superblock->s_blocks_count,
		superblock->s_inodes_per_group, groups[i].bg_free_blocks_count, groups[i].bg_free_inodes_count, 
		groups[i].bg_block_bitmap, groups[i].bg_inode_bitmap,
		groups[i].bg_inode_table);
  }
}

void p_bfree() { 
  int bitmask;
  int val;
  int isFree;
  for (int i = 0; i <= n_groups; i++) {
  	if (n_groups != 0 && i == n_groups)
  	  break;
  	for (int j = 0; j < superblock->s_log_block_size; j++) {
  	  pread(fileFD, &isFree, 1, (superblock->s_log_block_size * groups[i].bg_block_bitmap) + j);
  	  bitmask = 1;
  	  for (int k = 0; k < 8; k++) {
  	  	val = isFree & bitmask;
  	  	if (val == 0) {
  	  	  fprintf(stdout, "BFREE,%d\n", (j*8) + k + (i*superblock->s_blocks_per_group) + 1);
  	  	}
  	  	bitmask = bitmask << 1;
  	  }
  	}
  }
}

void p_ifree() {
  int bitmask;
  int val;
  int isFree;
  for (int i = 0; i <= n_groups; i++) {
  	if (n_groups != 0 && i == n_groups)
  	  break;
  	for (int j = 0; j < superblock->s_log_block_size; j++) {
  	  pread(fileFD, &isFree, 1, (superblock->s_log_block_size * groups[i].bg_inode_bitmap) + j);
  	  bitmask = 1;
  	  for (int k = 0; k < 8; k++) {
  	  	val = isFree & bitmask;
  	  	if (val == 0)
  	  	  fprintf(stdout, "IFREE,%d\n", (j*8) + k + (i*superblock->s_inodes_per_group) + 1);
  	  	bitmask = bitmask << 1;
  	  }
  	}
  }
}

void p_inode() {
  inodes = malloc(sizeof(struct ext2_inode) * superblock->s_inodes_count);
}

void cleanup() {
  free(superblock);
  free(groups);
  free(inodes);
}

/*
struct Inode {
  char file_type;
  uint16_t mode, owner, group, link_count;
  uint32_t number, ctime, mtime, atime, size, n_blocks;
  uint32_t bp[15];
};

Struct Inode* inodes;
void print_inode() {
  int modulo;
  int offset;
  for (int i = 0; i < tot_groups; i++) {
  	for (int j = 0; j < inodes_per_group; j++) {
  	  int k = i * inodes_per_group + j;
  	  modulo = (inode[k].number - 1) % inodes_per_group;
  	  offset = group[i].inode_table * block_size;
  	  int final_offset = offset + modulo + inode_size;
  	  pread(fileFD, &inodes[k].mode, 2, final_offset + 0);

  	  // File type
  	  if ((inodes[k].mode & 0x8000) && (inodes[k].mode & 0x2000)) {
  	  	inodes[k].file_type = 's';
  	  } else if (inodes[k].mode & 0x8000) {
  	  	inodes[k].file_type = 'f';
  	  } else if (inodes[k].mode & 0x4000) {
  	  	// We have to do directory stuff here
  	  	inodes[k].file_type = 'd';
  	  } else {
  	  	inodes[k].file_type = '?';
  	  }

  	  // Owner
  	  pread(fileFD, &inodes[k].owner, 2, final_offset + 2);
  	  // Group
  	  &inodes[k].group = i;
  	  // Link count
  	  pread(fileFD, &inodes[k].link_count, 2, final_offset + 26);
	  // Creation Time
	  pread(fileFD, &inodes[k].ctime, 4, final_offset + 12);
	  // Modification Time
	  pread(fileFD, &inodes[k].mtime, 4, final_offset + 16);
	  // Access time
	  pread(fileFD, &inodes[k].atime, 4, final_offset + 8);
	  // File size
	  pread(fileFD, &inodes[k].size, 4, final_offset + 4);
	  // N Blocks
	  pread(fileFD, &inodes[k].n_blocks, 4, final_offset + 28);
	  // Print it all out
	  
	  // Block pointers
	  int blockpointer_offset = 40;
	  for (int p = 0; p < 15; p++) {
	  	pread(fileFD, &inodes[k].bp[p], 4, final_offset + blockpointer_offset);
	  	// Print the block pointer
	  	if (inodes[k].bp[p] != 0 && p == 12) {
	  	  // Indirect
	  	}
	  	blockpointer_offset += 4;
	  }
	}
  }
}
*/

int main(int argc, char *argv[]){
  // Check arguments
  if(argc != 2){
   fprintf(stderr, "Invalid argument. Required file...exiting!\n");
    exit(1);
  }
  fileFD = open(argv[1], O_RDONLY); //open file given                                                                                   
  if (fileFD == -1){
    fprintf(stderr, "Error opening file...exiting!\n");
    exit(2);
  }
  p_superblock();
  p_group();
  p_bfree();
  p_ifree();
  p_inode();

  cleanup();
  exit(0); 
}
