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

    int nb = 0;

    while (1){

        //read tar in tar_header buffer
        tar_header_t buf;
        if (pread(tar_fd, &buf, sizeof(tar_header_t), nb*sizeof(tar_header_t)) < 0) perror("read error in check_archive");

        if (!strlen((char *) &buf)){
            tar_header_t header2;
            if (pread(tar_fd, &header2, sizeof(tar_header_t), (nb+1)*sizeof(tar_header_t)) < 0) perror("pread error in check_archive\n");
            if (!strlen((char *) &header2)){
                return 0;
            }
        }

        //checking for version and magic
        if (strcmp(buf.magic, TMAGIC)) return -1;
        if (buf.version[0] != TVERSION[0] && buf.version[1] != TVERSION[1]) return -2;//this is needed because strcmp adding "/0" or not is ambiguous

        //checking for checksum

        //first store value from header
        uint sum = TAR_INT(buf.chksum);

        //then calculate checksum ourselves
        if (memset(buf.chksum, ' ', 8) == NULL) perror("memset error in check_archive");//replace checksum bytes with spaces
        uint8_t *mapping = (uint8_t *) &buf;
        uint count = 0;
        for (int i = 0; i < BLOCKSIZE; i++){
            count += *(mapping++);
        }

        //we can finally check
        if (sum != count) return -3;

        if (TAR_INT(buf.size)%BLOCKSIZE == 0){
            nb += (1 + TAR_INT(buf.size)/BLOCKSIZE);
        } else {
            nb += (2 + TAR_INT(buf.size)/BLOCKSIZE);
        }
    }
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

    int nb = 0;

    while (1){
        tar_header_t header;
        if (pread(tar_fd, &header, sizeof(tar_header_t), nb*sizeof(tar_header_t)) < 0) perror("pread error in exists\n");

        if (!strcmp(header.name, path)) return 1;

        if (!strlen((char *) &header)){
            tar_header_t header2;
            if (pread(tar_fd, &header2, sizeof(tar_header_t), (nb+1)*sizeof(tar_header_t)) < 0) perror("pread error in exists\n");
            if (!strlen((char *) &header2)){
                return 0;
            }
        }

        if (TAR_INT(header.size)%BLOCKSIZE == 0){
            nb += (1 + TAR_INT(header.size)/BLOCKSIZE);
        } else {
            nb += (2 + TAR_INT(header.size)/BLOCKSIZE);
        }
    }
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

    int nb = 0;

    while (1){
        tar_header_t header;
        if (pread(tar_fd, &header, sizeof(tar_header_t), nb*sizeof(tar_header_t)) < 0) perror("pread error in is_dir\n");

        if (!strcmp(header.name, path) && header.typeflag == DIRTYPE) return 1;

        if (!strlen((char *) &header)){
            tar_header_t header2;
            if (pread(tar_fd, &header2, sizeof(tar_header_t), (nb+1)*sizeof(tar_header_t)) < 0) perror("pread error in is_dir\n");
            if (!strlen((char *) &header2)){
                return 0;
            }
        }

        if (TAR_INT(header.size)%BLOCKSIZE == 0){
            nb += (1 + TAR_INT(header.size)/BLOCKSIZE);
        } else {
            nb += (2 + TAR_INT(header.size)/BLOCKSIZE);
        }
    }
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

    int nb = 0;

    while (1){
        tar_header_t header;
        if (pread(tar_fd, &header, sizeof(tar_header_t), nb*sizeof(tar_header_t)) < 0) perror("pread error in is_file\n");

        if (!strcmp(header.name, path) && (header.typeflag == REGTYPE || header.typeflag == AREGTYPE)) return 1;

        if (!strlen((char *) &header)){
            tar_header_t header2;
            if (pread(tar_fd, &header2, sizeof(tar_header_t), (nb+1)*sizeof(tar_header_t)) < 0) perror("pread error in is_file\n");
            if (!strlen((char *) &header2)){
                return 0;
            }
        }

        if (TAR_INT(header.size)%BLOCKSIZE == 0){
            nb += (1 + TAR_INT(header.size)/BLOCKSIZE);
        } else {
            nb += (2 + TAR_INT(header.size)/BLOCKSIZE);
        }
    }
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

    int nb = 0;

    while (1){
        tar_header_t header;
        if (pread(tar_fd, &header, sizeof(tar_header_t), nb*sizeof(tar_header_t)) < 0) perror("pread error in is_symlink\n");

        if (!strcmp(header.name, path) && (header.typeflag == LNKTYPE || header.typeflag == SYMTYPE)) return 1;

        if (!strlen((char *) &header)){
            tar_header_t header2;
            if (pread(tar_fd, &header2, sizeof(tar_header_t), (nb+1)*sizeof(tar_header_t)) < 0) perror("pread error in is_symlink\n");
            if (!strlen((char *) &header2)){
                return 0;
            }
        }

        if (TAR_INT(header.size)%BLOCKSIZE == 0){
            nb += (1 + TAR_INT(header.size)/BLOCKSIZE);
        } else {
            nb += (2 + TAR_INT(header.size)/BLOCKSIZE);
        }

    }

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

    //fonctionne presque, ca met tous les sous-fichiers (y compris d dans l'exemple)

    int nb = 0;
    int index = 0;

    while (1){
        tar_header_t header;
        if (pread(tar_fd, &header, sizeof(tar_header_t), nb*sizeof(tar_header_t)) < 0) perror("pread error in list\n");

        if (!strcmp(header.name, path)){
            if (header.typeflag == DIRTYPE){
                int nb2 = nb + 1;
                while(1){
                    tar_header_t entry;
                    if (pread(tar_fd, &entry, sizeof(tar_header_t), (nb2)*sizeof(tar_header_t)) < 0) perror("pread error in list\n");

                    if (!strncmp(entry.name, path, strlen(path))){
                        memcpy(entries[index], entry.name, strlen(entry.name));
                        index++;
                    } else if (!(header.typeflag == LNKTYPE || header.typeflag == SYMTYPE)){
                        *no_entries = index;
                        return 1;
                    }
                    if (TAR_INT(entry.size)%BLOCKSIZE == 0){
                        nb2 += (1 + TAR_INT(entry.size)/BLOCKSIZE);
                    } else {
                        nb2 += (2 + TAR_INT(entry.size)/BLOCKSIZE);
                    }
                }
            } else if (header.typeflag == LNKTYPE || header.typeflag == SYMTYPE){
                list(tar_fd, header.linkname, entries, no_entries);
            }
        }

        if (!strlen((char *) &header)){
            tar_header_t header2;
            if (pread(tar_fd, &header2, sizeof(tar_header_t), (nb+1)*sizeof(tar_header_t)) < 0) perror("pread error in list\n");
            if (!strlen((char *) &header2)){
                return 0;
            }
        }

        if (TAR_INT(header.size)%BLOCKSIZE == 0){
            nb += (1 + TAR_INT(header.size)/BLOCKSIZE);
        } else {
            nb += (2 + TAR_INT(header.size)/BLOCKSIZE);
        }
    }

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