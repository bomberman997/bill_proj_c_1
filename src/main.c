#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]) {
    printf("Usage: %s -f <database file> [-n] [-a \"Name,Address,Hours\"] [-l]\n", argv[0]);
    printf("\t -n  - create new database file\n");
    printf("\t -f  - (required) path to database file\n");
    printf("\t -a  - add employee (format: \"Name,Address,Hours\")\n");
    printf("\t -l  - list all employees\n");
}

int main(int argc, char *argv[]) { 
    char *filepath = NULL;
    bool newfile = false;
    bool list = false;
    char *addstring = NULL;
    int c;

    int dbfd = -1;
    struct dbheader_t *dbhdr = NULL;
	lseek(fd, 0, SEEK_SET); 
	write(fd, dbhdr, sizeof(struct dbheader_t));
}	

int validate_db_header(int fd, struct dbheader_t **headerOut) {

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
	printf("New count: %d\n", dbhdr->count);

	//tokenize the addstring
	char *name = strtok(addstring, ",");
	char *address = strtok(NULL, ",");
	short hours = atoi(strtok(NULL, ","));		
	printf("Adding new employee: %s %s %d\n", name, address, hours);
	strncpy(employees[dbhdr->count-1].name, name, sizeof(employees[dbhdr->count-1].name));	
	strncpy(employees[dbhdr->count-1].address, address, sizeof(employees[dbhdr->count-1].address));	
	employees[dbhdr->count-1].hours = hours;
}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
	int realcount = dbhdr->count;

	struct employee_t *employees = calloc(realcount, sizeof(struct employee_t));
	if(employees == NULL) {
		perror("calloc");
		return STATUS_ERROR;
	}
	
	int bytes = read(fd, employees, (sizeof(struct employee_t) * realcount));
	
	if(bytes == -1) {
		perror("read");
		return STATUS_ERROR;
	}

	int i;
	for(; i < realcount ; i++) {
		employees[i].hours = ntohl(employees[i].hours);
	}

	*employeesOut = employees;
	printf("Read the employees into the memory\n");

	return STATUS_SUCCESS;
}

int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
	if(fd == -1) {
		printf("Invalid File Descriptor\n");
		return -1;
	}

	int realcount = dbhdr->count;

	int total_file_size = sizeof(struct dbheader_t) + (realcount * sizeof(struct employee_t));
	dbhdr->version = htons(dbhdr->version);
	dbhdr->count = htons(dbhdr->count);
	dbhdr->magic = htonl(dbhdr->magic);
	dbhdr->filesize = htonl(total_file_size);

	lseek(fd, 0, SEEK_SET); 
	write(fd, dbhdr, sizeof(struct dbheader_t));

	int i = 0;
	for(; i < realcount; i++) {
		employees[i].hours = htonl(employees[i].hours);
		printf("writing to db: %s\n", employees[i].name);
		write(fd, &employees[i], sizeof(struct employee_t));
	}
}	

int validate_db_header(int fd, struct dbheader_t **headerOut) {

	if(fd == -1) {
		printf("Invalid file descriptor %d\n", fd);
		return STATUS_ERROR;
	}
	
	struct dbheader_t *header = NULL;
	header = calloc(1, sizeof(struct dbheader_t));
	if(header == NULL) {
		perror("calloc");
		free(header);
		return STATUS_SUCCESS;
	}

	printf("validating header with fd: %d\n", fd);
	
	if((read(fd, header, sizeof(struct dbheader_t))) !=  sizeof(struct dbheader_t)) {
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



