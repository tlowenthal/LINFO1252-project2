#include "lib_tar.h"

/**
 * Checks whether the archive is valid.
 *
 * Each non-null header of a valid archive has:
 *  - a magic value of "ustar" and a null,
 *  - a version value of "00" and no null,
 *  - a correct checksum
 *
 * @param tar_fd A file descriptor pointing to the start of a file supposed to contain a tar archive.
 *
 * @return a zero or positive value if the archive is valid, representing the number of non-null headers in the archive,
 *         -1 if the archive contains a header with an invalid magic value,
 *         -2 if the archive contains a header with an invalid version value,
 *         -3 if the archive contains a header with an invalid checksum value
 */
int check_archive(int tar_fd) {
    /*

    //define values
    char *mag = malloc(6*sizeof(char));
    char *vers = malloc(2*sizeof(char));
    char *sum = malloc(8*sizeof(char));

    //store and check size
    struct stat buf;
    if (fstat(tar_fd, &buf) < 0) perror("fstat failed in check_archive");
    size_t size = buf.st_size;
    if (size < 512) perror("file not long enough");

    //file mapping
    char *mapper = (char *) mmap(NULL, size, PROT_READ, MAP_SHARED, tar_fd, 0);
    if (mapper == MAP_FAILED) perror("map failed in check_archive");

    //get actual checkcount
    memset(mapper + 148, ' ', 8);//set checkcount to spaces to perform counting
    int count = 0;
    //char *increment = malloc(8);
    //for (int i = 0; i < 512/8; i+=8){
        //memcpy(increment, mapper + i, 8);
        //count += TAR_INT(increment);
    //}

    //read and store values
    if (pread(tar_fd, (void *) mag, 6, 257) < 0) perror("magic pread failed in check_archive");
    if (pread(tar_fd, (void *) vers, 2, 263) < 0) perror("version pread failed in check_archive");
    if (pread(tar_fd, (void *) sum, 8, 148) < 0) perror("checksum pread failed in check_archive");

    //checking
    if (strcmp(mag, TMAGIC)) return -1;
    if (strcmp(vers, TVERSION)) return -2;
    printf("%ld\n", sizeof(sum));
    printf("%ld\n", sizeof(uint32_t));
    if (TAR_INT(sum) != count) return -3;
    return 0;
    */

    tar_header_t *buf = malloc(sizeof(tar_header_t));
    if (read(tar_fd, buf, sizeof(tar_header_t)) < 0) perror("read error in check_archive");
    if (strcmp(buf->magic, TMAGIC)) return -1;
    if (buf->version[0] != TVERSION[0] && buf->version[1] != TVERSION[1]) return -2;


    uint8_t sum = TAR_INT(buf->chksum);
    if (memset(buf->chksum, 32, 8) == NULL) perror("memset error in check_archive");
    uint8_t *mapping = (uint8_t *) buf;
    uint8_t count = 0;
    for (int i = 0; i < 512/8; i++){
        count += *(mapping++);
    }

    printf("%d\n", count);
    printf("%d\n", sum);
    if (sum != count) return -3;
    return 0;
}

/**
 * Checks whether an entry exists in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive,
 *         any other value otherwise.
 */
int exists(int tar_fd, char *path) {
    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a directory.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a directory,
 *         any other value otherwise.
 */
int is_dir(int tar_fd, char *path) {
    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a file.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a file,
 *         any other value otherwise.
 */
int is_file(int tar_fd, char *path) {
    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a symlink.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 * @return zero if no entry at the given path exists in the archive or the entry is not symlink,
 *         any other value otherwise.
 */
int is_symlink(int tar_fd, char *path) {
    return 0;
}


/**
 * Lists the entries at a given path in the archive.
 * list() does not recurse into the directories listed at the given path.
 *
 * Example:
 *  dir/          list(..., "dir/", ...) lists "dir/a", "dir/b", "dir/c/" and "dir/e/"
 *   ├── a
 *   ├── b
 *   ├── c/
 *   │   └── d
 *   └── e/
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive. If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param entries An array of char arrays, each one is long enough to contain a tar entry path.
 * @param no_entries An in-out argument.
 *                   The caller set it to the number of entries in `entries`.
 *                   The callee set it to the number of entries listed.
 *
 * @return zero if no directory at the given path exists in the archive,
 *         any other value otherwise.
 */
int list(int tar_fd, char *path, char **entries, size_t *no_entries) {
    return 0;
}

/**
 * Reads a file at a given path in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive to read from.  If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param offset An offset in the file from which to start reading from, zero indicates the start of the file.
 * @param dest A destination buffer to read the given file into.
 * @param len An in-out argument.
 *            The caller set it to the size of dest.
 *            The callee set it to the number of bytes written to dest.
 *
 * @return -1 if no entry at the given path exists in the archive or the entry is not a file,
 *         -2 if the offset is outside the file total length,
 *         zero if the file was read in its entirety into the destination buffer,
 *         a positive value if the file was partially read, representing the remaining bytes left to be read to reach
 *         the end of the file.
 *
 */
ssize_t read_file(int tar_fd, char *path, size_t offset, uint8_t *dest, size_t *len) {
    return 0;
}