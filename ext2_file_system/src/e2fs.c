/*
 *------------
 * This code is provided solely for the personal and private use of
 * students taking the CSC369H5 course at the University of Toronto.
 * Copying for purposes other than this use is expressly prohibited.
 * All forms of distribution of this code, whether as given or with
 * any changes, are expressly prohibited.
 *
 * All of the files in this directory and all subdirectories are:
 * Copyright (c) 2022 MCS @ UTM
 * -------------
 */

/**
 * TODO: Make sure to add all necessary includes here
 */

#include "e2fs.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

 /**
  * TODO: Add any helper implementations here
  */
void debug_print_dir_entry(struct ext2_dir_entry *entry);

unsigned char *get_block_bits() {
    return (unsigned char *)(disk + EXT2_BLOCK_SIZE * gd->bg_block_bitmap);
}

unsigned char *get_inode_bits() {
    return (unsigned char *)(disk + EXT2_BLOCK_SIZE * gd->bg_inode_bitmap);
}

struct ext2_inode *get_inode_table() {
    return (struct ext2_inode *)(disk + EXT2_BLOCK_SIZE * gd->bg_inode_table);
}



// finds the next available block in the block bitmap, sets it to 1, and returns the inode number
// returns -1 on failure
int next_available_inode() {
    char *inode_bits = (char *)get_inode_bits();
    int inode_num = 0;
    for (int byte=0; byte<sb->s_inodes_count/8; byte++) {    // divide by 8 since each byte is 8 bits
        for (int bit=0; bit<8; bit++) {
            inode_num++;

            if (!(inode_bits[byte] & (1 << bit))) {
                // printf("found inode bitmap space\n");
                inode_bits[byte] |= (1 << bit);
                return inode_num;
            }
        }
    }
    return -1;
}

// finds the next available block in the block bitmap, sets it to 1, and returns the block number
// returns -1 on failure
int next_available_block() {
    char *block_bits = (char *)get_block_bits();
    int block_num = 0;
    for (int byte=0; byte<sb->s_blocks_count/8; byte++) {    // divide by 8 since each byte is 8 bits
        for (int bit=0; bit<8; bit++) {
            block_num++;

            if (!(block_bits[byte] & (1 << bit))) {
                
                sb->s_free_blocks_count--;
                gd->bg_free_blocks_count--;

                block_bits[byte] |= (1 << bit);
                return block_num;

            }
        }
    }
    return -1;
}


int new_inode(char type, unsigned int size) {
    int inode_num = next_available_inode();
    struct ext2_inode *inode_table = get_inode_table();
    struct ext2_inode *new_inode = &inode_table[inode_num - 1];   // subtract 1 since inode numbers start from 1
    
    memset(new_inode, 0, sizeof(struct ext2_inode));
    if ('d' == type) {
        new_inode->i_mode = EXT2_S_IFDIR;   // set the directory bits of i_mode
    } else if ('f' == type) {
         new_inode->i_mode = EXT2_S_IFREG;   // set the file bits of i_mode
    } else {
        new_inode->i_mode = EXT2_S_IFLNK;
    }
    new_inode->i_size = size;    // 1024 for directories
    new_inode->i_links_count = 0;
    new_inode->i_blocks = 0;

    sb->s_free_inodes_count--;
    gd->bg_free_inodes_count--;

    return inode_num;
}

void init_dir_entry(struct ext2_dir_entry *new_dir, unsigned int inode, unsigned short rec_len, unsigned char type, char *name) {
    get_inode_from_num(inode)->i_links_count++;

    new_dir->inode = inode;
    new_dir->name_len = strlen(name);
    if (type == 'd') {
        new_dir->file_type |= EXT2_FT_DIR;
    } else if (type == 'f') {
        new_dir->file_type |= EXT2_FT_REG_FILE;
    } else {
        new_dir->file_type |= EXT2_FT_SYMLINK;
    }
    
    debug_printf("adding name %s\n", name);
    strncpy(new_dir->name, name, strlen(name));     // removes the null byte
    new_dir->rec_len = rec_len;
}

// creates a dir_entry in the blocks of inode_under and sets the the dir_entry pointing to inode_point
// takes entry_size, path, and type to populate the new dir_entry
int add_dir_entry(int inode_under, int inode_point, int entry_size, const char *path, char type) {
    struct ext2_inode *inode_table = get_inode_table();
    struct ext2_inode *inode = &inode_table[inode_under - 1];
    char *name = get_name(path);

    for (int i=0; i<12; i++) {  // Only 12 direct blocks in i_block
        // printf("inode_iblock[i] %d\n", inode->i_block[i]);
        if (inode->i_block[i] == 0) {
            int free_block_num = next_available_block();
            debug_printf("free_block_num %d\n", free_block_num);
            struct ext2_dir_entry *new_dir = (struct ext2_dir_entry *)(disk + EXT2_BLOCK_SIZE*free_block_num);
            memset(new_dir, 0, sizeof(struct ext2_dir_entry));

            init_dir_entry(new_dir, inode_point, EXT2_BLOCK_SIZE, type, name);

            inode->i_block[i] = free_block_num;
            debug_printf("inode_iblock[i] IN %d\n", inode->i_block[i]);

            inode->i_blocks += 2;
            return 0;

        } else if ((i + 1 < 12) && inode->i_block[i+1] == 0) {   // check if there is space to add onto a block
        // get the last element in the block, check whether there is space to insert this entry
        // if there is space, modify the last element's rec_len and insert the new entry
        
            // calculate the rec_len of the new dir_entry
            int new_rec_len = sizeof(struct ext2_dir_entry) + strlen(name);
            new_rec_len = new_rec_len - (new_rec_len % 4) + 4;

            debug_printf("new rec len size %d\n", new_rec_len);

            int count = 0;
            struct ext2_dir_entry *entry = (struct ext2_dir_entry *)(disk + EXT2_BLOCK_SIZE*inode->i_block[i]);

            while (count != 1024 && entry->rec_len != 0) {
                
                debug_printf("------------\n");
                debug_print_dir_entry(entry);
                // find the last entry
                if (count + entry->rec_len >= EXT2_BLOCK_SIZE) {

                    // calculate the rec_len of the last entry to see if there is space
                    // for a new entry
                    unsigned short entry_size = sizeof(struct ext2_dir_entry) + entry->name_len;
                    entry_size = entry_size - (entry_size % 4) + 4;   
                    debug_printf("entry size: %d\n", entry_size);

                    if (entry->rec_len - entry_size + new_rec_len <= EXT2_BLOCK_SIZE) {
                        debug_printf("add to end, entry+entry_size %p\n", (entry+entry_size));

                        struct ext2_dir_entry *new_dir = (struct ext2_dir_entry *)(((char*) entry)+ entry_size);
                        memset(new_dir, 0, sizeof(struct ext2_dir_entry));
                        init_dir_entry(new_dir, inode_point, entry->rec_len - entry_size, type, name);

                        entry->rec_len = entry_size;
                        return 0;
                    }
                }
                count += entry->rec_len;
                entry = (struct ext2_dir_entry *)(((char*) entry)+ entry->rec_len);
            }
           
        }
    }
    return 0;
}

// checks if a given path is valid (all directories) and returns 0 if valid, returns ENOENT if not.
int traverse_path(const char *path) {
    char path_cpy[strlen(path) + 1];
    strcpy(path_cpy, path);
    char *file_name = strtok(path_cpy, "/");     //since strtok modifies the string, use a copy

    if (path_cpy[0] != '/') {  // check if the path is an absolute path
        return EINVAL;
    }

     int inode_num = 2;
    // printf("inode_num %d\n", inode_num);
    while (file_name != NULL) {
        
        if (strcmp(file_name, "") == 0) {
            file_name = strtok(NULL, "/");
            continue;
        }
        struct ext2_dir_entry *dir_entry = get_file_dir(inode_num, file_name);
        if (dir_entry == NULL) {
            return ENOENT;
        }
        inode_num = dir_entry->inode;
        // printf("inode_num %d\n", inode_num);
        file_name = strtok(NULL, "/");
    }

    return 0;
}

int traverse_path_check(const char *path) {
    if (path[0] != '/') {
        debug_printf("Path must be absolute\n");
        return -1;
    }

    // Start at the root directory
    struct ext2_inode *inode_table = get_inode_table();
    struct ext2_inode *inode = &inode_table[EXT2_ROOT_INO - 1]; // Root inode

    char *token, *path_copy;
    path_copy = strdup(path); // Make a copy of the path to tokenize
    token = strtok(path_copy, "/");

    while (token)
    {
        debug_printf("token=%s\n", token);
        if (!(inode->i_mode & EXT2_S_IFDIR))
        {
            debug_printf("Not a directory: %s\n", token);
            free(path_copy);
            return -1;
        }

        // Read the directory entries in the current inode
        int found = 0;
        for (int i = 0; i < inode->i_blocks / (EXT2_BLOCK_SIZE / 512); i++)
        {
            struct ext2_dir_entry *entry = (struct ext2_dir_entry *)(disk + inode->i_block[i] * EXT2_BLOCK_SIZE);
            while ((char *)entry < (char *)(disk + (inode->i_block[i] + 1) * EXT2_BLOCK_SIZE))
            {
                debug_printf("------ i=%i\n", i);
                debug_printf("inode->i_block[i]=%i\n", inode->i_block[i]);
                debug_print_dir_entry(entry);
                if (strncmp(entry->name, token, entry->name_len) == 0 && strlen(token) == entry->name_len)
                {
                    inode = &inode_table[entry->inode - 1];
                    found = 1;
                    break;
                }
                entry = (void *)entry + entry->rec_len; // Move to the next entry
            }
            if (found)
                break;
        }

        if (!found)
        {
            // fprintf(stderr, "File or directory not found: %s\n", token);
            free(path_copy);
            return -2;
        }

        token = strtok(NULL, "/"); // Move to the next component
    }

    free(path_copy);
    debug_printf("inode->i_mode=0x%X\n", inode->i_mode);
    debug_printf("inode isDir=%i\n", inode->i_mode & EXT2_S_IFDIR);
    return inode - inode_table + 1; // Return the inode number
}


// finds the directory entry of a specified file name in the files of an inode directory
struct ext2_dir_entry *get_file_dir(int inode_num, char *name) {
    struct ext2_inode *inode_table = get_inode_table();
    struct ext2_inode *inode = &inode_table[inode_num - 1];

    int count = 0;
    int i_block_index = 0;
    while (inode->i_block[i_block_index] != 0) {
        struct ext2_dir_entry *entry = (struct ext2_dir_entry *)(disk + EXT2_BLOCK_SIZE*inode->i_block[i_block_index]);

            while (count != 1024 && entry->rec_len != 0) {
                
                char entry_name_cpy[entry->name_len + 1];
                strncpy(entry_name_cpy, entry->name, entry->name_len);
                entry_name_cpy[entry->name_len] = '\0';
                debug_printf("entry name %s\n", entry_name_cpy);
                

                if (strcmp(entry_name_cpy, name) == 0) {
                    // printf("name after %s\n", name);
                    debug_printf("entry inode %d\n", entry->inode);
                    return entry;
                }
                count += entry->rec_len;
                entry = (struct ext2_dir_entry *)(((char*) entry)+ entry->rec_len);
            }
        count = 0;
        i_block_index++;
    }

    return NULL;
}

// finds the previous directory entry of a specified directory entry in the entries of an inode directory
struct ext2_dir_entry *get_prev_dir_entry(int inode_num, char *name) {
    struct ext2_inode *inode_table = get_inode_table();
    struct ext2_inode *inode = &inode_table[inode_num - 1];

    struct ext2_dir_entry *prev_entry = NULL;
    int count = 0;
    int i_block_index = 0;
    while (inode->i_block[i_block_index] != 0) {
        struct ext2_dir_entry *entry = (struct ext2_dir_entry *)(disk + EXT2_BLOCK_SIZE*inode->i_block[i_block_index]);

            while (count != 1024 && entry->rec_len != 0) {
                
                char entry_name_cpy[entry->name_len + 1];
                strncpy(entry_name_cpy, entry->name, entry->name_len);
                entry_name_cpy[entry->name_len] = '\0';
                debug_printf("entry name %s\n", entry_name_cpy);
                

                if (strcmp(entry_name_cpy, name) == 0) {

                    //checks if prev is the first entry in the block
                    if (prev_entry == NULL) {
                        return NULL;
                    }
                    return prev_entry;
                }
                count += entry->rec_len;
                prev_entry = entry;
                entry = (struct ext2_dir_entry *)(((char*) entry)+ entry->rec_len);
            }
        count = 0;
        prev_entry = NULL;
        i_block_index++;
    }

    return NULL;
}

// returns the number of the second_last element in path.
// path must be a valid path.
int get_second_last_inode(const char *path) {
     int inode_num = 2;

    char path_cpy[strlen(path) + 1];
    strcpy(path_cpy, path);
    char *file_name = strtok(path_cpy, "/");     //since strtok modifies the string, use a copy

    while (file_name != NULL) {
        
        if (strcmp(file_name, "") == 0) {
            file_name = strtok(NULL, "/");
            continue;
        }
        struct ext2_dir_entry *dir_entry = get_file_dir(inode_num, file_name);

        file_name = strtok(NULL, "/");
        if (file_name != NULL) {
            inode_num = dir_entry->inode;
        }
        
    }
    return inode_num;
}

// returns the number of the last element in path.
// path must be a valid path.
int get_last_inode(const char *path) {
     int inode_num = 2;

    char path_cpy[strlen(path) + 1];
    strcpy(path_cpy, path);
    char *file_name = strtok(path_cpy, "/");     //since strtok modifies the string, use a copy

    while (file_name != NULL) {
        
        if (strcmp(file_name, "") == 0) {
            file_name = strtok(NULL, "/");
            continue;
        }
        struct ext2_dir_entry *dir_entry = get_file_dir(inode_num, file_name);

        file_name = strtok(NULL, "/");
        inode_num = dir_entry->inode;
        
    }
    return inode_num;
}

// returns the pointer to the inode referenced by it's inode num
struct ext2_inode *get_inode_from_num(int inode_num) {
    struct ext2_inode *inode_table = get_inode_table();
    return &inode_table[inode_num - 1];
}

// returns the name of the last element of the path
char *get_name(const char *path) {
    const char *last_slash = strrchr(path, '/');

    if (last_slash == NULL) {
        return (char *)path;
    }
    
    return (char *)(last_slash + 1);
}

// frees an inode and all its associated blocks
void free_inode(int inode_num) {
    unsigned char *inode_bits = get_inode_bits();
    struct ext2_inode *inode = get_inode_from_num(inode_num);

    // set the corresponding bit in the inode bitmap to 0
    int byte = floor((inode_num - 1) / 8);
    int bit = (inode_num - 1) % 8;

    debug_printf("INODE MASK\n");
    inode_bits[byte] &= ~(1 << bit);

    sb->s_free_inodes_count++;
    gd->bg_free_inodes_count++;

    // free all the associated blocks
    for (int i=0; i<12; i++) {
        if (inode->i_block[i] != 0) {
            debug_printf("FREE BLOCK\n");
            free_block(inode->i_block[i]);
        }
    }

    // free the indirect block if needed
    if (inode->i_block[12] != 0) {

        // total block pointers are the size of the block / the size of a pointer
        int max_pointers = EXT2_BLOCK_SIZE / sizeof(unsigned int);

        unsigned int *ind_block = (unsigned int *)(disk + EXT2_BLOCK_SIZE*inode->i_block[12]);

        for (int i=0; i<max_pointers; i++) {
            if (ind_block[i] != 0) {
                free_block(ind_block[i]);
            }
        }

        free_block(inode->i_block[12]);
    }
}

// changes the correct bit in the bitmap to 0
void free_block(int block_num) {
    unsigned char *block_bits = get_block_bits();
    int byte = floor((block_num - 1) / 8);
    int bit = (block_num - 1) % 8;

    block_bits[byte] &= ~(1 << bit);

    sb->s_free_blocks_count++;
    gd->bg_free_blocks_count++;
}

// decrements the link count of a given inode and updates its values
// if the links count reaches 0
void decrement_link(int inode_num) {
    struct ext2_inode *inode = get_inode_from_num(inode_num);
    inode->i_links_count--;

    if (inode->i_links_count == 0) {
        inode->i_dtime = (unsigned int)(time(NULL));
        free_inode(inode_num);
    }
}


void remove_dir_entry(int parent_ino_num, const char *path) {
    char *name = get_name(path);
    int last_inode = get_last_inode(path);

    struct ext2_dir_entry *dir_entry = get_file_dir(parent_ino_num, name);
    struct ext2_dir_entry *prev_entry = get_prev_dir_entry(parent_ino_num, name);

    if (prev_entry != NULL) {
        prev_entry->rec_len += dir_entry->rec_len;
    } else {
        dir_entry->inode = 0;
        // rec len is already pointing to the next entry
    }

    decrement_link(last_inode);
}

int max(int a, int b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

int update_directory_entry(const char *dest_path, int inode_index) {
    int ret = -1;
    // find dir inode
    int dir_ino = get_second_last_inode(dest_path);
    debug_printf("CP: dir_ino=%i\n", dir_ino);

    // add dir entry
    ret = add_dir_entry(dir_ino, inode_index, 0, dest_path, 'f');

    return ret;
}

void debug_printf( const char * format, ...) {
    if (DEBUGPRINT) {
        va_list args;                // Declare a variable to hold the arguments
        va_start(args, format);      // Initialize it with the last named parameter
        vprintf(format, args);       // Use vprintf to handle the variable arguments
        va_end(args);        
    }
}

void debug_print_int_to_bin( const char * str, unsigned int n) {
    if (DEBUGPRINT) {
        char * binary = int_to_binary(n);
        debug_printf("%s=%s\n", str, binary);
        free(binary);
    }
}

char *int_to_binary(unsigned int n) {
    int bits = sizeof(n) * 8; // Total bits in an integer (32 for a 4-byte int)
    char *binary = (char *)malloc(bits + 1); // Allocate memory for bits and null terminator

    if (!binary) {
        perror("Failed to allocate memory");
        exit(1);
    }

    for (int i = bits - 1; i >= 0; i--) {
        unsigned int mask = 1U << i;
        binary[bits - 1 - i] = (n & mask) ? '1' : '0';
    }

    binary[bits] = '\0'; // Null-terminate the string
    return binary;
}

void debug_print_dir_entry(struct ext2_dir_entry *entry) {
    if (DEBUGPRINT) {
        debug_printf("entry=%p\n", entry);
        debug_printf("entry->file_type=0x%X\n", entry->file_type);
        debug_printf("entry->inode=%u\n", entry->inode);
        debug_printf("entry->rec_len=%u\n", entry->rec_len);
        debug_printf("entry->name_len=%u\n", entry->name_len);
        debug_printf("entry->name=%s\n", entry->name);
    }
}