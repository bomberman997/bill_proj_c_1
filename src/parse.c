#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h> // For htonl, htons, etc.

#include "parse.h"
#include "common.h"

// Stubs for functions not required by this specific test
int add_employee(struct dbheader_t *dbhdr, struct employee_t *employees, char *addstring) {
    (void)dbhdr; (void)employees; (void)addstring;
    return STATUS_SUCCESS;
}
int list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {
    (void)dbhdr; (void)employees;
    return STATUS_SUCCESS;
}
int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
    (void)fd; (void)dbhdr; (void)employeesOut;
    return STATUS_SUCCESS;
}

// Corrected signature: Does not take fd
int create_db_header(struct dbheader_t **headerOut) {
    if (!headerOut) {
        return STATUS_ERROR;
    }

    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == NULL) {
        perror("calloc for header");
        return STATUS_ERROR;
    }

    header->magic = HEADER_MAGIC;
    header->version = 1;
    header->count = 0;
    header->filesize = sizeof(struct dbheader_t);

    *headerOut = header;
    return STATUS_SUCCESS;
}


int validate_db_header(int fd, struct dbheader_t **headerOut) {
    if (fd < 0 || !headerOut) {
        return STATUS_ERROR;
    }

    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == NULL) {
        perror("calloc for validation");
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
        fprintf(stderr, "Invalid magic number\n");
        free(header);
        return STATUS_ERROR;
    }

    if (header->version != 1) {
        fprintf(stderr, "Invalid version\n");
        free(header);
        return STATUS_ERROR;
    }

    struct stat dbstat = {0};
    if (fstat(fd, &dbstat) == -1) {
        perror("fstat for validation");
        free(header);
        return STATUS_ERROR;
    }
    
    if (header->filesize != (unsigned int)dbstat.st_size) {
        fprintf(stderr, "Filesize in header does not match actual file size\n");
        free(header);
        return STATUS_ERROR;
    }

    *headerOut = header;
    return STATUS_SUCCESS; // CRITICAL: This was missing
}


int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
    if (fd < 0 || !dbhdr) {
        return STATUS_ERROR;
    }
    (void)employees; // Not used in this step

    int realcount = dbhdr->count;

    // --- CRITICAL FIX: Create a temporary copy for disk operations ---
    // This prevents corrupting the original dbhdr in memory.
    struct dbheader_t disk_header = *dbhdr;

    // Convert the fields of the COPY to network byte order
    disk_header.magic = htonl(disk_header.magic);
    disk_header.filesize = htonl(sizeof(struct dbheader_t) + (sizeof(struct employee_t) * realcount));
    disk_header.count = htons(disk_header.count);
    disk_header.version = htons(disk_header.version);

    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("lseek for output");
        return STATUS_ERROR;
    }

    // Write the network-ordered COPY to the file
    if (write(fd, &disk_header, sizeof(disk_header)) != sizeof(disk_header)) {
        perror("write header for output");
        return STATUS_ERROR;
    }

    // Truncate the file to the exact size of the header (for this step)
    if (ftruncate(fd, sizeof(struct dbheader_t)) == -1) {
        perror("ftruncate");
        return STATUS_ERROR;
    }

    return STATUS_SUCCESS;
}
