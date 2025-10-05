// src/parse.c
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "parse.h"

// If your project already defines STATUS_OK/STATUS_ERROR in common.h, you can
// include that and remove these. For now, keep it simple:
#ifndef STATUS_OK
#define STATUS_OK 0
#endif
#ifndef STATUS_ERROR
#define STATUS_ERROR -1
#endif

int add_employee(struct dbheader_t *dbhdr, struct employee_t *employees, char *addstring) {
    // Not required for this test; stub so you can compile now.
    (void)dbhdr; (void)employees; (void)addstring;
    return STATUS_OK;
}

int list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {
    // Not required for this test; stub so you can compile now.
    (void)dbhdr; (void)employees;
    return STATUS_OK;
}

/*
 * Create a brand-new header in memory with the required fields:
 * - magic    = HEADER_MAGIC
 * - version  = 1
 * - count    = 0
 * - filesize = sizeof(struct dbheader_t)
 */
int create_db_header(struct dbheader_t **headerOut) {
    if (!headerOut) return STATUS_ERROR;

    struct dbheader_t *h = calloc(1, sizeof *h);
    if (!h) {
        perror("calloc");
        return STATUS_ERROR;
    }

    h->magic    = HEADER_MAGIC;
    h->version  = 1;
    h->count    = 0;
    h->filesize = (uint32_t)sizeof(struct dbheader_t);

    *headerOut = h;
    return STATUS_OK;
}

/*
 * Read the header from fd and validate:
 * - version == 1
 * - magic   == HEADER_MAGIC
 * - filesize matches actual file size
 * On success, returns STATUS_OK and sets *headerOut to a heap-allocated copy.
 * Caller owns *headerOut and must free it.
 */
// Replace your validate_db_header with this hardened version
int validate_db_header(int fd, struct dbheader_t **headerOut) {
    if (headerOut) *headerOut = NULL;
    if (fd < 0 || !headerOut) return STATUS_ERROR;

    struct stat st;
    if (fstat(fd, &st) == -1) { perror("fstat"); return STATUS_ERROR; }

    if (lseek(fd, 0, SEEK_SET) == (off_t)-1) { perror("lseek"); return STATUS_ERROR; }

    struct dbheader_t *header = calloc(1, sizeof *header);
    if (!header) { perror("calloc"); return STATUS_ERROR; }

    ssize_t n = read(fd, header, sizeof *header);
    if (n != (ssize_t)sizeof *header) { perror("read"); free(header); return STATUS_ERROR; }

    if (header->version != 1) { free(header); return STATUS_ERROR; }
    if (header->magic   != HEADER_MAGIC) { free(header); return STATUS_ERROR; }
    if (header->filesize != (uint32_t)st.st_size) { free(header); return STATUS_ERROR; }

    *headerOut = header;
    return STATUS_OK;
}

/*
 * Later tests will likely need these. For now, provide safe stubs so you pass
 * compilation for the header test. You can fill these out in the next sections.
 */
int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
    (void)fd; (void)dbhdr; (void)employeesOut;
    return STATUS_OK;
}

int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
    (void)fd; (void)dbhdr; (void)employees;
    return STATUS_OK;
}

