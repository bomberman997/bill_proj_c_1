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
    struct employee_t *employees = NULL;

    while ((c = getopt(argc, argv, "nf:a:l")) != -1) {
        switch (c) {
            case 'n':
                newfile = true;
                break;
            case 'f':
                filepath = optarg;
                break;
            case 'a':
                addstring = optarg;
                break;
            case 'l':
                list = true;
                break;
            case '?':
                printf("Unknown option -%c\n", c);
                break;
            default:
                return -1;
        }
    }

    if (filepath == NULL) {
        printf("Filepath is a required argument\n");
        print_usage(argv);
        return -1;
    }

    if (newfile) {
        dbfd = create_db_file(filepath);
        if (dbfd == STATUS_ERROR) {
            printf("Unable to create database file\n");
            return -1;
        }

        // Fixed: Only ONE argument (removed dbfd)
        if (create_db_header(&dbhdr) == STATUS_ERROR) {
            printf("Failed to create database header\n"); 
            return -1;
        }
    } else {
        dbfd = open_db_file(filepath);
        if (dbfd == STATUS_ERROR) {
            printf("Unable to open database file\n");
            return -1;
        }

        if (validate_db_header(dbfd, &dbhdr) == STATUS_ERROR) {
            printf("Failed to validate database header\n");
            return -1;
        }
    }

    if (read_employees(dbfd, dbhdr, &employees) != STATUS_SUCCESS) {
        printf("Failed to read employees\n");
        output_file(dbfd, dbhdr, NULL);
        return -1;
    }

    if (addstring) {
        dbhdr->count++;
        struct employee_t *tmp = realloc(employees, dbhdr->count * sizeof(struct employee_t));
        if (!tmp) {
            perror("realloc");
            free(employees);
            return -1;
        }
        employees = tmp;
        add_employee(dbhdr, employees, addstring);
    }

    if (list) {
        list_employees(dbhdr, employees);
    }

    output_file(dbfd, dbhdr, employees);

    free(employees);
    free(dbhdr);
    close(dbfd);

    return 0;
}
