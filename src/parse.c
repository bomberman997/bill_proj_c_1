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

}

int add_employee(struct dbheader_t *dbhdr, struct employee_t *employees, char *addstring) {

}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {

}

int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
	if(fd == -1) {
		printf("Invalid File Descriptor\n");
		return -1;
	}
	dbhdr->version = htons(dbhdr->version);
	dbhdr->count = htonl(dbhdr->count);
	dbhdr->magic = htonl(dbhdr->magic);
	dbhdr->filesize = htonl(dbhdr->filesize);

	lseek(fd, 0, SEEK_SET); 
	write(fd, dbhdr, sizeof(struct dbheader_t));
}	

int validate_db_header(int fd, struct dbheader_t **headerOut) {

	if(fd == -1) {
		printf("Invalid file descriptor\n");
		return STATUS_ERROR;
	}
	
	struct dbheader_t *header = NULL;
	header = calloc(1, sizeof(struct dbheader_t));
	if(header == NULL) {
		perror("calloc");
		free(header);
		return STATUS_SUCCESS;
	}
	
	if((read(fd, header, sizeof(struct dbheader_t))) != sizeof(struct dbheader_t)) {
		perror("read");
		free(header);
		return STATUS_ERROR;
	}

	//changing to host endian from network endian
	//if we don't do the following the endian-ness could get messed up
	//and we could be parsing something wrong
	header->version = ntohs(header->version);
	header->count = ntohs(header->count);
	header->magic = ntohl(header->magic);
	header->filesize = ntohl(header->filesize);
	
	if(header->magic != HEADER_MAGIC) {
		printf("Invalid header magic\n");	
		free(header);
		return STATUS_ERROR;
	}

	if(header->version != 1) {
		printf("Invalid header version\n");
		free(header);
		return STATUS_ERROR;
	}


	//matching the filesize in header to the stat file size
	struct stat dbstat = {0};
	fstat(fd, &dbstat);
	if(header->filesize != dbstat.st_size) {
		printf("Database file is corrupt\n");
		free(header);
		return STATUS_ERROR;
	}


	*headerOut = header;
	return STATUS_SUCCESS;
}

int create_db_header(int fd, struct dbheader_t **headerOut) {
	struct dbheader_t *header = NULL;
	header = calloc(1, sizeof(struct dbheader_t));
	if(header == NULL) {
		perror("calloc");
		return STATUS_ERROR;
	}
	header->version = 1;
	header->filesize = sizeof(struct dbheader_t);
	header->count = 0;
	header->magic = HEADER_MAGIC;
	*headerOut = header;
	return 0;
}


