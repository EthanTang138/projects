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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


int32_t ext2_fsal_ln_hl(const char *src,
                        const char *dst)
{
    /**
     * TODO: implement the ext2_ln_hl command here ...
     * src and dst are the ln command arguments described in the handout.
     */

    if (strlen(src) > 4095 || strlen(dst) > 4095) {
        return ENAMETOOLONG;
    }

    // check if paths are absolte
    if (src[0] != '/' || dst[0] != '/') {
        return EINVAL;
    }

    // make sure src exists
    debug_printf("HL: src=%s\n", src);
    int errnum = traverse_path(src);
    if (errnum != 0) {
        debug_printf("invalid path\n");
        return errnum;
    }

    // make sure src is not dir
    int src_inode_num = get_last_inode(src);
    struct ext2_inode *inode = get_inode_from_num(src_inode_num);

    debug_printf("SL: inode->i_mode=0x%X\n", inode->i_mode);
    debug_printf("SL: inode isDir=%i\n", inode->i_mode & EXT2_S_IFDIR);
    if (inode->i_mode & EXT2_S_IFDIR) {
        return EISDIR;
    }

    // make sure dst doesn't exist
    debug_printf("SL: dst=%s\n", dst);
    int second_last_ino = get_second_last_inode(dst);
    if (get_file_dir(second_last_ino, get_name(dst)) != NULL) {
        return EEXIST;
    }
    
    errnum = add_dir_entry(second_last_ino, src_inode_num, EXT2_BLOCK_SIZE, dst, 'f');
    if (errnum != 0) {
        return errnum;
    }

    return 0;
}
