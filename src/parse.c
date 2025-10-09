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
int create_db_header(struct dbheader_t **headerOut)
{
    if (!headerOut) return STATUS_ERROR;  // Don't dereference if NULL
    
    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == NULL) {
        printf("Malloc failed to create db header\n");
        return STATUS_ERROR;
    }

    header->magic    = HEADER_MAGIC;
    header->version  = 0x1;
    header->count    = 0;
    header->filesize = sizeof(struct dbheader_t);

    *headerOut = header;  // Only dereference after we know it's valid
    return STATUS_SUCCESS;
}
// In src/parse.c
int validate_db_header(int fd, struct dbheader_t **headerOut) {
    if (headerOut) *headerOut = NULL;
    if (fd < 0 || !headerOut) return STATUS_ERROR;

    struct stat st;
    if (fstat(fd, &st) == -1) { 
        perror("fstat"); 
        return STATUS_ERROR; 
    }
    if ((size_t)st.st_size < sizeof(struct dbheader_t)) {
        return STATUS_ERROR;
    }

    if (lseek(fd, 0, SEEK_SET) == (off_t)-1) { 
        perror("lseek"); 
        return STATUS_ERROR; 
    }

    struct dbheader_t disk_hdr;
    ssize_t n = read(fd, &disk_hdr, sizeof(disk_hdr));
    if (n != (ssize_t)sizeof(disk_hdr)) { 
        perror("read"); 
        return STATUS_ERROR; 
    }

    // Convert from network order to host order
    struct dbheader_t *h = calloc(1, sizeof(*h));
    if (!h) { 
        perror("calloc"); 
        return STATUS_ERROR; 
    }

    h->magic    = ntohl(disk_hdr.magic);
    h->version  = ntohs(disk_hdr.version);
    h->count    = ntohs(disk_hdr.count);
    h->filesize = ntohl(disk_hdr.filesize);

    // Validate fields (now in host order)
    if (h->magic != HEADER_MAGIC) { 
        free(h); 
        return STATUS_ERROR; 
    }
    if (h->version != 1) { 
        free(h); 
        return STATUS_ERROR; 
    }
    if (h->filesize != (unsigned int)st.st_size) { 
        free(h); 
        return STATUS_ERROR; 
    }

    *headerOut = h;
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

    // Create a network-order copy for writing
    struct dbheader_t disk_hdr;
    disk_hdr.magic    = htonl(dbhdr->magic);
    disk_hdr.version  = htons(dbhdr->version);
    disk_hdr.count    = htons(dbhdr->count);
    disk_hdr.filesize = htonl(dbhdr->filesize);

    // Truncate to the size specified in the header
    if (ftruncate(fd, dbhdr->filesize) == -1) { 
        perror("ftruncate"); 
        return STATUS_ERROR; 
    }

    if (lseek(fd, 0, SEEK_SET) == (off_t)-1) { 
        perror("lseek"); 
        return STATUS_ERROR; 
    }

    // Write the network-order copy
    ssize_t n = write(fd, &disk_hdr, sizeof(disk_hdr));
    if (n != (ssize_t)sizeof(disk_hdr)) { 
        perror("write"); 
        return STATUS_ERROR; 
    }

    fsync(fd);
    return STATUS_SUCCESS;
}
