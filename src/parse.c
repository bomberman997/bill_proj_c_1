// src/parse.c
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>


#include "parse.h"
#include "common.h"

int add_employee(struct dbheader_t *dbhdr, struct employee_t *employees, char *addstring) {
    // Not required for this test; stub so you can compile now.
    (void)dbhdr; (void)employees; (void)addstring;
    return STATUS_SUCCESS;
}

int list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {
    // Not required for this test; stub so you can compile now.
    (void)dbhdr; (void)employees;
    return STATUS_SUCCESS;
}

/*
 * Create a brand-new header in memory with the required fields:
 * - magic    = HEADER_MAGIC
 * - version  = 1
 * - count    = 0
 * - filesize = sizeof(struct dbheader_t)
 
int create_db_header(struct dbheader_t **headerOut) {
    if (!headerOut) return STATUS_ERROR;


    struct dbheader_t *h = calloc(1, sizeof *h);
    if (!h) {
        perror("calloc");
        return STATUS_ERROR;
    }

    h->magic  = HEADER_MAGIC;
    h->version  = 1;
    h->count    = 0;
    h->filesize = sizeof(struct dbheader_t);

    *headerOut = h;
    return STATUS_SUCCESS;
}*/

// In src/parse.c
int create_db_header(struct dbheader_t **headerOut) {
    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == NULL) {
        printf("Malloc failed to create db header\n");
        return STATUS_ERROR;
    }

    header->version = 1;
    header->count = 0;
    header->magic = HEADER_MAGIC;
    header->filesize = sizeof(struct dbheader_t);

    *headerOut = header;
    return STATUS_SUCCESS;
}


// In src/parse.c

int validate_db_header(int fd, struct dbheader_t **headerOut) {
    if (fd < 0) {
        return STATUS_ERROR;
    }

    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == NULL) {
        return STATUS_ERROR;
    }

    if (read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
        perror("read for validation");
        free(header);
        return STATUS_ERROR;
    }

    // Convert from network to host byte order for validation
    header->version = ntohs(header->version);
    header->count = ntohs(header->count);
    header->magic = ntohl(header->magic);
    header->filesize = ntohl(header->filesize);

    if (header->magic != HEADER_MAGIC) {
        free(header);
        return STATUS_ERROR;
    }

    if (header->version != 1) {
        free(header);
        return STATUS_ERROR;
    }

    struct stat dbstat = {0};
    if (fstat(fd, &dbstat) == -1) {
        perror("fstat for validation");
        free(header);
        return STATUS_ERROR;
    }
    
    if (header->filesize != dbstat.st_size) {
        free(header);
        return STATUS_ERROR;
    }

    *headerOut = header;
    return STATUS_SUCCESS;
}


/**
 * Later tests will likely need these. For now, provide safe stubs so you pass
 * compilation for the header test. You can fill these out in the next sections.
 */
int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
    (void)fd; (void)dbhdr; (void)employeesOut;
    return STATUS_SUCCESS;
}


// ... other functions

int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
    (void)employees;
    if (fd < 0 || !dbhdr) { 
        fprintf(stderr, "Bad fd or null header\n"); 
        return STATUS_ERROR; 
    }

    // Ensure filesize is set correctly
    dbhdr->filesize = sizeof(struct dbheader_t);

    // Truncate to exact size
    if (ftruncate(fd, dbhdr->filesize) == -1) { 
        perror("ftruncate"); 
        return STATUS_ERROR; 
    }

    if (lseek(fd, 0, SEEK_SET) == (off_t)-1) { 
        perror("lseek"); 
        return STATUS_ERROR; 
    }

    // Write in HOST ORDER (matching your hex dump)
    ssize_t n = write(fd, dbhdr, sizeof(*dbhdr));
    if (n != (ssize_t)sizeof(*dbhdr)) { 
        perror("write"); 
        return STATUS_ERROR; 
    }

    fsync(fd);
    return STATUS_SUCCESS;
}
