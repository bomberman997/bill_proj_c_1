#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "file.h"
#include "common.h"

// Creates a new database file, fails if it already exists.
int create_db_file(char *filename) {
    int fd = open(filename, O_RDWR | O_CREAT | O_EXCL, 0644);
    if (fd == -1) {
        perror("open (create)");
        return STATUS_ERROR;
    }
    return fd;
}

// Opens an existing database file.
int open_db_file(char *filename) {
    int fd = open(filename, O_RDWR, 0644);
    if (fd == -1) {
        perror("open (open)");
        return STATUS_ERROR;
    }
    return fd;
}
