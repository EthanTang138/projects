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

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


int allocate_and_copy_blocks(const char *dest_path, const char *buffer, int size, int inode_index, bool overwrite) {
    debug_printf("CP: dest_path=%s, buffer=0x%X, size=%i, inode_index=%i\n", dest_path, buffer, size, inode_index);

    // Allocate blocks for the file data
    int blocks_needed = (size + EXT2_BLOCK_SIZE - 1) / EXT2_BLOCK_SIZE;
    debug_printf("CP: blocks_needed=%i\n", blocks_needed);
    if (blocks_needed > 12) {
        debug_printf("only copying 12 blocks of data");
        blocks_needed = 12;
    }

    int blocks[blocks_needed];
    for (int i = 0; i < blocks_needed; i++) {
        blocks[i] = next_available_block();
        if (blocks[i] < 0) {
            debug_printf("No more free blocks available\n");
            return ENOMEM;
        }
    }

    // Write the file data to the allocated blocks
    int size_counter = size;
    for (int i = 0; i < blocks_needed; i++) {
        int block_offset = blocks[i] * EXT2_BLOCK_SIZE;
        int to_write = (size_counter > EXT2_BLOCK_SIZE) ? EXT2_BLOCK_SIZE : size_counter;

        memcpy((char *)(disk + block_offset), buffer + (i * EXT2_BLOCK_SIZE), to_write);
        size_counter -= EXT2_BLOCK_SIZE;
    }

    // Update the inode with file metadata
    struct ext2_inode *inode = get_inode_from_num(inode_index);
    inode->i_size = size;
    inode->i_mode = EXT2_S_IFREG;
    inode->i_blocks = blocks_needed * 2;
    memcpy(inode->i_block, blocks, blocks_needed * sizeof(int));

    if (!overwrite) {
        // Update the directory entry
        if (update_directory_entry(dest_path, inode_index) < 0) {
            debug_printf("Error updating directory entry\n");
            return -1;
        }
    }
    return 0;
}

// cp to new file
int32_t cp_file(const char *src, const char *dst, bool overwrite) {
    debug_printf("CP cp_file: src=%s, dst=%s, overwrite=%i\n", src, dst, overwrite);

    // Open source file
    int src_fd = open(src, O_RDONLY);
    if (src_fd < 0) {
        debug_printf("Error opening source file");
        return ENOENT;
    }
    debug_printf("CP: src_fd=%i\n", src_fd);

    // Get source file size
    struct stat src_stat;
    if (fstat(src_fd, &src_stat) < 0) {
        debug_printf("Error getting source file size");
        // close(src_fd);
        return ENOENT;
    }
    debug_printf("CP: src_stat.st_size=%u\n", src_stat.st_size);

    // Allocate memory to read the file
    char *buffer = malloc(src_stat.st_size);
    if (!buffer) {
        debug_printf("Error allocating memory");
        // close(src_fd);
        return ENOMEM;
    }
    debug_printf("CP: buffer=0x%x\n", buffer);

    // Read file into buffer
    if (read(src_fd, buffer, src_stat.st_size) != src_stat.st_size) {
        debug_printf("Error reading source file");
        free(buffer);
        // close(src_fd);
        return ENODATA;
    }
    debug_printf("CP: after read\n");

    int inode_index = 0;
    if (overwrite) {
        // find inode
        inode_index = traverse_path(dst);
        struct ext2_inode *inode = get_inode_from_num(inode_index);

        // free the existing blocks
        for (int i = 0; i < inode->i_blocks / 2; i++) {
            free_block(inode->i_block[i]);
            inode->i_block[i]=0;
        }
        inode->i_blocks = 0;
    }
    else {
        // get a new inode
        inode_index = next_available_inode();
        if (inode_index < 0) {
            debug_printf("No free inode available\n");
            free(buffer);
            return ENOMEM;
        }
    }

    // Write file to ext2 filesystem
    if (allocate_and_copy_blocks(dst, buffer, src_stat.st_size, inode_index, overwrite) < 0) {
        debug_printf("Error writing file to ext2 filesystem\n");
        free(buffer);
        return -1;
    }

    debug_printf("File successfully copied to ext2 filesystem\n");

    free(buffer);
    return 0;
}

int32_t ext2_fsal_cp(const char *src, const char *dst) {
    /**
     * TODO: implement the ext2_cp command here ...
     * Arguments src and dst are the cp command arguments described in the handout.
     */

    if (strlen(src) > 4095 || strlen(dst) > 4095) {
        return ENAMETOOLONG;
    }

    debug_printf("CP: src=%s\n", src);
    struct stat src_stat;
    if (stat(src, &src_stat) < 0) { // see if src exists
        return ENOENT;
    }

    debug_printf("CP: dst=%s\n", dst);
    int ino_dst = traverse_path_check(dst);
    debug_printf("CP: ino_dst=%i\n", ino_dst);
    if (ino_dst == -1) { // -1 == invalid path, -2 == not found
        return ENOENT;
    } else if (ino_dst == -2) {
        // new file
        return cp_file(src, dst, false);

    } else if (ino_dst > 0) {
        struct ext2_inode *inode_dst = get_inode_from_num(ino_dst);
        debug_printf("CP: inode->i_mode=0x%X\n", inode_dst->i_mode);
        if (inode_dst->i_mode == EXT2_S_IFLNK) {
            return EEXIST;
        } else if (inode_dst->i_mode == EXT2_S_IFREG) {
            // overwrite existing
            return cp_file(src, dst, true);
        }
    }

    return 0;
}