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

    int nb = 0;//track where we are in the archive, at which block
    int nb_headers = 0;//header count to be returned

    while (1){

        //read block in tar_header buffer
        tar_header_t buf;
        if (pread(tar_fd, &buf, sizeof(tar_header_t), nb*sizeof(tar_header_t)) < 0) perror("read error in check_archive");

        //if the block is filled with "\0" (so strlen == 0), check if the following one is also filled with "\0", and if so return 0
        if (!strlen((char *) &buf)){
            tar_header_t header2;
            if (pread(tar_fd, &header2, sizeof(tar_header_t), (nb+1)*sizeof(tar_header_t)) < 0) perror("pread error in check_archive\n");
            if (!strlen((char *) &header2)){
                return nb_headers;
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

        //here we go to the next header
        if (TAR_INT(buf.size)%BLOCKSIZE == 0){// if all file blocks are exactly full (no padding)
            nb += (1 + TAR_INT(buf.size)/BLOCKSIZE);
        } else {
            nb += (2 + TAR_INT(buf.size)/BLOCKSIZE);
        }
        nb_headers++;//increment number of headers

        //the four next function follow the exact same technique !
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

    //devrait check archive avant de procéder

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

    //devrait utiliser exist avant

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

    //devrait utiliser exist avant

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

    //devrait utiliser exist

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

int check(char *s,char c)
{
    int count=0;
    for(int i=0;s[i];i++)  {
        if(s[i]==c) count++;
 	}
 	return count; 		  
}

int is_entry_but_not_sub(char *dir, char* entry){
    if (!strncmp(dir, entry, strlen(dir)) && strlen(dir) < strlen(entry)){// starts the same but entry must be longer
        if (check(dir, '/') == check(entry, '/')){
            return 1;
        } else if (check(dir, '/') + 1 == check(entry, '/') && entry[strlen(entry) - 1] == '/'){
            return 1;
        }
        return 0;
    }
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

    //devrait utiliser is_dir/is_symlink et eventuellement is_file

    int link = 0;
    int nb = 0;
    int index = 0;

    if(is_dir(tar_fd, path)==0){
        if(is_symlink(tar_fd, path)==0){
            perror("path given to list is invalid.\n");
            *no_entries = 0;
            return 0;
        }else{ link=1; }
    }

    if (link){
        while(1){
            tar_header_t header;
            if (pread(tar_fd, &header, sizeof(tar_header_t), nb*sizeof(tar_header_t)) < 0) perror("pread error in list\n");

            if (!strcmp(header.name, path)){//when we find the symbolic link
                return list(tar_fd, strcat(header.linkname, "/"), entries, no_entries);
            }

            if (TAR_INT(header.size)%BLOCKSIZE == 0){
                nb += (1 + TAR_INT(header.size)/BLOCKSIZE);
            } else {
                nb += (2 + TAR_INT(header.size)/BLOCKSIZE);
            }
        }
    } else {
        while(1){
            tar_header_t header;
            if (pread(tar_fd, &header, sizeof(tar_header_t), nb*sizeof(tar_header_t)) < 0) perror("pread error in list\n");

            if (is_entry_but_not_sub(path, header.name)){
                if (index < *no_entries){
                    memcpy(entries[index], header.name, strlen(header.name));//if it is an entry but not a sub-entry, we copy it to entries
                    index++;
                } else {
                    return 1;
                }
            }

            if (!strlen((char *) &header)){
                tar_header_t header2;
                if (pread(tar_fd, &header2, sizeof(tar_header_t), (nb+1)*sizeof(tar_header_t)) < 0) perror("pread error in list\n");
                if (!strlen((char *) &header2)){
                    *no_entries = index;
                    return 1;
                }
            }

            if (TAR_INT(header.size)%BLOCKSIZE == 0){
                nb += (1 + TAR_INT(header.size)/BLOCKSIZE);
            } else {
                nb += (2 + TAR_INT(header.size)/BLOCKSIZE);
            }
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

    int link = 0;

    printf("%s\n", path);

    if(is_file(tar_fd, path)==0){
        if(is_symlink(tar_fd, path)==0){
            perror("path given to read_file is invalid.\n");
            return -1;
        }else{ link=1; }
    }

    int nb = 0;
    int bytes_to_read;

    while (1){
        tar_header_t header;
        if (pread(tar_fd, &header, sizeof(tar_header_t), nb*sizeof(tar_header_t)) < 0) perror("pread error in read_file\n");

        if (!strcmp(header.name, path)){//if we find the file
            if (link){//if link, we run read_file on the link
                return read_file(tar_fd, header.linkname, offset, dest, len);
            }else{
                bytes_to_read = TAR_INT(header.size) - offset; //number of bytes we should read to get to the end of the file
                if(bytes_to_read < 0){
                    perror("read_file error: the offset is outside of the file size.\n");
                    return -2;
                }else{
                    if(bytes_to_read <= *len){ //dest buffer size is long enough to read until the end of the file
                        *len = (size_t) bytes_to_read; //len is set to the number of bytes written to dest
                        pread(tar_fd, dest, *len, (nb+1)*sizeof(tar_header_t) + offset);
                        return 0;
                    }else{  //dest buffer is too short to reach the end of the file
                        pread(tar_fd, dest, *len, (nb+1)*sizeof(tar_header_t) + offset);
                        return bytes_to_read - *len;
                    }
                }
            }
        }

        if (!strlen((char *) &header)){
            tar_header_t header2;
            if (pread(tar_fd, &header2, sizeof(tar_header_t), (nb+1)*sizeof(tar_header_t)) < 0) perror("pread error in read_file\n");
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
