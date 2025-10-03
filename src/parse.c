#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include "common.h"
#include "parse.h"

void list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {
  (void)dbhdr; (void)employees; // TODO: implement listing
}

int add_employee(struct dbheader_t *dbhdr, struct employee_t *employees, char *addstring) {
  (void)dbhdr; (void)employees; (void)addstring; // TODO: parse & append
  return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
  (void)fd; (void)dbhdr; (void)employeesOut; // TODO: allocate & read N employees
  return STATUS_SUCCESS;
}

int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
  if (fd < 0){
    printf("Got a bad FD from user\n");
    return STATUS_ERROR;
  }
  if (!dbhdr) {
    printf("Missing db header\n");
    return STATUS_ERROR;
  }

  // Use host-order values to compute sizes; don't mutate dbhdr in place.
  uint16_t realcount = dbhdr->count;

  struct dbheader_t wire = *dbhdr; // copy to convert to wire format (network byte order)
  wire.filesize = sizeof(struct dbheader_t) + (sizeof(struct employee_t) * realcount);
  wire.magic    = htonl(wire.magic);
  wire.filesize = htonl(wire.filesize);
  wire.count    = htons(wire.count);
  wire.version  = htons(wire.version);

  if (lseek(fd, 0, SEEK_SET) == (off_t)-1) {
    perror("lseek");
    return STATUS_ERROR;
  }
  if (write(fd, &wire, sizeof wire) != (ssize_t)sizeof wire) {
    perror("write header");
    return STATUS_ERROR;
  }

  // No employees yet? header-only write is fine.
  if (employees == NULL || realcount == 0) {
    return STATUS_SUCCESS;
  }

  for (int i = 0; i < realcount; i++) {
    struct employee_t onwire = employees[i];
    onwire.hours = htonl(onwire.hours); // convert per-field as needed
    if (write(fd, &onwire, sizeof onwire) != (ssize_t)sizeof onwire) {
      perror("write employee");
      return STATUS_ERROR;
    }
  }

  return STATUS_SUCCESS;
}

int validate_db_header(int fd, struct dbheader_t **headerOut) {
  if (fd < 0){
    printf("Got a bad FD from user\n");
    return STATUS_ERROR;
  }
  struct dbheader_t *header = calloc(1,sizeof(struct dbheader_t));
  if (header == NULL){
    printf("Malloc failed create db header\n");
    return STATUS_ERROR;
  }

  if (read(fd, header, sizeof *header) != (ssize_t)sizeof *header){
    perror("read");
    free(header);
    return STATUS_ERROR;
  }

  header->version  = ntohs(header->version);
  header->count    = ntohs(header->count);
  header->magic    = ntohl(header->magic);
  header->filesize = ntohl(header->filesize);

  if (header->version != 1){
    printf("Improper header version\n");
    free(header);
    return STATUS_ERROR;
  }
  if (header->magic != HEADER_MAGIC){
    printf("Improper header magic\n");
    free(header);
    return STATUS_ERROR;
  }

  struct stat dbstat = {0};
  if (fstat(fd, &dbstat) == -1) {
    perror("fstat");
    free(header);
    return STATUS_ERROR;
  }
  if (header->filesize != (unsigned int)dbstat.st_size){
    printf("Corrupted database\n");
    free(header);
    return STATUS_ERROR;
  }

  *headerOut = header;
  return STATUS_SUCCESS; // ✅ ensure we return
}
int create_db_header(struct dbheader_t **headerOut) {
    if (!headerOut) return STATUS_ERROR;

    struct dbheader_t *h = calloc(1, sizeof *h);
    if (!h) return STATUS_ERROR;

    h->magic    = HEADER_MAGIC;                 // 0x4c4c4144
    h->version  = 1;                            // start at 1
    h->count    = 0;                            // none yet
    h->filesize = (unsigned int)sizeof(struct dbheader_t);  // <-- NO htonl

    *headerOut = h;
    return STATUS_SUCCESS;
}
