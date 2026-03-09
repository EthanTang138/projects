/*
 *------------
 * This code is provided solely for the personal and private use of
 * students taking the CSC369H5 course at the University of Toronto.
 * Copying for purposes other than this use is expressly prohibited.
 * All forms of distribution of this code, whether as given or with
 * any changes, are expressly prohibited.
 *
 * All of the files in this directory and all subdirectories are:
 * Copyright (c) 2024 MCS @ UTM
 * -------------
 */

#ifndef CSC369_E2FS_H
#define CSC369_E2FS_H

#include "ext2.h"
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>


extern unsigned char *disk;
extern struct ext2_super_block *sb;
extern struct ext2_group_desc *gd;

/**
 * TODO: add in here prototypes for any helpers you might need.
 * Implement the helpers in e2fs.c
 */

unsigned char *get_block_bits(void);
unsigned char *get_inode_bits(void);
struct ext2_inode *get_inode_table(void);
struct ext2_dir_entry *get_file_dir(int inode_num, char *name);
struct ext2_dir_entry *get_prev_dir_entry(int inode_num, char *name);
char *get_name(const char *path);
int get_second_last_inode(const char *path);
int get_last_inode(const char *path);

int traverse_path(const char *path);

int new_inode(char type, unsigned int size);
int next_available_inode();
int next_available_block();
int new_inode(char type, unsigned int size);
int add_dir_entry(int inode_under, int inode_point, int entry_size, const char *path, char type);
struct ext2_inode *get_inode_from_num(int inode_num);

void remove_dir_entry(int parent_ino_num, const char *path);
void free_inode(int inode_num);
void free_block(int block_num);

int update_directory_entry(const char *dest_path, int inode_index);
int traverse_path_check(const char *path);
int max(int a, int b);

#define DEBUGPRINT 0

// print helpers based on DEBUGPRINT flag
void debug_printf(const char * format, ...);
void debug_print_int_to_bin( const char * str, unsigned int n);
char *int_to_binary(unsigned int n);

#endif

