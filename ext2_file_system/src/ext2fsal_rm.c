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
#include <time.h>


int32_t ext2_fsal_rm(const char *path)
{
    /**
     * TODO: implement the ext2_rm command here ...
     * the argument 'path' is the path to the file to be removed.
     */

    if (strlen(path) > 4095) {
        return ENAMETOOLONG;
    }

    // check if paths are absolte
    if (path[0] != '/') {
        return EINVAL;
    }

    // make sure path exists
    debug_printf("RM: path=%s\n", path);
    int errnum = traverse_path(path);
    if (errnum != 0) {
        debug_printf("invalid path\n");
        return errnum;
    }

    int last_inode_num = get_last_inode(path);
    struct ext2_inode *last_inode = get_inode_from_num(last_inode_num);

    // check if last element is a directory
    if (last_inode->i_mode & EXT2_S_IFDIR) {
        return EISDIR;
    }

    last_inode->i_dtime = time(NULL);

    int sec_last_inode_num = get_second_last_inode(path);
    
    remove_dir_entry(sec_last_inode_num, path);
    return 0;
}