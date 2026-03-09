#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "ext2.h"
#include "e2fs.h"
#include <assert.h>
#include <errno.h>
#include "ext2fsal.h"

unsigned char *disk;
struct ext2_super_block *sb;
struct ext2_group_desc *gd;

struct Node {
    unsigned int block_num;
    struct Node *next;
};

void append(struct Node **head, unsigned int num) {
    struct Node *add = malloc(sizeof(struct Node));
    add->block_num = num;
    add->next = NULL;
    
    if (*head == NULL) {
        *head = add;
        return;
    }

    struct Node *curr = *head;
    while (curr->next != NULL) {
        curr = curr->next;
    }

    curr->next = add;
}

int main(int argc, char **argv) {

    if(argc != 2) {
        fprintf(stderr, "Usage: %s <image file name>\n", argv[0]);
        exit(1);
    }
    int fd = open(argv[1], O_RDWR);

    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    sb = (struct ext2_super_block *)(disk + 1024);
    gd = (struct ext2_group_desc *)(disk + 1024*2);

    // assert(12 == get_file_dir(2, "level1")->inode);

    if (strcmp(argv[1], "./img/twolevel.img") == 0) {
        assert(ENOENT == traverse_path("/foo/bar/blah"));
        assert(EINVAL == traverse_path("foo/bar/blah"));
        assert(0 == traverse_path("/level1/level2/"));
        assert(0 == traverse_path("/level1/////level2"));
        assert(ENOENT == traverse_path("/level1/boo"));
        assert(ENOENT == traverse_path("/afile/booo"));

        assert(13 == get_second_last_inode("/level1/level2/blah"));
        assert(13 == get_second_last_inode("/level1/////////level2/blah"));
        assert(12 == get_second_last_inode("/level1/level2/"));
        assert(2 == get_second_last_inode("/level1"));

        assert (13 == get_last_inode("/level1/level2"));
        assert (13 == get_last_inode("/level1/level2/"));

        assert (2 == get_file_dir(12, "..")->inode);
        assert (12 == get_prev_dir_entry(13, "bfile")->inode);
        assert (NULL == get_prev_dir_entry(13, "."));
    }
//    assert(0 == traverse_path("/level1"));

    // int num = new_inode('d', 1024);
    // struct ext2_inode *inode_table = get_inode_table();
    // assert(inode_table[num - 1].i_size == 1024);

    // add_dir_entry(2, 14, EXT2_BLOCK_SIZE, "level3");

//

//    assert(0 == ext2_fsal_ln_hl("/level1/bfile", "/bfile-ln"));

    // printf("%d asdfsdfsd\n", ext2_fsal_ln_sl("/level1/bfile", "/bfile-sl"));
    // assert(0 == ext2_fsal_ln_sl("/level1/bfile", "/bfile-sl"));
    // assert(EISDIR == ext2_fsal_ln_sl("/level1/bfile", "/aa"));

    // assert(0 == ext2_fsal_ln_sl("/level1/level2/bfile", "/slb"));

    ext2_fsal_cp("./img/dog.txt", "/dogcopy");

    // assert(EISDIR == ext2_fsal_rm("/.."));
    // assert (EISDIR == ext2_fsal_rm("/lost+found"));
    // assert (EISDIR == ext2_fsal_rm("/lost+found/"));
    // assert(0 == ext2_fsal_rm("/largefile.txt"));

    return 0;
}