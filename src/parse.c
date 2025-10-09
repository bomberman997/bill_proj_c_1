#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "parse.h"

void list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {
    int i = 0;
    for (; i < dbhdr->count; i++) {
        printf("Employee %d\n", i);
        printf("\tName: %s\n", employees[i].name);
        printf("\tAddress: %s\n", employees[i].address);
        printf("\tHours: %d\n", employees[i].hours);
    }
}

int add_employee(struct dbheader_t *dbhdr, struct employee_t *employees, char *addstring) {
    printf("%s\n", addstring);

    char *name = strtok(addstring, ",");
    char *addr = strtok(NULL, ",");
    char *hours = strtok(NULL, ",");
    printf("%s %s %s\n", name, addr, hours);

    strncpy(employees[dbhdr->count-1].name, name, sizeof(employees[dbhdr->count-1].name));
    strncpy(employees[dbhdr->count-1].address, addr, sizeof(employees[dbhdr->count-1].address));
    employees[dbhdr->count-1].hours = atoi(hours);

    return STATUS_SUCCESS;
}

int remove_employee(struct dbheader_t *dbhdr, struct employee_t *employees, char *name_to_remove) {
    int i = 0;
    int count_before_remove = dbhdr->count;
    for (; i < dbhdr->count; i++) {
        if (strcmp(employees[i].name, name_to_remove)) {
            continue;
        }

        memmove(employees + i, employees + i + 1, (--(dbhdr->count) - i) * sizeof(struct employee_t));
        i--;
        printf("Employee \"%s\" has been removed. %d employees are currently stored in the DB\n", name_to_remove, dbhdr->count);
    }

    if (count_before_remove != dbhdr->count) {
        return STATUS_SUCCESS;
    }

    printf("Employee \"%s\" does not exist\n", name_to_remove);
    return STATUS_ERROR;
}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
    if (fd < 0) {
        printf("Got invalid FD\n");
        return STATUS_ERROR;
    }

    int count = dbhdr->count;

    struct employee_t *employees = calloc(count, sizeof(struct employee_t));
    if (employees == NULL) {
        printf("calloc failed");
        return STATUS_ERROR;
    }

    read(fd, employees, count * sizeof(struct employee_t));

    int i = 0;
    for (; i < count; i++) {
        employees[i].hours = ntohl(employees[i].hours);
    }

    *employeesOut = employees;

    return STATUS_SUCCESS;
}

int update_employee_hours(struct dbheader_t *dbhdr, struct employee_t *employees, char *name, unsigned int hours) {
    int i = 0;
    bool updated = false;
    for (; i < dbhdr->count; i++) {
        if (strcmp(employees[i].name, name)) {
            continue;
        }

        employees[i].hours = hours;
        printf("Employee \"%s\" has been updated.\n", name);
    }
    
    if (!updated) {
        printf("Employee \"%s\" does not exist.\n", name);
    }

    return STATUS_SUCCESS;
}

int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
    if (fd < 0) {
        printf("Got invalid FD\n");
        return STATUS_ERROR;
    }
    
    int realcount = dbhdr->count;
    unsigned int current_filesize = dbhdr->filesize;
    unsigned int new_filesize = sizeof(struct dbheader_t) + sizeof(struct employee_t) * realcount; 

    dbhdr->magic = htonl(dbhdr->magic);
    dbhdr->filesize = htonl(new_filesize);
    dbhdr->count = htons(dbhdr->count);
    dbhdr->version = htons(dbhdr->version); 

    lseek(fd, 0, SEEK_SET);
    write(fd, dbhdr, sizeof(struct dbheader_t));

    int i = 0;
    for (; i < realcount; i++) {
        employees[i].hours = htonl(employees[i].hours);
        write(fd, &employees[i], sizeof(struct employee_t));
    }

    if (current_filesize > new_filesize && ftruncate(fd, new_filesize) != 0) {
        perror("ftruncate");
        return STATUS_ERROR;
    }

    return STATUS_SUCCESS;
}	

int validate_db_header(int fd, struct dbheader_t **headerOut) {
    if (fd < 0) {
        printf("Got invalid FD\n");
        return STATUS_ERROR;
    }

    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == NULL) {
        printf("calloc failed to create a db header\n");
        return STATUS_ERROR;
    }

    if (read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
        perror("read");
        printf("The size of the header in file does not match with expected size\n");
        free(header);
        return STATUS_ERROR;
    }

    header->version = ntohs(header->version);
    header->count = ntohs(header->count);
    header->magic = ntohl(header->magic);
    header->filesize = ntohl(header->filesize);

    if (header->magic != HEADER_MAGIC) {
        printf("Improper header magic\n");
        free(header);
        return -1;
    }

    if (header->version != 1) {
        printf("Improper header version\n");
        free(header);
        return -1;
    }

    struct stat dbstat = {0};
    fstat(fd, &dbstat);
    if (header->filesize != dbstat.st_size) {
        printf("Corrupted database\n");
        free(header);
        return -1;
    }

    *headerOut = header;

    return STATUS_SUCCESS;
}

int create_db_header(int fd, struct dbheader_t **headerOut) {
    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == NULL) {
        printf("calloc failed to create db header\n");
        return STATUS_ERROR;
    }

    header->version = 0x1;
    header->count = 0;
    header->magic = HEADER_MAGIC;
    header->filesize = sizeof(struct dbheader_t);

    *headerOut = header;

    return STATUS_SUCCESS;
}




