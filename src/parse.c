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

int create_db_header(struct dbheader_t **headerOut) {  // No fd!
    if (!headerOut) return STATUS_ERROR;
    
    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == NULL) {
        printf("Malloc failed to create db header\n");
        return STATUS_ERROR;
    }

    header->version = 0x01;
    header->count = 0;
    header->magic = HEADER_MAGIC;
    header->filesize = sizeof(struct dbheader_t);
	
    *headerOut = header; 
    return STATUS_SUCCESS;
}

int validate_db_header(int fd, struct dbheader_t **headerOut) {
    if (fd < 0) {
        printf("Got a bad FD from the user\n");
        return STATUS_ERROR;
    }

    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == NULL) {
        printf("Malloc failed to create dbheader\n");
        return STATUS_ERROR;
    }

    if (read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
        perror("read");
        free(header);
        return STATUS_ERROR;
    }

    header->version = ntohs(header->version);
    header->count = ntohs(header->count);
    header->magic = ntohl(header->magic);
    header->filesize = ntohl(header->filesize);
    
    if (header->magic != HEADER_MAGIC) {
        printf("Improper header magic!\n");
        free(header);
        return STATUS_ERROR;
    }

    if (header->version != 1) {
        printf("Improper header version!\n");
        free(header);
        return STATUS_ERROR;
    }

    struct stat dbstat = {0};
    fstat(fd, &dbstat);
    if (header->filesize != dbstat.st_size) {
        printf("Corrupted database!\n");
        free(header);
        return STATUS_ERROR;
    }

    *headerOut = header;
    return STATUS_SUCCESS;  // ← ADD THIS!
}
/**
 * Later tests will likely need these. For now, provide safe stubs so you pass
 * compilation for the header test. You can fill these out in the next sections.
 */
int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
    (void)fd; (void)dbhdr; (void)employeesOut;
    return STATUS_SUCCESS;
}


int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
    if (fd < 0) {
        printf("Got a bad fd from the user\n");
        return STATUS_ERROR;
    }

    int realcount = dbhdr->count;
    
    // Create a COPY for network byte order conversion
    struct dbheader_t disk_hdr = *dbhdr;
    disk_hdr.magic = htonl(disk_hdr.magic);
    disk_hdr.filesize = htonl(sizeof(struct dbheader_t) + (sizeof(struct employee_t) * realcount));
    disk_hdr.count = htons(disk_hdr.count);
    disk_hdr.version = htons(disk_hdr.version);

    lseek(fd, 0, SEEK_SET);
    write(fd, &disk_hdr, sizeof(struct dbheader_t));  // Write the copy
        
    int i = 0;
    for (; i < realcount; i++) {
        struct employee_t disk_emp = employees[i];
        disk_emp.hours = htonl(disk_emp.hours);
        write(fd, &disk_emp, sizeof(struct employee_t));
    } 
        
    return STATUS_SUCCESS;
}
