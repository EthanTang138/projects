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

#include "ext2fsal.h"
#include "e2fs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include  <ctype.h>

void debug_print_dir_entry(struct ext2_dir_entry *entry);

int32_t ext2_fsal_mkdir(const char *path)
{
    /**
     * TODO: implement the ext2_mkdir command here ...
     * the argument path is the path to the directory that is to be created.
     */

     /* This is just to avoid compilation warnings, remove this line when you're done. */

    if (strlen(path) > 4095) {
        return ENAMETOOLONG;
    }

    // check if paths are absolte
    if (path[0] != '/') {
        return EINVAL;
    }

    // check if every element on the path is a directory except the last
    char path_cpy[strlen(path) + 1];
    strcpy(path_cpy, path);

    char *last_slash = strrchr(path_cpy, '/');
    if (last_slash == NULL) {
        exit(1);
    }
    *last_slash = '\0';

    debug_printf("all-1 %s\n", path_cpy);

    // check if path is valid
    if (path_cpy[0] != '\0') {
        int errnum = traverse_path(path_cpy);
        if (errnum != 0) {
            debug_printf("invalid path\n");
            return errnum;
        }
    }

    char *new_dir_name = get_name(path);
    debug_printf("new dir name: %s\n", new_dir_name);

    // get the inode number of the parent of the created directory
    int parent_inode_num = get_second_last_inode(path);

    // check if the file/dir already exists
    if (get_file_dir(parent_inode_num, new_dir_name) != NULL) {
        debug_printf("already exists\n");
        return EEXIST;
    }

    // allocate a new inode
    int new_inode_num = new_inode('d', EXT2_BLOCK_SIZE);
    if (new_inode_num == -1) {
        return ENOMEM;
    }
    debug_printf("File type %x\n", get_inode_from_num(new_inode_num)->i_mode);

    // add . and .. to the new directory
    int errnum = add_dir_entry(new_inode_num, new_inode_num, EXT2_BLOCK_SIZE, ".", 'd');
    if (errnum != 0) {
        return errnum;
    }

    errnum = add_dir_entry(new_inode_num, parent_inode_num, EXT2_BLOCK_SIZE, "..", 'd');
    if (errnum != 0) {
        return errnum;
    }


    struct ext2_dir_entry *entry = (struct ext2_dir_entry *)(disk + EXT2_BLOCK_SIZE*get_inode_from_num(new_inode_num)->i_block[0]);
    // add the new directory to the parent's directory block
    debug_printf("get name: %s\n", get_name(path));
    errnum = add_dir_entry(parent_inode_num, new_inode_num, EXT2_BLOCK_SIZE, path, 'd');
    if (errnum != 0) {
        return errnum;
    }

    debug_printf("File type %x\n", get_inode_from_num(new_inode_num)->i_mode);
    int count = 0;
    while (count != 1024) {
        
        debug_printf("------------\n");
        debug_print_dir_entry(entry);

        count += entry->rec_len;
        entry = (struct ext2_dir_entry *)(((char*) entry)+ entry->rec_len);
    }
   

    return 0;
}