#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include "common.h"
#include "parse.h"

int add_employee(struct dbheader_t *dbhdr, struct employee_t *employees, char *addstring) {
    if (!dbhdr || !employees || !addstring) {
        return STATUS_ERROR;
    }

    // Create a writable copy of the input string because strtok modifies it.
    // Passing a read-only string from argv to strtok causes a segfault.
    char *addstring_copy = strdup(addstring);
    if (addstring_copy == NULL) {
        perror("strdup");
        return STATUS_ERROR;
    }

    char *name = strtok(addstring_copy, ",");
    if (name == NULL) {
        free(addstring_copy);
        return STATUS_ERROR;
    }

    char *address = strtok(NULL, ",");
    if (address == NULL) {
        free(addstring_copy);
        return STATUS_ERROR;
    }

    char *hours_str = strtok(NULL, ",");
    if (hours_str == NULL) {
        free(addstring_copy);
        return STATUS_ERROR;
    }

    // main() increments the count *before* calling, so the new employee is at the last index.
    struct employee_t *new_employee = &employees[dbhdr->count - 1];

    // Safely copy the parsed data, ensuring null termination.
    strncpy(new_employee->name, name, NAME_LEN - 1);
    new_employee->name[NAME_LEN - 1] = '\0';

    strncpy(new_employee->address, address, ADDRESS_LEN - 1);
    new_employee->address[ADDRESS_LEN - 1] = '\0';

    new_employee->hours = atoi(hours_str);

    // Free the memory allocated by strdup.
    free(addstring_copy);

    return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
    if (!dbhdr || !employeesOut) return STATUS_ERROR;
    if (fd < 0) return STATUS_ERROR;

    unsigned short count = dbhdr->count;

    if (count == 0) {
        *employeesOut = NULL;
        return STATUS_SUCCESS;
    }

    if (lseek(fd, (off_t)sizeof(struct dbheader_t), SEEK_SET) == (off_t)-1) {
        perror("lseek");
        return STATUS_ERROR;
    }

    size_t bytes = (size_t)count * sizeof(struct employee_t);
    struct employee_t *buf = (struct employee_t *)calloc(count, sizeof(struct employee_t));
    if (!buf) {
        perror("calloc");
        return STATUS_ERROR;
    }

    ssize_t got = read(fd, buf, bytes);
    if (got != (ssize_t)bytes) {
        perror("read employees");
        free(buf);
        return STATUS_ERROR;
    }

    *employeesOut = buf;
    return STATUS_SUCCESS;
}

// Changed to return int to match test harness, and added defensive null-termination.
int list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {
    if (!dbhdr) {
        return STATUS_ERROR;
    }

    unsigned short count = dbhdr->count;
    if (count == 0 || !employees) {
        return STATUS_SUCCESS;
    }

    // --- START FIX ---
    // The test harness may pass employee data with strings that are not
    // null-terminated. This loop makes the function "defensive" by ensuring
    // a null terminator exists at the end of the buffer before printing.
    for (unsigned short i = 0; i < count; i++) {
        employees[i].name[NAME_LEN - 1] = '\0';
        employees[i].address[ADDRESS_LEN - 1] = '\0';
        printf("%s,%s,%u\n", employees[i].name, employees[i].address, employees[i].hours);
    }
    // --- END FIX ---
    
    return STATUS_SUCCESS;
}


int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
    if (fd < 0 || !dbhdr) return STATUS_ERROR;

    uint16_t realcount = dbhdr->count;
    struct dbheader_t wire = *dbhdr;
    wire.filesize = sizeof(struct dbheader_t) + sizeof(struct employee_t) * realcount;

    if (lseek(fd, 0, SEEK_SET) == (off_t)-1) { perror("lseek"); return STATUS_ERROR; }
    if (write(fd, &wire, sizeof wire) != (ssize_t)sizeof wire) { perror("write header"); return STATUS_ERROR; }

    if (!employees || realcount == 0) return STATUS_SUCCESS;

    for (int i = 0; i < realcount; i++) {
        if (write(fd, &employees[i], sizeof employees[i]) != (ssize_t)sizeof employees[i]) {
            perror("write employee");
            return STATUS_ERROR;
        }
    }
    return STATUS_SUCCESS;
}


int validate_db_header(int fd, struct dbheader_t **headerOut) {
    if (fd < 0) { return STATUS_ERROR; }

    struct dbheader_t *header = calloc(1, sizeof *header);
    if (!header) { return STATUS_ERROR; }

    if (lseek(fd, 0, SEEK_SET) == (off_t)-1) { perror("lseek"); free(header); return STATUS_ERROR; }
    if (read(fd, header, sizeof *header) != (ssize_t)sizeof *header) { perror("read"); free(header); return STATUS_ERROR; }

    if (header->version != 1) { free(header); return STATUS_ERROR; }
    if (header->magic   != HEADER_MAGIC) { free(header); return STATUS_ERROR; }

    struct stat st = {0};
    if (fstat(fd, &st) == -1) { perror("fstat"); free(header); return STATUS_ERROR; }

    if (header->filesize != (unsigned int)st.st_size) {
        free(header);
        return STATUS_ERROR;
    }

    *headerOut = header;
    return STATUS_SUCCESS;
}

int create_db_header(struct dbheader_t **headerOut) {
    if (!headerOut) return STATUS_ERROR;

    struct dbheader_t *h = calloc(1, sizeof *h);
    if (!h) return STATUS_ERROR;

    h->magic    = HEADER_MAGIC;
    h->version  = 1;
    h->count    = 0;
    h->filesize = (unsigned int)sizeof(struct dbheader_t);

    *headerOut = h;
    return STATUS_SUCCESS;
}
