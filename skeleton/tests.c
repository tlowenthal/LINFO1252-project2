#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "lib_tar.h"

/**
 * You are free to use this file to write tests for your implementation
 */

void debug_dump(const uint8_t *bytes, size_t len) {
    for (int i = 0; i < len;) {
        printf("%04x:  ", (int) i);

        for (int j = 0; j < 16 && i + j < len; j++) {
            printf("%02x ", bytes[i + j]);
        }
        printf("\t");
        for (int j = 0; j < 16 && i < len; j++, i++) {
            printf("%c ", bytes[i]);
        }
        printf("\n");
    }
}

void test_list(int fd, char *path){

    size_t len = 100;
    char *entries[len];
    for (int i = 0; i < len; i++){
        entries[i] = malloc(100);
    }

    int ret = list(fd, path, entries, &len);
    printf("list returned %d\n", ret);

    for (int i = 0; i < len; i++){
        printf("%s\n", entries[i]);
    }
    printf("\n");
    for (int i = 0; i < 100; i++){
        free(entries[i]);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s tar_file\n", argv[0]);
        return -1;
    }

    int fd = open(argv[1] , O_RDONLY);
    if (fd == -1) {
        perror("open(tar_file)");
        return -1;
    }

    int ret = check_archive(fd);
    printf("check_archive returned %d\n", ret);
    printf("should have returned : 14\n\n");

    ret = is_symlink(fd, "testlinktofile");
    printf("is_symlink returned %d\n", ret);
    printf("should have returned : 1\n\n");

    test_list(fd, "dir2");

    /*

    ret = exists(fd, "dir1/folder1/file1.txt");
    printf("exists returned %d\n", ret);
    ret = exists(fd, "dir1/folder1/");
    printf("exists returned %d\n", ret);
    ret = exists(fd, "di");
    printf("exists returned %d\n", ret);
    printf("should have returned : 1, 1, 0\n\n");

    ret = is_dir(fd, "dir1/folder1/");
    printf("is_dir returned %d\n", ret);
    ret = is_dir(fd, "dir1/folder1/file1.txt");
    printf("is_dir returned %d\n", ret);
    printf("should have returned : 1, 0\n\n");

    ret = is_file(fd, "dir1/folder1/");
    printf("is_file returned %d\n", ret);
    ret = is_file(fd, "dir1/folder1/file1.txt");
    printf("is_file returned %d\n", ret);
    printf("should have returned : 0, 1\n\n");

    ret = is_symlink(fd, "dir1");
    printf("is_symlink returned %d\n", ret);
    printf("should have returned : 0\n\n");
    */

    size_t len = 1000;
    uint8_t buffer[len];
    size_t offset = 0;
    ssize_t bytes_left = read_file(fd, "fichier5", offset, buffer, &len);
    printf("read_file returned %ld\n", bytes_left);
    //printf("should have returned : 1\n");
    printf("%ld bytes were written to the destination buffer.\n", len);

    return 0;
}
