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
#include <math.h>


int32_t ext2_fsal_ln_sl(const char *src,
                        const char *dst)
{
    /**
     * TODO: implement the ext2_ln_sl command here ...
     * src and dst are the ln command arguments described in the handout.
     */

    if (strlen(src) > 4095 || strlen(dst) > 4095) {
        return ENAMETOOLONG;
    }

    // check if paths are absolte
    if (src[0] != '/' || dst[0] != '/') {
        return EINVAL;
    }

    // make sure dst doesn't exist
    debug_printf("SL: dst=%s\n", dst);
    int second_last_ino = get_second_last_inode(dst);
    if (get_file_dir(second_last_ino, get_name(dst)) != NULL) {
        // check that dst is not dir
        int dst_inode_num = get_last_inode(dst);
        struct ext2_inode *inode = get_inode_from_num(dst_inode_num);

        debug_printf("SL: inode->i_mode=0x%X\n", inode->i_mode);
        debug_printf("SL: inode isDir=%i\n", inode->i_mode & EXT2_S_IFDIR);
        if (inode->i_mode & EXT2_S_IFDIR) {
            return EISDIR;
        }

        return EEXIST;
    }

    // add the link name file
    int new_inode_num = new_inode('l', EXT2_BLOCK_SIZE);
    if (new_inode_num == -1) {
        return ENOMEM;
    }
    int errnum = add_dir_entry(second_last_ino, new_inode_num, EXT2_BLOCK_SIZE, dst, 'l');
    if (errnum != 0) {
        return errnum;
    }

    // store the path to src in the created file
    // struct ext2_inode *new_inode = get_inode_from_num(new_inode_num);
    int needed_blocks = strlen(src) / 1024 + 1;
    debug_printf("%d needed blocks %ld strlen src \n", needed_blocks, strlen(src));


     // Allocate blocks for the file data
    int size = strlen(src);
    int blocks_needed = (size + EXT2_BLOCK_SIZE - 1) / EXT2_BLOCK_SIZE;
    debug_printf("SL: blocks_needed=%i\n", blocks_needed);

    // max src length is 4096, 4 blocks
    if (blocks_needed > 4) {
        debug_printf("only copying 12 blocks of data");
        blocks_needed = 4;
    }

    int blocks[blocks_needed];
    for (int i = 0; i < blocks_needed; i++) {
        blocks[i] = next_available_block();
        if (blocks[i] < 0) {
            debug_printf("No more free blocks available\n");
            for (; i>=0; i--) {
                free_block(blocks[i]);
            }
            return ENOMEM;
        }
    }

    // Write the src path to the allocated blocks
    debug_printf("%s src path\n", src);
    int size_counter = size;
    for (int i = 0; i < blocks_needed; i++) {
        int block_offset = blocks[i] * EXT2_BLOCK_SIZE;
        int to_write = (size_counter > EXT2_BLOCK_SIZE) ? EXT2_BLOCK_SIZE : size_counter;

        memcpy((char *)(disk + block_offset), src + (i * EXT2_BLOCK_SIZE), to_write);
        size_counter -= EXT2_BLOCK_SIZE;
    }


    // Update the inode with file metadata
    struct ext2_inode *inode = get_inode_from_num(new_inode_num);
    inode->i_size = size;
    inode->i_mode = EXT2_S_IFLNK;
    inode->i_blocks = blocks_needed * 2;
    memcpy(inode->i_block, blocks, blocks_needed * sizeof(int));

    return 0;
}
